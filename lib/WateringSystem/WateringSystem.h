#ifndef WATERING_SYSTEM_H
#define WATERING_SYSTEM_H

#include <Arduino.h>
#include <Preferences.h>
#include "../WateringCore/WateringCore.h" // 引用之前的逻辑大脑

class WateringSystem {
private:
    WateringLogic logic;    // 大脑
     Preferences prefs;     // 用于存储状态
    int relayPin;           // 硬件引脚
    int durationSec;        // 浇水时长

    // 内部状态
    int wateredCount = 0;
    int lastWateredDay = -999;
    bool hasWateredToday = false;

    // 私有动作：干活
    void activatePump() {
        Serial.printf("[Watering] START (Cycle %d / %d)\n", wateredCount + 1, logic.maxCycles);
        digitalWrite(relayPin, LOW); // 开
        
        for(int i=0; i<durationSec; i++) {
            delay(1000); // 阻塞式延时
        }
        
        digitalWrite(relayPin, HIGH); // 关
        Serial.println("[Watering] STOP");
    }

    // 保存状态到 Flash
    void saveState() {
        prefs.begin("plant_data", false); // 打开命名空间
        prefs.putInt("count", wateredCount);
        prefs.putInt("last_day", lastWateredDay);
        prefs.end();
        Serial.println("[System] State Saved to Flash.");
    }

public:
    // 构造函数
    WateringSystem(int pin, int duration, int hour, int min, int interval, int max)
        : logic(hour, min, interval, max), relayPin(pin), durationSec(duration) {}

    void begin() {
        pinMode(relayPin, OUTPUT);
        digitalWrite(relayPin, HIGH); // 默认关

        // 启动时加载存档
        prefs.begin("plant_data", false); // 只读模式打开
        wateredCount = prefs.getInt("count", 0);       // 读不到默认返 0
        lastWateredDay = prefs.getInt("last_day", -999); // 读不到默认返 -999
        prefs.end();

        Serial.printf("[Watering] Loaded State: Count=%d, LastDay=%d\n", wateredCount, lastWateredDay);
    }

    // 核心接口：接收时间，执行逻辑
    // 这个函数不需要自己去获取时间，而是由外部传入
    void update(struct tm &currentTime) {
        
        // 1. 询问大脑
        bool shouldRun = logic.shouldStart(
            currentTime.tm_hour, 
            currentTime.tm_min, 
            currentTime.tm_yday, 
            wateredCount, 
            lastWateredDay, 
            hasWateredToday
        );

        // 2. 执行
        if (shouldRun) {
            activatePump();
            
            // 更新状态
            wateredCount++;
            lastWateredDay = currentTime.tm_yday;
            hasWateredToday = true;

            saveState();
        }

        // 3. 重置防抖
        if (currentTime.tm_hour != logic.targetHour) {
            hasWateredToday = false;
        }
    }

    void resetSystem() {
        wateredCount = 0;
        lastWateredDay = -999;
        saveState();
        Serial.println("[Watering] RESET DONE.");
    }
    
    int getCount() { return wateredCount; }
};

#endif