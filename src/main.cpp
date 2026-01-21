#include <Arduino.h>
#include <esp_task_wdt.h> // 引入看门狗库

#include "NetworkManager.h"
#include "TimeManager.h"
#include "WateringSystem.h"
#include "MqttManager.h"
#include "OtaManager.h" // 引入 OTA

// ================= 实例化模块 =================
NetworkManager wifiMgr(WIFI_SSID, WIFI_PASSWORD);
TimeManager timeMgr;
// 使用 secrets.ini 里的 OTA_PASS 密码
OtaManager otaMgr("esp32-watering", OTA_PASS);
MqttManager mqttMgr(MQTT_SERVER, MQTT_PORT, MQTT_USER, MQTT_PASS);

// 传入 ENABLE_NVS 宏
WateringSystem waterSys(SYSTEM_WATERING_PIN, SYSTEM_WATERING_DURATION, SYSTEM_WATERING_START_HOUR, SYSTEM_WATERING_START_MINUTE, SYSTEM_WATERING_INTERVAL_DAYS, SYSTEM_WATERING_COUNT, ENABLE_NVS);

// ================= 辅助函数 =================
void reportDeviceStatus()
{
    if (!mqttMgr.isConnected())
        return;
    String info = waterSys.getSystemInfoJson();
    String ip = WiFi.localIP().toString();
    String payload = "{\"event\":\"boot\", \"ip\":\"" + ip + "\", \"system\":" + info + ", \"ota_version\":\"0.2.1\"}";
    mqttMgr.publish(MQTT_TOPIC_STATUS, payload.c_str());
}

void onMqttMessage(char *topic, uint8_t *payload, unsigned int length)
{
    String msg = "";
    for (int i = 0; i < length; i++)
        msg += (char)payload[i];
    Serial.printf("[MQTT] Recv [%s]: %s\n", topic, msg.c_str());

    if (String(topic) == MQTT_TOPIC_CMD)
    {
        msg.toLowerCase();
        if (msg == "start" || msg == "on")
            waterSys.forceWatering();
        else if (msg == "stop" || msg == "off")
            waterSys.stopWatering();
        else if (msg == "reset")
            waterSys.resetSystem();
        else if (msg == "kill")
            waterSys.killSystem();
        else if (msg == "revive")
            waterSys.reviveSystem();
        else if (msg == "info" || msg == "status")
            reportDeviceStatus();
    }
}

void onWateringEvent(const char *statusMsg)
{
    mqttMgr.publish(MQTT_TOPIC_STATUS, statusMsg);
}

// ================= SETUP =================
void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n==== System Booting(OTA v0.2.0)  ====");


    // 1. 初始化看门狗 (30秒超时)
    // 如果系统卡死超过30秒不喂狗，自动重启
    esp_task_wdt_init(30, true);
    esp_task_wdt_add(NULL);

    // 2. 初始化各模块
    waterSys.begin();
    timeMgr.begin();
    mqttMgr.begin(onMqttMessage);

    // 3. 绑定回调
    waterSys.setNotifier([](const char *msg)
                         { onWateringEvent(msg); });

    // 绑定呼吸逻辑 (包含 MQTT 循环 和 喂狗)
    waterSys.setYieldCallback([]()
                              {
        // 浇水时也要处理 MQTT，防止断连
        if (wifiMgr.isConnected()) mqttMgr.loop(); 
        // 关键：浇水时也要喂狗，防止浇水38秒超过看门狗30秒限制
        esp_task_wdt_reset();
        // 3. 【新增】处理 OTA 请求
        // 这样即使正在浇水，也能接收固件升级！
        otaMgr.handle();
    });

    

    // 4. 启动网络 (带冷却机制，防止死循环)
    wifiMgr.connect();

    // 5. 联网后操作
    if (wifiMgr.isConnected())
    {
        Serial.print("WiFi Connected. IP: ");
        Serial.println(WiFi.localIP());

        // A. 启动 OTA 服务
        otaMgr.begin();

        // B. 连接 MQTT 并上报
        mqttMgr.connect();
        reportDeviceStatus();

        // C. 同步时间
        timeMgr.waitForSync();
    }
    else
    {
        Serial.println("WiFi Failed! Running offline mode.");
    }

    Serial.println("==== System Ready ====");
}

// ================= LOOP =================
void loop()
{
    // 1. 喂狗 (防止主循环卡死)
    esp_task_wdt_reset();

    // 2. 处理 OTA 请求
    otaMgr.handle();

    // 3. 网络维护
    if (!wifiMgr.isConnected())
    {
        wifiMgr.connect(); // 内部有冷却时间，不会一直卡这里
    }
    else
    {
        if (!mqttMgr.isConnected())
            mqttMgr.connect();
        mqttMgr.loop();
    }

    // 4. 业务逻辑
    struct tm currentTime;
    if (timeMgr.getTime(currentTime))
    {
        waterSys.update(currentTime);
    }

    delay(1000);
}