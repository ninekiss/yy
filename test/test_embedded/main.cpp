#include <Arduino.h>
#include <unity.h>
#include "TestIndicator.h"
#include "../common/logic_tests.h"

// 实例化灯光工具
TestIndicator indicator(48, 20000); // 引脚48，展示时间20秒以加快测试反馈

void setup() {
    delay(2000);

    // 1. 准备工作
    indicator.begin();

    // 2. 开始测试框架
    UNITY_BEGIN();

    // 3. 运行各个分区的测试
    run_shared_logic_tests();
    
    // 如果未来你有系统测试，可以加：
    // run_system_tests(); 

    // 4. 结束并获取结果
    UNITY_END(); 

    // 5. 展示结果
    if (Unity.TestFailures == 0) {
        indicator.showSuccess();
    } else {
        indicator.showFailure(Unity.TestFailures);
    }
}

void loop() {
}