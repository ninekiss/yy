#include <Arduino.h>
#include <unity.h>
#include "TestIndicator.h"
#include "../common/logic_tests.h"
#include "tests_system.h"

// 实例化灯光工具
TestIndicator indicator(48, 20000); // 引脚48，展示时间20秒以加快测试反馈

void setUp(void)
{
    // 每次运行 test_system 里的用例前，都重置一下 NVS
    // 注意：这里简单处理，实际 Unity 会自动匹配
    setUp_System();
}

void setup()
{
    delay(2000);

    // 1. 准备工作
    indicator.begin();

    // 2. 开始测试框架
    UNITY_BEGIN();

    // 1. 跑纯逻辑测试(复用 common)
    run_shared_logic_tests();

    // 2. 跑系统集成测试 (新写的)
    run_system_tests();

    // 如果未来你有系统测试，可以加：
    // run_system_tests();

    // 4. 结束并获取结果
    UNITY_END();

    // 5. 展示结果
    if (Unity.TestFailures == 0)
    {
        indicator.showSuccess();
    }
    else
    {
        indicator.showFailure(Unity.TestFailures);
    }
}

void loop()
{
}