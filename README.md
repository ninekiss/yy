# ESP32-S3 WiFi Web Server

这是一个基于ESP32-S3开发板的WiFi Web服务器项目，支持通过SPIFFS配置文件动态设置WiFi凭据。

## 功能特性

- 连接到WiFi网络
- 启动Web服务器（端口80）
- 显示设备运行时间和IP地址信息
- 通过SPIFFS配置文件管理WiFi凭据

## 配置方法

### 1. 配置WiFi凭据

项目使用SPIFFS文件系统来存储WiFi配置，避免将敏感信息硬编码到固件中。

1. 编辑 `data/config.txt` 文件，填入您的WiFi信息：
   ```
   WIFI_SSID=YourWiFiName
   WIFI_PASSWORD=YourWiFiPassword
   ```

2. 将配置文件上传到ESP32的SPIFFS文件系统：
   ```bash
   platformio run -t uploadfs
   ```

3. 烧录主程序：
   ```bash
   platformio run --target upload
   ```

### 2. 关于环境变量

请注意，ESP32 Arduino环境无法直接读取操作系统的环境变量或项目根目录下的`.env`文件。
本项目中的`.env.example`文件仅作为配置参考模板，实际的配置值需要通过SPIFFS文件系统进行设置。

## 项目结构

- `src/main.cpp` - 主程序文件
- `include/config_manager.h` - 配置管理器头文件
- `lib/config_manager/config_manager.cpp` - 配置管理器实现
- `data/config.txt` - 配置文件模板（上传到SPIFFS）
- `.env.example` - 配置示例文件（仅供参考，实际配置在SPIFFS中）
- `platformio.ini` - PlatformIO项目配置

## 使用说明

1. 首次烧录前，请先上传SPIFFS文件系统中的配置文件
2. 成功连接WiFi后，串口监视器将输出设备IP地址
3. 在浏览器中访问该IP地址即可查看Web服务器页面

## 安全注意事项

- 请勿将包含实际WiFi密码的配置文件提交到版本控制系统
- `.gitignore` 已配置忽略 `.env` 文件和SPIFFS配置文件
- 如需更改配置，只需更新SPIFFS中的配置文件并重新上传

## 开发

如需修改配置管理功能，可编辑 `include/config_manager.h` 和 `lib/config_manager/config_manager.cpp` 文件。
