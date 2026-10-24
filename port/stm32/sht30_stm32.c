#include "sht30_stm32.h"

static int stm32_write(void *context, uint8_t address, const uint8_t *data, size_t length)
{
    sht30_stm32_t *port = context;

    if ((port == NULL) || (data == NULL) || (length > 0xFFFFU)) {
        return -1;
    }
    return (HAL_I2C_Master_Transmit(port->hi2c,
                                    (uint16_t)((uint16_t)address << 1),
                                    (uint8_t *)data,
                                    (uint16_t)length,
                                    port->timeout_ms) == HAL_OK) ? 0 : -1;
}

static int stm32_read(void *context, uint8_t address, uint8_t *data, size_t length)
{
    sht30_stm32_t *port = context;

    if ((port == NULL) || (data == NULL) || (length > 0xFFFFU)) {
        return -1;
    }
    return (HAL_I2C_Master_Receive(port->hi2c,
                                   (uint16_t)((uint16_t)address << 1),
                                   data,
                                   (uint16_t)length,
                                   port->timeout_ms) == HAL_OK) ? 0 : -1;
}

static void stm32_delay_ms(void *context, uint32_t milliseconds)
{
    (void)context;
    HAL_Delay(milliseconds);
}

sht30_result_t sht30_stm32_init(sht30_t *device,
                                sht30_stm32_t *port,
                                I2C_HandleTypeDef *hi2c,
                                uint8_t address,
                                uint32_t timeout_ms)
{
    sht30_config_t config;

    if ((device == NULL) || (port == NULL) || (hi2c == NULL) || (timeout_ms == 0U)) {
        return SHT30_ERROR_INVALID_ARGUMENT;
    }

    port->hi2c = hi2c;
    port->timeout_ms = timeout_ms;
    config.bus.context = port;
    config.bus.write = stm32_write;
    config.bus.read = stm32_read;
    config.bus.delay_ms = stm32_delay_ms;
    config.address = address;
    return sht30_init(device, &config);
}
