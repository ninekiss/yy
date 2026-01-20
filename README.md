# ESP32-S3 智能自动浇水系统 (Smart Watering System)

这是一个基于 ESP32-S3 的企业级高可靠性自动浇水系统。项目采用**模块化架构设计**，实现了逻辑与硬件的彻底解耦。不仅包含完整的**单元测试**体系，更集成了 **MQTT 物联网远程控制**、**OTA 空中升级**、**生命周期管理**及**断电记忆**功能。

## ✨ 主要特性

- **精准定时**：通过 NTP 同步网络时间，并在指定时间（如凌晨 2:00）自动触发。
- **断电记忆 (Persistence)**：利用 NVS (Preferences) 保存浇水次数、日期及系统状态（Kill/Active），断电重启后自动恢复进度。
- **IoT 远程控制 (MQTT)**：
  - **实时指令**：支持 `start` / `stop` / `reset` / `kill` / `revive` / `info` 六大指令。
  - **状态反馈**：实时上报运行状态（如 `Manual Start [1/18]`, `Aborted`, `Done`）。
  - **自检上报**：设备启动或重连时，自动推送 IP、进度、配置参数等 JSON 报告。
  - **心跳维持**：采用 Yield Callback 机制，防止长时浇水导致 MQTT 断连。
- **空中升级 (OTA)**：支持通过 WiFi 无线更新固件，无需 USB 连接，包含密码安全验证。
- **并发安全 (Concurrency Safety)**：
  - **状态锁**：防止在浇水过程中重复触发指令。
  - **冲突保护**：防止在系统忙碌时误操作 Reset。
- **逻辑解耦**：核心算法 (`WateringCore`) 与硬件驱动 (`WateringSystem`) 分离，互不依赖。
- **双环境测试**：
  - **Native**: 在电脑上验证核心数学逻辑。
  - **Embedded**: 在 ESP32 真机上验证 NVS 读写、GPIO 控制及系统集成逻辑。

## 🛠️ 硬件要求

- **开发板**: ESP32-S3 DevKitC-1 (支持 N16R8 高配版配置)
- **执行器**: 5V 水泵 + 继电器模块 (低电平触发)
- **电源**: 5V 插座供电 (**禁止**使用充电宝，以免因电流过小自动关机)
- **指示灯**: 板载 WS2812B RGB 灯 (默认 GPIO 48)

### 接线说明

| 组件            | ESP32 引脚 | 备注                       |
| :-------------- | :--------- | :------------------------- |
| **继电器 (IN)** | GPIO 4     | 控制水泵 (可在配置中修改)  |
| **RGB 灯 (DI)** | GPIO 48    | 这里的引脚号视具体板子而定 |
| **继电器 VCC**  | 5V         |                            |
| **继电器 GND**  | GND        |                            |

## 📂 项目结构

```text
├── lib/
│   ├── WateringCore/      # [大脑] 纯数学逻辑，无硬件依赖
│   ├── NetworkManager/    # [网络] WiFi 连接管理
│   ├── MqttManager/       # [通信] MQTT 协议封装，含自动重连机制
│   ├── OtaManager/        # [维护] OTA 空中升级管理
│   ├── WateringSystem/    # [四肢] 业务总管，含 NVS 存储、状态锁、回调接口
│   └── TestIndicator/     # [工具] 测试结果的灯光反馈
├── src/
│   └── main.cpp           # [入口] 依赖注入 (Dependency Injection) 与模块组装
├── test/
│   ├── common/            # 公共测试逻辑 (Native 与 Embedded 共享)
│   ├── test_native/       # 本机测试入口 (CI/CD 友好)
│   └── test_embedded/     # 硬件集成测试入口 (NVS/Kill/Reset 测试)
├── secrets.ini            # 敏感配置文件 (需手动创建)
└── platformio.ini         # PIO 配置文件
```

## 🚀 快速开始

### 1. 配置文件

在项目根目录新建 `secrets.ini` 文件（此文件已在 `.gitignore` 中被忽略）。
**注意**：所有运行参数均在此配置，无需修改代码。

