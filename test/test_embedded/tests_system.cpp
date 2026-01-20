#include <Arduino.h>
#include <unity.h>
#include "WateringSystem.h"

// 实例化一个测试专用的系统
// GPIO 4, 时长 2秒 (短一点方便测), 2:00, 间隔 3天, 18次, 开启NVS
WateringSystem testSys(4, 2, 2, 0, 3, 18, true);

// 辅助：构造一个时间结构体
struct tm createTime(int day, int hour, int min)
{
    struct tm t;
    t.tm_yday = day;
    t.tm_hour = hour;
    t.tm_min = min;
    t.tm_sec = 0;
    return t;
}

// === 测试前准备：清空环境 ===
void setUp_System(void)
{
    testSys.begin();
    testSys.factoryReset(); // 确保每次测试都是干净的起点
}

// === 测试用例 1: NVS 断电记忆功能 ===
void test_nvs_persistence(void)
{
    // 1. 模拟第一次运行
    struct tm t1 = createTime(100, 2, 0); // 第100天 2:00
    testSys.update(t1);                   // 这应该会触发一次浇水

    TEST_ASSERT_EQUAL_INT(1, testSys.getWateredCount()); // 验证计数+1

    // 2. 模拟重启 (重新执行 begin，会从 NVS 读取)
    // 我们新建一个对象来模拟"重启后的系统"
    WateringSystem rebootedSys(4, 2, 2, 0, 3, 18, true);
    rebootedSys.begin(); // 读取 Flash

    // 3. 验证数据是否恢复
    TEST_ASSERT_EQUAL_INT(1, rebootedSys.getWateredCount());
}

// === 测试用例 2: Kill 功能 (彻底终止) ===
void test_kill_function(void)
{
    // 1. 杀死系统
    testSys.killSystem();
    TEST_ASSERT_FALSE(testSys.isEnabled());

    // 2. 尝试触发 (第100天 2:00)
    struct tm t1 = createTime(100, 2, 0);
    testSys.update(t1);

    // 3. 验证：应该完全无视，计数器依然是 0
    TEST_ASSERT_EQUAL_INT(0, testSys.getWateredCount());

    // 4. 模拟重启：验证 Kill 状态是否被保存
    WateringSystem rebootedSys(4, 2, 2, 0, 3, 18, true);
    rebootedSys.begin();
    TEST_ASSERT_FALSE(rebootedSys.isEnabled()); // 重启后应该依然是死的
}

// === 测试用例 3: Revive 功能 (复活) ===
void test_revive_function(void)
{
    // 1. 先杀死
    testSys.killSystem();

    // 2. 再复活
    testSys.reviveSystem();
    TEST_ASSERT_TRUE(testSys.isEnabled());

    // 3. 尝试触发
    struct tm t1 = createTime(200, 2, 0);
    testSys.update(t1);

    // 4. 验证：应该正常工作了
    TEST_ASSERT_EQUAL_INT(1, testSys.getWateredCount());
}

// === 测试用例 4: Stop 功能 (中途打断) ===
void test_stop_interruption(void)
{
    // 这个测试稍微复杂，利用 yield 回调来模拟中途发送指令

    // 设置时长为 5 秒
    WateringSystem longSys(4, 5, 2, 0, 3, 18, false);
    longSys.begin();

    // 注入回调：在第 1 秒的时候发送 stop
    longSys.setYieldCallback([&]()
                             {
        static int calls = 0;
        calls++;
        if (calls == 1) {
            Serial.println("[Test] Injecting STOP command...");
            longSys.stopWatering();
        } });

    unsigned long start = millis();
    longSys.forceWatering(); // 开始 (原本应该跑5秒)
    unsigned long duration = millis() - start;

    // 验证：实际运行时长应该远小于 5000ms (考虑 overhead，约 1000-2000ms)
    // 如果 stop 没生效，这里会跑满 5000ms
    Serial.printf("[Test] Actual duration: %lu ms\n", duration);
    TEST_ASSERT_LESS_THAN(4500, duration);
}
// === 测试用例 5: Reset 功能 ===
void test_reset_function(void)
{
    // 1. 先制造一些脏数据
    // 我们手动把 NVS 里的数据改掉，模拟已经浇了 10 次
    // (由于 WateringSystem 没有 setWateredCount 接口，我们通过 NVS 后门修改)
    // 或者更简单：运行一次 update 让它变成 1，验证能变回 0 即可

    testSys.factoryReset(); // 归零

    // 触发一次浇水 -> Count 变 1
    struct tm t1 = createTime(100, 2, 0);
    testSys.update(t1);
    TEST_ASSERT_EQUAL_INT(1, testSys.getWateredCount());

    // 2. 执行 Reset
    testSys.resetSystem();

    // 3. 验证内存变量
    TEST_ASSERT_EQUAL_INT(0, testSys.getWateredCount());

    // 4. 验证 NVS 持久化 (模拟重启)
    WateringSystem rebootedSys(4, 2, 2, 0, 3, 18, true);
    rebootedSys.begin();
    TEST_ASSERT_EQUAL_INT(0, rebootedSys.getWateredCount());
}

// === 运行入口 ===
void run_system_tests()
{
    RUN_TEST(test_nvs_persistence);
    RUN_TEST(test_kill_function);
    RUN_TEST(test_revive_function);
    RUN_TEST(test_stop_interruption);
    RUN_TEST(test_reset_function);
}