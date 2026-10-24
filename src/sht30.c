#include "sht30.h"

#include <string.h>

#define SHT30_CMD_SINGLE_HIGH   0x2400U
#define SHT30_CMD_SINGLE_MEDIUM 0x240BU
#define SHT30_CMD_SINGLE_LOW    0x2416U
#define SHT30_CMD_SOFT_RESET    0x30A2U
#define SHT30_CMD_HEATER_ENABLE 0x306DU
#define SHT30_CMD_HEATER_DISABLE 0x3066U
#define SHT30_CMD_READ_STATUS   0xF32DU
#define SHT30_CMD_CLEAR_STATUS  0x3041U

static bool device_is_valid(const sht30_t *device)
{
    return (device != NULL) && device->initialized;
}

static sht30_result_t write_command(sht30_t *device, uint16_t command)
{
    uint8_t buffer[2];

    if (!device_is_valid(device)) {
        return SHT30_ERROR_INVALID_ARGUMENT;
    }

    buffer[0] = (uint8_t)(command >> 8);
    buffer[1] = (uint8_t)(command & 0xFFU);
    if (device->bus.write(device->bus.context, device->address, buffer, sizeof(buffer)) != 0) {
        return SHT30_ERROR_BUS_WRITE;
    }
    return SHT30_OK;
}

static sht30_result_t read_bytes(sht30_t *device, uint8_t *data, size_t length)
{
    if (!device_is_valid(device) || (data == NULL)) {
        return SHT30_ERROR_INVALID_ARGUMENT;
    }
    if (device->bus.read(device->bus.context, device->address, data, length) != 0) {
        return SHT30_ERROR_BUS_READ;
    }
    return SHT30_OK;
}

uint8_t sht30_crc8(const uint8_t *data, size_t length)
{
    uint8_t crc = 0xFFU;
    size_t i;
    uint8_t bit;

    if (data == NULL) {
        return 0U;
    }

    for (i = 0U; i < length; ++i) {
        crc ^= data[i];
        for (bit = 0U; bit < 8U; ++bit) {
            if ((crc & 0x80U) != 0U) {
                crc = (uint8_t)((uint8_t)(crc << 1) ^ 0x31U);
            } else {
                crc = (uint8_t)(crc << 1);
            }
        }
    }
    return crc;
}

sht30_result_t sht30_init(sht30_t *device, const sht30_config_t *config)
{
    if ((device == NULL) || (config == NULL) || (config->bus.write == NULL) ||
        (config->bus.read == NULL) || (config->bus.delay_ms == NULL) ||
        ((config->address != SHT30_I2C_ADDRESS_DEFAULT) &&
         (config->address != SHT30_I2C_ADDRESS_ALT))) {
        return SHT30_ERROR_INVALID_ARGUMENT;
    }

    memset(device, 0, sizeof(*device));
    device->bus = config->bus;
    device->address = config->address;
    device->initialized = true;
    return SHT30_OK;
}

sht30_result_t sht30_read_single_shot(sht30_t *device,
                                      sht30_repeatability_t repeatability,
                                      sht30_data_t *data)
{
    uint16_t command;
    uint32_t conversion_time_ms;
    uint8_t frame[6];
    uint16_t raw_temperature;
    uint16_t raw_humidity;
    sht30_result_t result;
    sht30_data_t converted;

    if (!device_is_valid(device) || (data == NULL)) {
        return SHT30_ERROR_INVALID_ARGUMENT;
    }

    switch (repeatability) {
    case SHT30_REPEATABILITY_HIGH:
        command = SHT30_CMD_SINGLE_HIGH;
        conversion_time_ms = 16U;
        break;
    case SHT30_REPEATABILITY_MEDIUM:
        command = SHT30_CMD_SINGLE_MEDIUM;
        conversion_time_ms = 7U;
        break;
    case SHT30_REPEATABILITY_LOW:
        command = SHT30_CMD_SINGLE_LOW;
        conversion_time_ms = 5U;
        break;
    default:
        return SHT30_ERROR_INVALID_ARGUMENT;
    }

    result = write_command(device, command);
    if (result != SHT30_OK) {
        return result;
    }
    device->bus.delay_ms(device->bus.context, conversion_time_ms);
    result = read_bytes(device, frame, sizeof(frame));
    if (result != SHT30_OK) {
        return result;
    }

    if ((sht30_crc8(&frame[0], 2U) != frame[2]) ||
        (sht30_crc8(&frame[3], 2U) != frame[5])) {
        return SHT30_ERROR_CRC;
    }

    raw_temperature = ((uint16_t)frame[0] << 8) | frame[1];
    raw_humidity = ((uint16_t)frame[3] << 8) | frame[4];
    converted.temperature_millic =
        (int32_t)(((int64_t)raw_temperature * 175000) / 65535) - 45000;
    converted.humidity_milli_percent =
        (uint32_t)(((uint64_t)raw_humidity * 100000U) / 65535U);
    *data = converted;
    return SHT30_OK;
}

sht30_result_t sht30_soft_reset(sht30_t *device)
{
    sht30_result_t result = write_command(device, SHT30_CMD_SOFT_RESET);

    if (result == SHT30_OK) {
        device->bus.delay_ms(device->bus.context, 2U);
    }
    return result;
}

sht30_result_t sht30_read_status(sht30_t *device, uint16_t *status)
{
    uint8_t frame[3];
    sht30_result_t result;

    if (!device_is_valid(device) || (status == NULL)) {
        return SHT30_ERROR_INVALID_ARGUMENT;
    }
    result = write_command(device, SHT30_CMD_READ_STATUS);
    if (result != SHT30_OK) {
        return result;
    }
    result = read_bytes(device, frame, sizeof(frame));
    if (result != SHT30_OK) {
        return result;
    }
    if (sht30_crc8(frame, 2U) != frame[2]) {
        return SHT30_ERROR_CRC;
    }
    *status = ((uint16_t)frame[0] << 8) | frame[1];
    return SHT30_OK;
}

sht30_result_t sht30_clear_status(sht30_t *device)
{
    return write_command(device, SHT30_CMD_CLEAR_STATUS);
}

sht30_result_t sht30_set_heater(sht30_t *device, bool enabled)
{
    return write_command(device, enabled ? SHT30_CMD_HEATER_ENABLE : SHT30_CMD_HEATER_DISABLE);
}

const char *sht30_result_string(sht30_result_t result)
{
    switch (result) {
    case SHT30_OK:
        return "ok";
    case SHT30_ERROR_INVALID_ARGUMENT:
        return "invalid argument or uninitialized device";
    case SHT30_ERROR_BUS_WRITE:
        return "I2C write failed";
    case SHT30_ERROR_BUS_READ:
        return "I2C read failed";
    case SHT30_ERROR_CRC:
        return "CRC mismatch";
    default:
        return "unknown error";
    }
}
