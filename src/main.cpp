#include <Arduino.h>
#include "NetworkManager.h"
#include "TimeManager.h"
#include "WateringSystem.h"

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
WateringSystem waterSys(4, 38, 2, 0, 3, 18);


void setup() {
    Serial.begin(115200);
    delay(1000);

    // 各个模块独立初始化
    wifiMgr.connect();      // 连网
    timeMgr.begin();        // 开启 NTP 配置
    waterSys.begin();       // 初始化 GPIO
    
    // 如果需要，可以在这里阻塞等待时间同步
    if (wifiMgr.isConnected()) {
        timeMgr.waitForSync();
    }
}

void loop() {
    // 1. 确保网络在线 (NetworkManager 内部处理重连逻辑)
    // 这里的实现比较简单，如果断网了，NTP 实际上会依赖内部晶振走时
    if (!wifiMgr.isConnected()) {
        wifiMgr.connect(); 
    }

    // 2. 从 TimeManager 获取当前时间数据
    struct tm currentTime;
    bool timeValid = timeMgr.getTime(currentTime);

    // 3. 如果时间有效，就把时间数据“喂”给浇水系统
    if (timeValid) {
        // 主程序作为中介，把 TimeManager 的数据传给 WateringSystem
        // 实现了 WateringSystem 和 TimeManager 的彻底解耦
        waterSys.update(currentTime);
        
        // (可选) 可以在这里把时间传给其他系统，比如 "DisplaySystem.update(currentTime)"
    } else {
        Serial.println("Time not synced yet...");
    }

    delay(1000);
}