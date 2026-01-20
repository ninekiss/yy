#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <Arduino.h>
#include <ArduinoOTA.h>

class OtaManager
{
private:
    const char *hostname;
    const char *password;

public:
    // 构造函数：需要主机名和密码
    OtaManager(const char *_hostname, const char *_password = nullptr)
        : hostname(_hostname), password(_password) {}

    void begin()
    {
        // 1. 设置主机名 (方便在路由器或IDE里识别)
        ArduinoOTA.setHostname(hostname);

        // 2. 设置密码 (从 secrets.ini 注入)
        if (password != nullptr)
        {
            ArduinoOTA.setPassword(password);
        }

        // 3. 定义回调函数 (用于串口调试)
        ArduinoOTA
            .onStart([]()
                     {
                         String type;
                         if (ArduinoOTA.getCommand() == U_FLASH)
                             type = "sketch";
                         else
                             type = "filesystem";
                         Serial.println("[OTA] Start updating " + type);

                         // 在这里可以加代码关闭所有继电器
                         // 但由于这是静态回调，很难访问 waterSys 对象
                         // 简单粗暴的方法：直接操作寄存器或引脚
                         digitalWrite(SYSTEM_WATERING_PIN, HIGH); // 强制关闭水泵 (假设4是水泵引脚)
                     })
            .onEnd([]()
                   { Serial.println("\n[OTA] End"); })
            .onProgress([](unsigned int progress, unsigned int total)
                        { Serial.printf("[OTA] Progress: %u%%\r", (progress / (total / 100))); })
            .onError([](ota_error_t error)
                     {
                Serial.printf("Error[%u]: ", error);
                if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
                else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
                else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
                else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
                else if (error == OTA_END_ERROR) Serial.println("End Failed"); });

        // 4. 启动服务
        ArduinoOTA.begin();
        Serial.printf("[OTA] Service Ready. Hostname: %s\n", hostname);
    }

    // 必须在 loop() 里频繁调用
    void handle()
    {
        ArduinoOTA.handle();
    }
};

#endif