#include "mock_i2c.h"
#include "sht30.h"

#include <stdio.h>
#include <string.h>

#define CHECK(condition)                                                                  \
    do {                                                                                  \
        if (!(condition)) {                                                               \
            (void)fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #condition); \
            return 1;                                                                     \
        }                                                                                 \
    } while (0)

typedef struct {
    mock_i2c_t mock;
    sht30_t device;
} fixture_t;

static void fixture_init(fixture_t *fixture)
{
    sht30_config_t config;

    mock_i2c_reset(&fixture->mock);
    config.bus.context = &fixture->mock;
    config.bus.write = mock_i2c_write;
    config.bus.read = mock_i2c_read;
    config.bus.delay_ms = mock_i2c_delay_ms;
    config.address = SHT30_I2C_ADDRESS_DEFAULT;
    (void)memset(&fixture->device, 0, sizeof(fixture->device));
    (void)sht30_init(&fixture->device, &config);
}

static uint16_t last_command(const mock_i2c_t *mock)
{
    return ((uint16_t)mock->last_write[0] << 8) | mock->last_write[1];
}

static int test_crc_reference_vector(void)
{
    const uint8_t data[] = {0xBEU, 0xEFU};

    CHECK(sht30_crc8(data, sizeof(data)) == 0x92U);
    CHECK(sht30_crc8(NULL, 2U) == 0U);
    return 0;
}

static int test_init_validation(void)
{
    fixture_t fixture;
    sht30_config_t config;

    mock_i2c_reset(&fixture.mock);
    config.bus.context = &fixture.mock;
    config.bus.write = mock_i2c_write;
    config.bus.read = mock_i2c_read;
    config.bus.delay_ms = mock_i2c_delay_ms;
    config.address = SHT30_I2C_ADDRESS_ALT;
    CHECK(sht30_init(NULL, &config) == SHT30_ERROR_INVALID_ARGUMENT);
    CHECK(sht30_init(&fixture.device, NULL) == SHT30_ERROR_INVALID_ARGUMENT);
    CHECK(sht30_init(&fixture.device, &config) == SHT30_OK);
    CHECK(fixture.device.address == SHT30_I2C_ADDRESS_ALT);
    CHECK(fixture.mock.write_count == 0U);
    config.address = 0x43U;
    CHECK(sht30_init(&fixture.device, &config) == SHT30_ERROR_INVALID_ARGUMENT);
    config.address = 0x80U;
    CHECK(sht30_init(&fixture.device, &config) == SHT30_ERROR_INVALID_ARGUMENT);
    config.address = SHT30_I2C_ADDRESS_DEFAULT;
    config.bus.write = NULL;
    CHECK(sht30_init(&fixture.device, &config) == SHT30_ERROR_INVALID_ARGUMENT);
    return 0;
}

static int test_single_shot_high_repeatability(void)
{
    fixture_t fixture;
    sht30_data_t data = {0};

    fixture_init(&fixture);
    mock_i2c_set_measurement(&fixture.mock, 0x6666U, 0x8000U);
    CHECK(sht30_read_single_shot(&fixture.device, SHT30_REPEATABILITY_HIGH, &data) == SHT30_OK);
    CHECK(data.temperature_millic == 25000);
    CHECK(data.humidity_milli_percent == 50000U);
    CHECK(last_command(&fixture.mock) == 0x2400U);
    CHECK(fixture.mock.last_delay_ms == 16U);
    CHECK(fixture.mock.last_address == SHT30_I2C_ADDRESS_DEFAULT);
    CHECK(fixture.mock.write_count == 1U);
    CHECK(fixture.mock.read_count == 1U);
    return 0;
}

static int test_single_shot_conversion_limits(void)
{
    fixture_t fixture;
    sht30_data_t data = {0};

    fixture_init(&fixture);
    mock_i2c_set_measurement(&fixture.mock, 0x0000U, 0xFFFFU);
    CHECK(sht30_read_single_shot(&fixture.device, SHT30_REPEATABILITY_LOW, &data) == SHT30_OK);
    CHECK(data.temperature_millic == -45000);
    CHECK(data.humidity_milli_percent == 100000U);
    CHECK(last_command(&fixture.mock) == 0x2416U);
    CHECK(fixture.mock.last_delay_ms == 5U);

    mock_i2c_set_measurement(&fixture.mock, 0xFFFFU, 0x0000U);
    CHECK(sht30_read_single_shot(&fixture.device, SHT30_REPEATABILITY_MEDIUM, &data) == SHT30_OK);
    CHECK(data.temperature_millic == 130000);
    CHECK(data.humidity_milli_percent == 0U);
    CHECK(last_command(&fixture.mock) == 0x240BU);
    CHECK(fixture.mock.last_delay_ms == 7U);
    return 0;
}

