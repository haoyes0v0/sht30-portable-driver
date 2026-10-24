# STM32CubeMX / STM32CubeIDE integration

This repository intentionally does not duplicate the complete vendor HAL or generated startup files. Create a normal STM32F103 project with STM32CubeMX or CubeIDE, then integrate the small portable driver and adapter.

## 1. Generate the base project

1. Select your exact STM32F103 part number.
2. Enable I2C1. On common boards, use PB6 for SCL and PB7 for SDA.
3. Select I2C standard mode at 100 kHz and 7-bit addressing.
4. Keep the pins open-drain and provide suitable pull-ups to 3.3 V.
5. Generate the project and confirm that `MX_I2C1_Init()` creates `hi2c1`.

## 2. Add the sources

Copy or reference:

- `include/sht30.h`
- `src/sht30.c`
- `port/stm32/sht30_stm32.h`
- `port/stm32/sht30_stm32.c`
- optionally `examples/stm32f103/sht30_example.c`

Add `include/` and `port/stm32/` to the compiler include paths. The adapter must compile against the real `stm32f1xx_hal.h`; the header under `tests/stm32_hal_stub/` is only a compile-only CI shim and must never be copied into firmware.

## 3. Call from the application

After `MX_I2C1_Init()`:

```c
(void)sht30_example_init(&hi2c1);
```

Inside the non-interrupt main loop:

```c
sht30_example_poll();
```

Declare both functions in your application header or add local prototypes. The example stores results in debugger-visible globals and delays for one second per sample. Integrate your own UART logger if desired; logging is deliberately not forced on the portable core.

## 4. Debug checklist

- `SHT30_ERROR_BUS_WRITE`: check power, address, wiring, I2C initialization, and whether SHT30 ACKs the command.
- `SHT30_ERROR_BUS_READ`: check pull-ups, conversion timing, and bus integrity.
- `SHT30_ERROR_CRC`: check for signal integrity problems, voltage mismatch, or an incompatible sensor command sequence.
- Always pass a 7-bit address (`0x44`/`0x45`), not the STM32 HAL's shifted address.
- The current adapter uses blocking HAL calls. Do not call the example from an ISR or a time-critical control loop.
