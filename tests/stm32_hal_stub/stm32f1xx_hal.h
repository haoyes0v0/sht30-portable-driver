#ifndef STM32F1XX_HAL_H
#define STM32F1XX_HAL_H

/* Compile-only CI shim. Production firmware must use the real STM32CubeF1 HAL. */
#include <stdint.h>

typedef struct {
    uint32_t unused;
} I2C_HandleTypeDef;

typedef enum {
    HAL_OK = 0,
    HAL_ERROR = 1,
    HAL_BUSY = 2,
    HAL_TIMEOUT = 3
} HAL_StatusTypeDef;

HAL_StatusTypeDef HAL_I2C_Master_Transmit(I2C_HandleTypeDef *hi2c,
                                          uint16_t device_address,
                                          uint8_t *data,
                                          uint16_t size,
                                          uint32_t timeout);
HAL_StatusTypeDef HAL_I2C_Master_Receive(I2C_HandleTypeDef *hi2c,
                                         uint16_t device_address,
                                         uint8_t *data,
                                         uint16_t size,
                                         uint32_t timeout);
void HAL_Delay(uint32_t milliseconds);

#endif