static int test_crc_failure_does_not_update_output(void)
{
    fixture_t fixture;
    sht30_data_t data = {123, 456U};

    fixture_init(&fixture);
    mock_i2c_set_measurement(&fixture.mock, 0x6666U, 0x8000U);
    fixture.mock.rx_data[5] ^= 0x01U;
    CHECK(sht30_read_single_shot(&fixture.device, SHT30_REPEATABILITY_HIGH, &data) == SHT30_ERROR_CRC);
    CHECK(data.temperature_millic == 123);
    CHECK(data.humidity_milli_percent == 456U);
    return 0;
}

static int test_bus_errors_and_arguments(void)
{
    fixture_t fixture;
    sht30_data_t data = {0};

    fixture_init(&fixture);
    fixture.mock.write_result = -1;
    CHECK(sht30_read_single_shot(&fixture.device, SHT30_REPEATABILITY_HIGH, &data) == SHT30_ERROR_BUS_WRITE);
    CHECK(fixture.mock.read_count == 0U);
    CHECK(fixture.mock.delay_count == 0U);

    fixture.mock.write_result = 0;
    fixture.mock.read_result = -1;
    CHECK(sht30_read_single_shot(&fixture.device, SHT30_REPEATABILITY_HIGH, &data) == SHT30_ERROR_BUS_READ);
    CHECK(fixture.mock.delay_count == 1U);
    CHECK(sht30_read_single_shot(&fixture.device, (sht30_repeatability_t)99, &data) ==
          SHT30_ERROR_INVALID_ARGUMENT);
    CHECK(sht30_read_single_shot(&fixture.device, SHT30_REPEATABILITY_HIGH, NULL) ==
          SHT30_ERROR_INVALID_ARGUMENT);
    CHECK(sht30_read_single_shot(NULL, SHT30_REPEATABILITY_HIGH, &data) ==
          SHT30_ERROR_INVALID_ARGUMENT);
    return 0;
}

static int test_status_reset_and_heater(void)
{
    fixture_t fixture;
    uint16_t status = 0U;

    fixture_init(&fixture);
    mock_i2c_set_status(&fixture.mock, 0x8010U);
    CHECK(sht30_read_status(&fixture.device, &status) == SHT30_OK);
    CHECK(status == 0x8010U);
    CHECK(last_command(&fixture.mock) == 0xF32DU);

    CHECK(sht30_soft_reset(&fixture.device) == SHT30_OK);
    CHECK(last_command(&fixture.mock) == 0x30A2U);
    CHECK(fixture.mock.last_delay_ms == 2U);
    CHECK(sht30_set_heater(&fixture.device, true) == SHT30_OK);
    CHECK(last_command(&fixture.mock) == 0x306DU);
    CHECK(sht30_set_heater(&fixture.device, false) == SHT30_OK);
    CHECK(last_command(&fixture.mock) == 0x3066U);
    CHECK(sht30_clear_status(&fixture.device) == SHT30_OK);
    CHECK(last_command(&fixture.mock) == 0x3041U);
    return 0;
}

static int test_result_strings(void)
{
    CHECK(strcmp(sht30_result_string(SHT30_OK), "ok") == 0);
    CHECK(strcmp(sht30_result_string(SHT30_ERROR_CRC), "CRC mismatch") == 0);
    CHECK(strcmp(sht30_result_string((sht30_result_t)-99), "unknown error") == 0);
    return 0;
}

typedef int (*test_fn_t)(void);

typedef struct {
    const char *name;
    test_fn_t function;
} test_case_t;

int main(void)
{
    static const test_case_t tests[] = {
        {"CRC reference vector", test_crc_reference_vector},
        {"initialization validation", test_init_validation},
        {"single-shot high repeatability", test_single_shot_high_repeatability},
        {"single-shot conversion limits", test_single_shot_conversion_limits},
        {"CRC failure handling", test_crc_failure_does_not_update_output},
        {"bus errors and arguments", test_bus_errors_and_arguments},
        {"status reset and heater", test_status_reset_and_heater},
        {"result strings", test_result_strings},
    };
    const size_t count = sizeof(tests) / sizeof(tests[0]);
    size_t i;

    for (i = 0U; i < count; ++i) {
        if (tests[i].function() != 0) {
            (void)fprintf(stderr, "[FAIL] %s\n", tests[i].name);
            return 1;
        }
        (void)printf("[PASS] %s\n", tests[i].name);
    }
    (void)printf("%zu tests passed\n", count);
    return 0;
}
