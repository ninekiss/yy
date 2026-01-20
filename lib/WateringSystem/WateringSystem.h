#ifndef WATERING_SYSTEM_H
#define WATERING_SYSTEM_H

#include <Arduino.h>
#include <Preferences.h>
#include <functional>
#include "../WateringCore/WateringCore.h" // 引用之前的逻辑大脑

class WateringSystem
{
private:
    // 定义通知回调函数类型：接收一个字符串消息
    typedef std::function<void(const char *)> NotifierCallback;
    NotifierCallback notifier = nullptr; // 状态变更通知回调

    // 定义一个空闲时的回调类型
    typedef std::function<void(void)> YieldCallback;
    YieldCallback yieldHandler = nullptr; // 保存回调函数

    WateringLogic logic; // 大脑
    Preferences prefs;   // 用于存储状态
    int relayPin;        // 硬件引脚
    int durationSec;     // 浇水时长
    bool enableStorage;  // 是否启用状态存储

    // 内部状态
    int wateredCount = 0;
    int lastWateredDay = -999;
    bool hasWateredToday = false;
    bool isBusy = false;
    bool stopRequested = false;
    bool systemEnabled = true; // 系统总开关

    // 私有动作：干活
    void activatePump(bool isManual = false)
    {

        // 2. 上锁
        isBusy = true;

        // 重置停止请求
        stopRequested = false;

        // 定义一个临时缓存区，用来存放拼接后的字符串
        char msgBuffer[64];

        // 计算当前是第几次 (因为 wateredCount 从0开始，所以显示时 +1)
        int currentCycle = wateredCount + 1;

        // 1. 格式化“开始”消息
        // 格式示例: "Auto Start [5/18]"
        snprintf(msgBuffer, sizeof(msgBuffer), "%s [%d/%d]",
                 isManual ? "Manual Start" : "Auto Start",
                 currentCycle,
                 logic.maxCycles);

        // 1. 通知：开始
        if (notifier)
            notifier(msgBuffer);

        Serial.printf("[Watering] START (Cycle %d / %d)\n", currentCycle, logic.maxCycles);
        digitalWrite(relayPin, LOW); // 开

        int elapsed = 0;
        for (int i = 0; i < durationSec; i++)
        {
            delay(1000); // 阻塞式延时
            elapsed++;

            // 每等待 1 秒，就执行一次外部传入的任务 (比如 MQTT loop)
            if (yieldHandler)
            {
                yieldHandler();
            }

            if (stopRequested)
            {
                Serial.println("[Watering] STOP signal received! Aborting...");
                break; // 跳出 for 循环
            }
        }

        digitalWrite(relayPin, HIGH); // 关

        // 3. 发送结束通知 (区分是正常完成还是被终止)
        if (stopRequested)
        {
            snprintf(msgBuffer, sizeof(msgBuffer), "Aborted (Run %ds) [%d/%d]",
                     elapsed, currentCycle, logic.maxCycles);
            Serial.printf("[Watering] ABORTED(Cycle %d / %d / %ds)\n", currentCycle, logic.maxCycles, elapsed);
        }
        else
        {
            snprintf(msgBuffer, sizeof(msgBuffer), "Done [%d/%d]",
                     currentCycle, logic.maxCycles);
            Serial.printf("[Watering] DONE(Cycle %d / %d)\n", currentCycle, logic.maxCycles);
        }

        // 2. 通知：结束
        if (notifier)
            notifier(msgBuffer);

        // 3. 解锁
        isBusy = false;
    }

    // 保存状态到 Flash
    void saveState()
    {
        if (!enableStorage)
            return;

        prefs.begin("plant_data", false); // 打开命名空间
        prefs.putInt("count", wateredCount);
        prefs.putInt("last_day", lastWateredDay);
        // 新增：保存是否被杀死的标志
        prefs.putBool("enabled", systemEnabled);
        prefs.end();
        Serial.println("[System] State Saved to Flash.");
    }

public:
    // 构造函数
    WateringSystem(int pin, int duration, int hour, int min, int interval, int max, bool useNVS = true)
        : logic(hour, min, interval, max), relayPin(pin), durationSec(duration), enableStorage(useNVS) {}

    // 获取当前计数
    int getWateredCount() { return wateredCount; }

    // 获取系统是否存活
    bool isEnabled() { return systemEnabled; }

    // [危险!!!] 仅用于测试：擦除 NVS 数据，重置环境
    void factoryReset()
    {
        if (enableStorage)
        {
            prefs.begin("plant_data", false);
            prefs.clear(); // 清空所有 key
            prefs.end();
            Serial.println("[Test] NVS Cleared.");
        }
        wateredCount = 0;
        lastWateredDay = -999;
        hasWateredToday = false;
        isBusy = false;
        stopRequested = false;
    }

