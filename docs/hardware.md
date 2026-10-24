# STM32F103 + SHT30 wiring and validation

## Reference wiring

The example uses an STM32F103 board with I2C1 on PB6/PB7 and an SHT30 module whose ADDR pin is tied to GND.

| SHT30 | STM32F103 / supply | Note |
|---|---|---|
| VCC | 3.3 V | Recommended for a direct 3.3 V I2C bus. Check your module schematic. |
| GND | GND | Common ground is required. |
| SCL | PB6 / I2C1_SCL | Add pull-up if the module does not include one. |
| SDA | PB7 / I2C1_SDA | Add pull-up if the module does not include one. |
| ADDR | GND | Selects 7-bit address `0x44`; tie high for `0x45`. |

Use approximately 4.7 kOhm pull-ups from SDA/SCL to 3.3 V when the module has none. Do not pull the bus up to 5 V. Many breakout modules already include pull-ups, level circuitry, or an ADDR connection; their schematics take precedence.

## CubeMX baseline

- I2C1 enabled on PB6/PB7.
- I2C standard mode, 100 kHz.
- 7-bit addressing.
- GPIO open-drain with suitable external/module pull-ups.
- `hi2c1` initialized before `sht30_stm32_init`.

## Physical validation checklist

1. Run all host tests first.
2. Confirm the sensor is not being pulled up to 5 V and verify the module pin order before powering it.
3. Add the driver and `examples/stm32f103/sht30_example.c` to the CubeMX project.
4. Call `sht30_example_init(&hi2c1)` after `MX_I2C1_Init()`.
5. Call `sht30_example_poll()` repeatedly from the main loop.
6. Watch `g_sht30_last_result`, `g_sht30_temperature_millic`, and `g_sht30_humidity_milli_percent` in the debugger.
7. Confirm plausible room-temperature/humidity readings and a gradual response when warming the sensor with a finger.
8. Disconnect SDA as a controlled negative test; the driver should return an I2C error rather than publish stale data as a new valid sample. Restore wiring afterward.

## Validation record

| Date | Board / MCU | Sensor module | Firmware commit | Result | Notes |
|---|---|---|---|---|---|
| Pending | STM32F103, exact board to be recorded | SHT30, exact module to be recorded | To be recorded | Not yet physically validated | Host tests and ARM compilation are tracked separately in CI. |

Physical readings and any observed limitations should be added here only after an actual test. Do not convert CI success into a hardware-validation claim.
