#include "sht30_stm32.h"

/* Add this file to a CubeMX-generated STM32F103 I2C project. */
static sht30_t s_sht30;
static sht30_stm32_t s_sht30_port;

/* Inspect these variables with the debugger or add your own logger. */
volatile int32_t g_sht30_temperature_millic;
volatile uint32_t g_sht30_humidity_milli_percent;
volatile sht30_result_t g_sht30_last_result;

sht30_result_t sht30_example_init(I2C_HandleTypeDef *hi2c)
{
    g_sht30_last_result = sht30_stm32_init(&s_sht30,
                                           &s_sht30_port,
                                           hi2c,
                                           SHT30_I2C_ADDRESS_DEFAULT,
                                           SHT30_STM32_DEFAULT_TIMEOUT_MS);
    return g_sht30_last_result;
}

void sht30_example_poll(void)
{
    sht30_data_t data;

    g_sht30_last_result = sht30_read_single_shot(&s_sht30, SHT30_REPEATABILITY_HIGH, &data);
    if (g_sht30_last_result == SHT30_OK) {
        g_sht30_temperature_millic = data.temperature_millic;
        g_sht30_humidity_milli_percent = data.humidity_milli_percent;
    }
    HAL_Delay(1000U);
}
