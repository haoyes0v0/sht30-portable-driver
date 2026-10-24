# sht30-portable-driver

[![CI](https://github.com/haoyes0v0/sht30-portable-driver/actions/workflows/ci.yml/badge.svg)](https://github.com/haoyes0v0/sht30-portable-driver/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Version](https://img.shields.io/badge/version-0.1.0-green.svg)](CHANGELOG.md)

[中文说明](README_zh-CN.md)

A small, dependency-free C99 driver for Sensirion SHT30 temperature and humidity sensors. The core is independent of any MCU vendor HAL and uses caller-provided I2C and delay callbacks. An STM32F1 HAL adapter and STM32F103 integration example are included.

## Why this driver?

- **Portable core:** no STM32 calls, dynamic allocation, RTOS, or global state in the core driver.
- **Fixed-point results:** temperature in `0.001 °C` and humidity in `0.001 %RH`.
- **Defensive communication:** validates both Sensirion CRC-8 bytes and keeps output unchanged on failure.
- **Testable without hardware:** mock I2C tests cover conversion limits, commands, timing, CRC errors, and bus failures.
- **Reproducible engineering:** host unit tests, strict compiler warnings, static analysis, and Cortex-M3 cross-compilation in CI.

> Verification scope: automated tests and cross-compilation do not replace physical sensor validation. See the explicit [hardware validation record](docs/hardware.md).

## Features in v0.1.0

- Single-shot measurement with high, medium, or low repeatability; clock stretching disabled.
- Configurable 7-bit address (`0x44` or `0x45`).
- Soft reset, raw status-register read, status clear, and heater control.
- Readable error codes and a minimal STM32F1 blocking-HAL adapter.

Periodic measurement, ART mode, and non-blocking/DMA operation are intentionally outside the first release.

## Quick start

### Portable API

```c
#include "sht30.h"

static sht30_t sensor;

void sensor_init(const sht30_bus_t *platform_bus)
{
    sht30_config_t config = {
        .bus = *platform_bus, /* Your write, read, delay_ms, and context. */
        .address = SHT30_I2C_ADDRESS_DEFAULT
    };
    (void)sht30_init(&sensor, &config);
}

int read_sensor(void)
{
    sht30_data_t data;
    sht30_result_t result = sht30_read_single_shot(
        &sensor, SHT30_REPEATABILITY_HIGH, &data);

    if (result != SHT30_OK) {
        return (int)result;
    }
    /* data.temperature_millic: 25325 means 25.325 °C. */
    /* data.humidity_milli_percent: 48120 means 48.120 %RH. */
    return 0;
}
```

The platform callbacks return `0` on success and non-zero on failure. A single-shot read blocks for the conversion delay plus I2C transfers.

### STM32F103 HAL

Configure I2C1 in CubeMX, then add `src/`, `include/`, and `port/stm32/` to the project. See the [CubeMX integration guide](docs/stm32cubemx.md).

```c
#include "sht30_stm32.h"

extern I2C_HandleTypeDef hi2c1;
static sht30_t sensor;
static sht30_stm32_t sensor_port;

void app_init(void)
{
    (void)sht30_stm32_init(&sensor, &sensor_port, &hi2c1,
                           SHT30_I2C_ADDRESS_DEFAULT, 100U);
}

void app_loop(void)
{
    sht30_data_t data;
    if (sht30_read_single_shot(&sensor, SHT30_REPEATABILITY_HIGH, &data) == SHT30_OK) {
        /* Consume data here. */
    }
    HAL_Delay(1000U);
}
```

The adapter shifts the 7-bit address for STM32 HAL internally. Do not pass `0x88`/`0x89`.

## Build and test on a host

Requirements: CMake 3.16+, a C99 compiler, and no third-party test framework. On Windows, Ninja is recommended. Some legacy MinGW tools cannot handle non-ASCII working directories; use an ASCII path if your toolchain reports path errors.

```bash
cmake -S . -B build -DSHT30_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

To reuse the core in another CMake project:

```cmake
add_subdirectory(path/to/sht30-portable-driver)
target_link_libraries(your_firmware PRIVATE sht30::sht30)
```

## Command coverage

| Operation | Command | CRC checked |
|---|---|---|
| Single-shot high / medium / low | `0x2400` / `0x240B` / `0x2416` | Temperature and humidity |
| Soft reset | `0x30A2` | Not applicable |
| Read status | `0xF32D` | Status word |
| Clear status | `0x3041` | Not applicable |
| Heater on / off | `0x306D` / `0x3066` | Not applicable |

## Repository layout

```text
include/                 Public portable API
src/                     Vendor-independent implementation
port/stm32/              STM32F1 HAL adapter
examples/stm32f103/      CubeMX integration example
tests/                   Mock I2C unit tests and compile-only HAL shim
docs/                    Wiring, integration, and verification guides
cmake/                   ARM cross-compilation toolchain file
.github/workflows/       Host and ARM CI
```

## Roadmap

- [x] Portable single-shot driver and fixed-point conversion
- [x] Mock I2C failure-injection tests
- [x] STM32F1 HAL adapter and Cortex-M3 compile check
- [ ] Physical STM32F103 + SHT30 validation record
- [ ] Periodic measurement and fetch-data commands
- [ ] Non-blocking transaction support
- [ ] Additional platform adapters driven by user needs

## Contributing and license

Small issues and focused pull requests are welcome. Keep the core platform-independent and run all host tests before submitting. This project is released under the [MIT License](LICENSE).
