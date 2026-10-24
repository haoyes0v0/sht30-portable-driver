# SHT30 可移植驱动库

一个不依赖厂商 HAL 的 C99 SHT30 温湿度传感器驱动。核心代码通过回调使用平台提供的 I2C 和延时能力，仓库同时提供 STM32F1 HAL 适配层与 STM32F103 接入示例。

## 首版特性

- 高、中、低重复精度的单次测量，不使用时钟延展。
- 支持 `0x44`、`0x45` 两个 7 位地址。
- 温度采用 `0.001 °C`、湿度采用 `0.001 %RH` 固定点单位，无需浮点转换。
- 分别校验温度和湿度 CRC；读取失败或 CRC 错误时不覆盖调用者原数据。
- 支持软复位、状态读取/清除和内部加热器控制。
- 核心不使用动态内存、全局状态或 RTOS。
- 模拟 I2C 测试覆盖正常数据、换算边界、命令、延时、总线错误和 CRC 错误。

## 实测与验证边界

主机单元测试和 ARM 交叉编译只能证明软件逻辑与目标架构可编译，不能替代实物验证。STM32F103 + SHT30 的接线和实测状态记录见 [硬件验证文档](docs/hardware.md)，未实测前不会标记为硬件通过。

## 在 STM32F103 中使用

1. 用 CubeMX 配置 I2C1，例如 `PB6=SCL`、`PB7=SDA`，标准模式 100 kHz。
2. 将 `src/sht30.c`、`include/sht30.h`、`port/stm32/sht30_stm32.c/.h` 加入工程并设置头文件路径。
3. 按 [硬件文档](docs/hardware.md) 完成 3.3 V 接线和 I2C 上拉。
4. 初始化 `hi2c1` 后绑定驱动：

```c
#include "sht30_stm32.h"

extern I2C_HandleTypeDef hi2c1;
static sht30_t sensor;
static sht30_stm32_t sensor_port;
static sht30_data_t latest_data;

void app_init(void)
{
    (void)sht30_stm32_init(&sensor, &sensor_port, &hi2c1,
                           SHT30_I2C_ADDRESS_DEFAULT, 100U);
}

void app_loop(void)
{
    sht30_result_t result = sht30_read_single_shot(
        &sensor, SHT30_REPEATABILITY_HIGH, &latest_data);
    if (result == SHT30_OK) {
        /* latest_data.temperature_millic = 25325 表示 25.325 °C。 */
        /* latest_data.humidity_milli_percent = 48120 表示 48.120 %RH。 */
    }
    HAL_Delay(1000U);
}
```

STM32 适配层会自动把 7 位地址左移一位，因此不要传入 `0x88` 或 `0x89`。完整步骤见 [CubeMX 接入指南](docs/stm32cubemx.md)。

## 在 Windows/Linux 主机上运行测试

只需要 CMake 3.16+ 和支持 C99 的编译器，无需下载测试框架。Windows 下建议使用 Ninja；部分 MinGW 工具无法处理中文工作路径，如遇路径错误，请将工程放在纯英文路径下：

```bash
cmake -S . -B build -DSHT30_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

作为其他 CMake 工程的子目录使用时，链接 `sht30::sht30` 即可。

## 当前限制与后续方向

首版专注于可移植单次测量，不包含周期测量、ART 模式、DMA 或非阻塞状态机。后续计划按实际使用需求增加周期测量和非阻塞事务，而不是一次堆叠未经测试的功能。

项目采用 [MIT 许可证](LICENSE)，版本变化见 [CHANGELOG](CHANGELOG.md)。