    void begin()
    {
        pinMode(relayPin, OUTPUT);
        digitalWrite(relayPin, HIGH); // 默认关

        if (enableStorage)
        {
            // 启动时加载存档
            prefs.begin("plant_data", false);                // 只读模式打开
            wateredCount = prefs.getInt("count", 0);         // 读不到默认返 0
            lastWateredDay = prefs.getInt("last_day", -999); // 读不到默认返 -999
            // 新增：读取开关状态 (默认为 true)
            systemEnabled = prefs.getBool("enabled", true);
            prefs.end();
            Serial.printf("[Watering] Init. Enabled: %s\n", systemEnabled ? "YES" : "NO (KILLED)");
            Serial.printf("[Watering] Persistence ENABLED. Loaded State: Count=%d, LastDay=%d\n", wateredCount, lastWateredDay);
        }
        else
        {
            Serial.println("[Watering] Persistence DISABLED. State will reset on reboot.");
        }
    }

    // 注册通知回调
    // 外部调用这个函数，传入一个函数，当浇水发生时，我会调用这个函数
    void setNotifier(NotifierCallback cb)
    {
        notifier = cb;
    }

    // 允许外部注入“呼吸”逻辑
    void setYieldCallback(YieldCallback cb)
    {
        yieldHandler = cb;
    }

    // 强制手动浇水接口
    void forceWatering()
    {
        if (!systemEnabled)
        {
            Serial.println("[System] Warning: System is KILLED, but Force run allowed.");
            // 或者直接 return 拒绝执行，看你需求
        }

        // 4. 检查锁
        if (isBusy)
        {
            Serial.println("[Watering] Ignored: Already watering.");
            // 可选：告诉 MQTT 我很忙
            if (notifier)
                notifier("Ignored: System is Busy");
            return; // 直接返回，不执行
        }

        Serial.println("[Watering] Force watering triggered!");
        activatePump(true); // true 表示是手动触发
        // 手动浇水通常不增加 wateredCount 计数，也不影响自动逻辑，
        // 或者你可以根据需求决定是否要更新 lastWateredDay
        // 更新状态
        wateredCount++;
        // lastWateredDay = currentTime.tm_yday;
        hasWateredToday = true;
    }

    // 2. 新增：彻底终止接口
    void killSystem()
    {
        if (!systemEnabled)
            return; // 已经死了，不用再杀

        Serial.println("[Watering] KILL command received. Shutting down forever.");

        // 如果正在浇水，先立刻停下
        if (isBusy)
            stopWatering();

        systemEnabled = false;
        saveState(); // 写入 Flash，重启后依然保持“死亡”状态

        if (notifier)
            notifier("System KILLED. No more auto watering.");
    }

    // 3. (可选) 新增：复活接口
    void reviveSystem()
    {
        if (systemEnabled)
            return;

        Serial.println("[Watering] Revive command received.");
        systemEnabled = true;
        saveState();

        if (notifier)
            notifier("System Revived. Back online.");
    }

    // 核心接口：接收时间，执行逻辑
    // 这个函数不需要自己去获取时间，而是由外部传入
    void update(struct tm &currentTime)
    {
        // 如果系统被 kill 了，直接忽略自动逻辑
        if (!systemEnabled)
        {
            return;
        }

        if (isBusy)
            return; // 忙碌中，跳过

        // 1. 询问大脑
        bool shouldRun = logic.shouldStart(
            currentTime.tm_hour,
            currentTime.tm_min,
            currentTime.tm_yday,
            wateredCount,
            lastWateredDay,
            hasWateredToday);

        // 2. 执行
        if (shouldRun)
        {
            activatePump();

            // 更新状态
            wateredCount++;
            lastWateredDay = currentTime.tm_yday;
            hasWateredToday = true;

            saveState();

            // MTEST:START:模拟隔天
            // delay(10000);
            // MTEST:END
        }

        // 3. 重置防抖
        if (currentTime.tm_hour != logic.targetHour)
        {
            hasWateredToday = false;
        }
    }

    void stopWatering()
    {
        if (isBusy)
        {
            Serial.println("[Watering] Stopping current session...");
            stopRequested = true; // 设置标志位
        }
        else
        {
            // 如果没在浇水，也可以发个通知
            if (notifier)
                notifier("Ignored: Not watering");
        }
    }

    void resetSystem()
    {
        // 1. 安全检查：如果正在浇水，禁止重置！
        // 因为浇水结束时会写入一次 Count，会覆盖掉你现在的重置操作。
        if (isBusy)
        {
            Serial.println("[System] Cannot Reset: System is Busy watering.");
            if (notifier)
                notifier("Reset Failed: System is Busy");
            return;
        }

        // 2. 执行重置
        wateredCount = 0;
        lastWateredDay = -999;
        hasWateredToday = false;

        // 注意：不要修改 systemEnabled，保持它的独立性

        saveState();

        Serial.println("[System] Task Reset. Counter back to 0.");
        if (notifier)
            notifier("System Reset: Counter=0");
    }

    int getCount() { return wateredCount; }
};

#endif