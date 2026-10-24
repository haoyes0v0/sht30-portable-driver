#include "mock_i2c.h"

#include "sht30.h"

#include <string.h>

void mock_i2c_reset(mock_i2c_t *mock)
{
    memset(mock, 0, sizeof(*mock));
}

int mock_i2c_write(void *context, uint8_t address, const uint8_t *data, size_t length)
{
    mock_i2c_t *mock = context;

    ++mock->write_count;
    mock->last_address = address;
    mock->last_write_length = length;
    memcpy(mock->last_write, data, length);
    return mock->write_result;
}

int mock_i2c_read(void *context, uint8_t address, uint8_t *data, size_t length)
{
    mock_i2c_t *mock = context;

    ++mock->read_count;
    mock->last_address = address;
    if ((mock->read_result != 0) || (mock->rx_length != length)) {
        return -1;
    }
    memcpy(data, mock->rx_data, length);
    return 0;
}

void mock_i2c_delay_ms(void *context, uint32_t milliseconds)
{
    mock_i2c_t *mock = context;

    ++mock->delay_count;
    mock->last_delay_ms = milliseconds;
}

static void set_word_with_crc(uint8_t *destination, uint16_t value)
{
    destination[0] = (uint8_t)(value >> 8);
    destination[1] = (uint8_t)(value & 0xFFU);
    destination[2] = sht30_crc8(destination, 2U);
}

void mock_i2c_set_measurement(mock_i2c_t *mock, uint16_t temperature, uint16_t humidity)
{
    set_word_with_crc(&mock->rx_data[0], temperature);
    set_word_with_crc(&mock->rx_data[3], humidity);
    mock->rx_length = 6U;
}

void mock_i2c_set_status(mock_i2c_t *mock, uint16_t status)
{
    set_word_with_crc(mock->rx_data, status);
    mock->rx_length = 3U;
}
