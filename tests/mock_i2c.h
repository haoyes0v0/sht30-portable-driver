#ifndef MOCK_I2C_H
#define MOCK_I2C_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t rx_data[16];
    size_t rx_length;
    uint8_t last_write[8];
    size_t last_write_length;
    uint8_t last_address;
    uint32_t last_delay_ms;
    unsigned int write_count;
    unsigned int read_count;
    unsigned int delay_count;
    int write_result;
    int read_result;
} mock_i2c_t;

void mock_i2c_reset(mock_i2c_t *mock);
int mock_i2c_write(void *context, uint8_t address, const uint8_t *data, size_t length);
int mock_i2c_read(void *context, uint8_t address, uint8_t *data, size_t length);
void mock_i2c_delay_ms(void *context, uint32_t milliseconds);
void mock_i2c_set_measurement(mock_i2c_t *mock, uint16_t temperature, uint16_t humidity);
void mock_i2c_set_status(mock_i2c_t *mock, uint16_t status);

#endif