```ini
[secrets]
; --- 网络配置 ---
wifi_ssid = 你的WiFi名称
wifi_pass = 你的WiFi密码

; --- MQTT配置 (推荐使用 broker.emqx.io 测试) ---
mqtt_server = broker.emqx.io
mqtt_port = 1883
mqtt_user = 
mqtt_pass = 
; 定义你的唯一 Topic (建议加上随机后缀防止冲突)
mqtt_topic_cmd = your_unique_id/watering/cmd
mqtt_topic_status = your_unique_id/watering/status

; --- OTA 安全 ---
ota_password = admin123

; --- 系统运行参数 ---
system_enable_nvs = 1              ; 1=开启断电记忆, 0=关闭(调试用)
system_manual_test = 0             ; 1=开启手动测试代码块, 0=关闭
system_watering_pin = 4            ; 继电器引脚
system_watering_duration = 38      ; 单次浇水时长(秒)
system_watering_count = 18         ; 总浇水次数
system_watering_interval_days = 3  ; 间隔天数
system_watering_start_hour = 2     ; 开始时间(小时)
system_watering_start_minute = 0   ; 开始时间(分钟)
```

### 2. 编译与上传

使用 PlatformIO 左侧任务栏：

- 选择 `env:esp32-s3-devkitc-1` -> `General` -> `Upload`。
- 或点击底部状态栏的 `→` 箭头。

### 3. OTA 无线升级 (Over-The-Air)

无需 USB 线，支持通过 WiFi 远程更新固件。

1.  **首次烧录**: 必须使用 USB 线，通过 `env:esp32-s3-devkitc-1` 上传。
2.  **获取 IP**: 首次运行后，在串口监视器或 MQTT `boot` 消息中查看设备 IP。
3.  **配置 OTA**: 修改 `platformio.ini` 中的 `upload_port` 为设备 IP。
4.  **无线上传**: 选择 `env:esp32-ota` 环境，点击 Upload。

## 🛡️ 系统稳定性机制

为了确保设备长期无人值守运行，系统内置了多重保障：

1.  **硬件看门狗 (WDT)**:
    - 超时时间设为 30 秒。
    - 如果系统死机或卡死，会自动重启。
    - 在浇水延时期间通过 `Yield Callback` 持续喂狗。
2.  **断网重连**:
    - `NetworkManager` 负责 WiFi 掉线后的自动重连（含冷却机制，防止死循环）。
    - `MqttManager` 负责 MQTT 断开后的自动重连。
3.  **并发锁**:
    - 防止指令冲突（如在浇水时 Reset）。
    - 防止递归调用。

## 🎮 MQTT 指令手册

通过 MQTT 客户端向 Topic `.../watering/cmd` 发送以下文本指令：

| 指令 | 作用 | 详细说明 |
| :--- | :--- | :--- |
| `start` | **立即浇水** | 强制开始一次浇水循环（不计入自动任务次数）。如果系统正在忙或被 Kill，则忽略。 |
| `stop` | **紧急停止** | 立即中断当前的浇水进程。本次任务标记为 Aborted。 |
| `reset` | **任务重置** | 将浇水计数器归零 (`Count=0`)。**仅当系统空闲时有效**。 |
| `kill` | **系统停用** | 彻底禁用自动任务。状态写入 Flash，重启后依然生效。用于长期维护。 |
| `revive` | **系统复活** | 恢复 `kill` 状态，系统重新上线。 |
| `info` | **查询状态** | 设备返回包含 IP、进度、配置参数的 JSON 数据。 |

## 🧪 测试指南

本项目支持 TDD (测试驱动开发)。

### 1. 运行逻辑测试 (Native)

_用于快速验证算法准确性（跨年逻辑、间隔天数等）。_

- 运行命令：`pio test -e native`

### 2. 运行系统集成测试 (Embedded)

_用于验证 NVS 持久化、Kill 开关逻辑、Reset 逻辑及硬件联动。_

- 运行命令：`pio test -e esp32-s3-devkitc-1`
- **反馈效果**：
  - 🟢 **绿灯常亮**：所有测试通过。
  - 🔴 **红灯闪烁**：有测试失败。

### 3. 手动测试代码
源码中包含部分手动验证代码（用于调试特定硬件行为），已被注释包裹。
如需启用，请在 `secrets.ini` 中设置 `system_manual_test = 1` 或搜索代码标记：
```cpp
// MTEST:START
// ... code ...
// MTEST:END
```

## ⚙️ 默认参数

- **触发时间**: 凌晨 02:00
- **间隔周期**: 每 3 天
- **单次时长**: 38 秒
- **最大次数**: 14 次
- **MQTT 心跳**: 60 秒 (防止长时浇水断连)


