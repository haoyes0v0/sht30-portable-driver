#ifndef SHT30_H
#define SHT30_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SHT30_VERSION_MAJOR 0
#define SHT30_VERSION_MINOR 1
#define SHT30_VERSION_PATCH 0

#define SHT30_I2C_ADDRESS_DEFAULT 0x44U
#define SHT30_I2C_ADDRESS_ALT     0x45U

typedef enum {
    SHT30_OK = 0,
    SHT30_ERROR_INVALID_ARGUMENT = -1,
    SHT30_ERROR_BUS_WRITE = -2,
    SHT30_ERROR_BUS_READ = -3,
    SHT30_ERROR_CRC = -4
} sht30_result_t;

typedef enum {
    SHT30_REPEATABILITY_HIGH = 0,
    SHT30_REPEATABILITY_MEDIUM = 1,
    SHT30_REPEATABILITY_LOW = 2
} sht30_repeatability_t;

typedef int (*sht30_i2c_write_fn)(void *context,
                                  uint8_t address,
                                  const uint8_t *data,
                                  size_t length);
typedef int (*sht30_i2c_read_fn)(void *context,
                                 uint8_t address,
                                 uint8_t *data,
                                 size_t length);
typedef void (*sht30_delay_ms_fn)(void *context, uint32_t milliseconds);

typedef struct {
    void *context;
    sht30_i2c_write_fn write;
    sht30_i2c_read_fn read;
    sht30_delay_ms_fn delay_ms;
} sht30_bus_t;

typedef struct {
    sht30_bus_t bus;
    uint8_t address; /**< 7-bit I2C address: 0x44 or 0x45. */
} sht30_config_t;

typedef struct {
    sht30_bus_t bus;
    uint8_t address;
    bool initialized;
} sht30_t;

typedef struct {
    int32_t temperature_millic;       /**< Degrees Celsius multiplied by 1000. */
    uint32_t humidity_milli_percent;  /**< Relative humidity (%) multiplied by 1000. */
} sht30_data_t;

/**
 * Initialize a driver instance. This function does not access the bus.
 * All callbacks must remain valid for the lifetime of the device.
 */
sht30_result_t sht30_init(sht30_t *device, const sht30_config_t *config);

/**
 * Perform one blocking single-shot measurement without clock stretching.
 * The configured delay callback waits for conversion before reading.
 */
sht30_result_t sht30_read_single_shot(sht30_t *device,
                                      sht30_repeatability_t repeatability,
                                      sht30_data_t *data);

/** Issue a soft reset and wait two milliseconds for sensor initialization. */
sht30_result_t sht30_soft_reset(sht30_t *device);

/** Read the raw 16-bit status register and verify its CRC. */
sht30_result_t sht30_read_status(sht30_t *device, uint16_t *status);

/** Clear the status register. */
sht30_result_t sht30_clear_status(sht30_t *device);

/** Enable or disable the internal heater. */
sht30_result_t sht30_set_heater(sht30_t *device, bool enabled);

/** Sensirion SHT3x CRC-8: polynomial 0x31, initial value 0xFF. */
uint8_t sht30_crc8(const uint8_t *data, size_t length);

/** Return a static string for logging; never returns NULL. */
const char *sht30_result_string(sht30_result_t result);

#ifdef __cplusplus
}
#endif

#endif
