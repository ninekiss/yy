# ESP32-S3 智能自动浇水系统 (Smart Watering System)

这是一个基于 ESP32-S3 的高可靠性自动浇水系统。项目采用**模块化架构设计**，实现了逻辑与硬件的彻底解耦，并包含完整的**单元测试**（Native & Embedded）体系。

## ✨ 主要特性

- **精准定时**：通过 NTP 同步网络时间，并在指定时间（如凌晨 2:00）触发任务。
- **断电记忆 (Persistence)**：利用 NVS (Preferences) 保存浇水次数和日期，防止断电导致任务重置。
- **逻辑解耦**：核心算法 (`WateringCore`) 与硬件驱动 (`WateringSystem`) 分离，支持本机极速测试。
- **双环境测试**：
  - **Native**: 在电脑上验证核心数学逻辑。
  - **Embedded**: 在 ESP32 上验证硬件联动和灯光反馈。
- **状态反馈**：集成 NeoPixel RGB 灯，测试通过显示绿灯，失败闪烁红灯。
- **安全配置**：WiFi 凭证通过 `secrets.ini` 注入，不硬编码在源码中。

## 🛠️ 硬件要求

- **开发板**: ESP32-S3 DevKitC-1 (或兼容板)
- **执行器**: 5V 水泵 + 继电器模块 (低电平触发)
- **电源**: 5V USB 供电 (推荐插座供电，**不建议**使用移动电源)
- **指示灯**: 板载 WS2812B RGB 灯 (默认 GPIO 48)

### 接线说明

| 组件            | ESP32 引脚 | 备注                       |
| :-------------- | :--------- | :------------------------- |
| **继电器 (IN)** | GPIO 4     | 控制水泵                   |
| **RGB 灯 (DI)** | GPIO 48    | 这里的引脚号视具体板子而定 |
| **继电器 VCC**  | 5V         |                            |
| **继电器 GND**  | GND        |                            |

## 📂 项目结构

```text
├── lib/
│   ├── WateringCore/      # [大脑] 纯数学逻辑，无硬件依赖
│   ├── NetworkManager/    # [网络] WiFi 连接管理
│   ├── TimeManager/       # [时间] NTP 同步管理
│   ├── WateringSystem/    # [四肢] 业务总管，整合逻辑与硬件，含 NVS 存储
│   └── TestIndicator/     # [工具] 测试结果的灯光反馈
├── src/
│   └── main.cpp           # [入口] 组装各个模块
├── test/
│   ├── common/            # 公共测试逻辑 (Native 与 Embedded 共享)
│   ├── test_native/       # 本机测试入口
│   └── test_embedded/     # 硬件测试入口
├── secrets.ini            # 敏感配置文件 (需手动创建)
└── platformio.ini         # PIO 配置文件
```

## 🚀 快速开始

### 1. 配置文件

在项目根目录新建 `secrets.ini` 文件（此文件已在 `.gitignore` 中被忽略）：

```ini
[secrets]
wifi_ssid = 你的WiFi名称
wifi_pass = 你的WiFi密码
```

或复制 `secrets.ini.example` 并修改其中的内容。

### 2. 编译与上传

使用 PlatformIO 左侧任务栏：

- 选择 `env:esp32-s3-devkitc-1` -> `General` -> `Upload`。
- 或点击底部状态栏的 `→` 箭头。

## 🧪 测试指南

本项目支持 TDD (测试驱动开发)。

### 运行逻辑测试 (Native)

_用于快速验证算法准确性（跨年逻辑、间隔天数等）。_

- 运行命令：`pio test -e native`
- 或在 PIO 侧边栏选择 `native` -> `Advanced` -> `Test`。

### 运行硬件测试 (Embedded)

_用于验证硬件联动及 RGB 灯光反馈。_

- 运行命令：`pio test -e esp32-s3-devkitc-1`
- **反馈效果**：
  - 🟢 **绿灯常亮**：所有测试通过。
  - 🔴 **红灯闪烁**：有测试失败。

## ⚙️ 默认参数

- **触发时间**: 凌晨 02:00
- **间隔周期**: 每 3 天
- **单次时长**: 38 秒
- **最大次数**: 18 次
