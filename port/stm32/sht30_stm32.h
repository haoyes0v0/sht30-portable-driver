#ifndef SHT30_STM32_H
#define SHT30_STM32_H

#include "sht30.h"
#include "stm32f1xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SHT30_STM32_DEFAULT_TIMEOUT_MS 100U

typedef struct {
    I2C_HandleTypeDef *hi2c;
    uint32_t timeout_ms;
} sht30_stm32_t;

/**
 * Bind the portable driver to an STM32F1 HAL I2C handle.
 * The handle must already be initialized by CubeMX/HAL before calling this.
 */
sht30_result_t sht30_stm32_init(sht30_t *device,
                                sht30_stm32_t *port,
                                I2C_HandleTypeDef *hi2c,
                                uint8_t address,
                                uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif
