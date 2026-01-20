#include <Arduino.h>
#include "NetworkManager.h"
#include "TimeManager.h"
#include "WateringSystem.h"
#include <MqttManager.h>

// ================= 实例化模块 =================

// 1. 网络模块
#ifndef WIFI_SSID
#define WIFI_SSID "your_ssid"
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "your_password"
#endif
NetworkManager wifiMgr(WIFI_SSID, WIFI_PASSWORD);

// 2. 时间模块
TimeManager timeMgr;

// 3. 浇水模块 (GPIO 4, 38秒, 2点0分, 间隔3天, 18次)
WateringSystem waterSys(4, 38, 18, 15, 3, 18, ENABLE_NVS);

MqttManager mqttMgr(MQTT_SERVER, MQTT_PORT, MQTT_USER, MQTT_PASS);

// A. 当 MQTT 收到消息时
void onMqttMessage(char *topic, uint8_t *payload, unsigned int length)
{
    String msg = "";
    for (int i = 0; i < length; i++)
        msg += (char)payload[i];

    Serial.printf("[MQTT] Recv [%s]: %s\n", topic, msg.c_str());
    // 判断指令
    if (String(topic) == MQTT_TOPIC_CMD)
    {
        msg.toLowerCase();

        if (msg == "start" || msg == "on")
        {
            waterSys.forceWatering(); // 调用浇水系统
        }
        else if (msg == "stop" || msg == "off")
        {
            waterSys.stopWatering();
        }
        else if (msg == "kill" || msg == "shutdown")
        {
            waterSys.killSystem();
        }
        else if (msg == "revive" || msg == "enable")
        {
            waterSys.reviveSystem();
        }
        else if (msg == "reset")
        {
            waterSys.resetSystem();
        }
    }
}

// B. 当浇水系统有状态变化时
void onWateringEvent(const char *statusMsg)
{
    // 发送 MQTT 消息
    mqttMgr.publish(MQTT_TOPIC_STATUS, statusMsg);
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    // 各个模块独立初始化
    wifiMgr.connect(); // 连网
    timeMgr.begin();   // 开启 NTP 配置
    waterSys.begin();  // 初始化 GPIO

    // 1. 设置 MQTT 回调：收到消息 -> onMqttMessage
    mqttMgr.begin(onMqttMessage);

    // 2. 设置 浇水系统 回调：状态变化 -> onWateringEvent
    // 这里使用了 Lambda 表达式，非常优雅地连接了两个模块
    waterSys.setNotifier([](const char *msg)
                         { onWateringEvent(msg); });

    // 注入 MQTT 心跳维护逻辑
    // 告诉浇水系统：你在干活的间隙，记得帮我跑一下 mqttMgr.loop()
    waterSys.setYieldCallback([]()
                              {
        // 只有连网的时候才跑，防止报错
        if (wifiMgr.isConnected()) {
            mqttMgr.loop(); 
        } });

    // 如果需要，可以在这里阻塞等待时间同步
    if (wifiMgr.isConnected())
    {
        timeMgr.waitForSync();
    }
}

void loop()
{
    // 1. 确保网络在线 (NetworkManager 内部处理重连逻辑)
    // 这里的实现比较简单，如果断网了，NTP 实际上会依赖内部晶振走时
    if (!wifiMgr.isConnected())
    {
        wifiMgr.connect();
    }
    else
    {
        // 只有网络通的时候才处理 MQTT
        if (!mqttMgr.isConnected())
        {
            mqttMgr.connect();
        }
        mqttMgr.loop(); // 处理收发
    }

    // 2. 从 TimeManager 获取当前时间数据
    struct tm currentTime;
    bool timeValid = timeMgr.getTime(currentTime);

    // 3. 如果时间有效，就把时间数据“喂”给浇水系统
    if (timeValid)
    {
        // 主程序作为中介，把 TimeManager 的数据传给 WateringSystem
        // 实现了 WateringSystem 和 TimeManager 的彻底解耦
        waterSys.update(currentTime);

        // (可选) 可以在这里把时间传给其他系统，比如 "DisplaySystem.update(currentTime)"
    }
    else
    {
        Serial.println("Time not synced yet...");
    }

    delay(1000);
}