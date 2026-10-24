# Changelog

All notable changes to this project will be documented in this file.

The format is based on Keep a Changelog, and this project uses Semantic Versioning.

## [0.1.0] - 2026-10-24

### Added

- Platform-independent C99 SHT30 driver with caller-provided I2C and delay callbacks.
- High/medium/low single-shot measurements with fixed-point conversion and dual CRC validation.
- Soft reset, status read/clear, and heater control.
- STM32F1 blocking-HAL adapter and STM32F103 CubeMX integration example.
- Mock I2C unit tests covering conversion boundaries, invalid arguments, bus failures, and CRC errors.
- Host and Cortex-M3 CI builds, hardware wiring guide, and explicit physical-validation record.
