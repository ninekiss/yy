#ifndef LOGIC_TESTS_H
#define LOGIC_TESTS_H

#include <unity.h>
#include "WateringCore.h"

// 实例化逻辑对象 (静态，防止冲突)
static WateringLogic logic(2, 0, 3, 18);

// === 测试用例 1: 时间未到 ===
void test_time_not_reached(void) {
    // 1:59, 第100天 -> 期望: false
    bool result = logic.shouldStart(1, 59, 100, 0, -999, false);
    TEST_ASSERT_FALSE(result); 
}

// === 测试用例 2: 首次运行 ===
void test_first_run(void) {
    // 2:00, 第100天, 上次-999 -> 期望: true
    bool result = logic.shouldStart(2, 0, 100, 0, -999, false);
    TEST_ASSERT_TRUE(result);
}

// === 测试用例 3: 间隔满足 ===
void test_interval_ok(void) {
    // 2:00, 第103天, 上次100 (差3天) -> 期望: true
    bool result = logic.shouldStart(2, 0, 103, 1, 100, false);
    TEST_ASSERT_TRUE(result);
}

// === 测试用例 4: 间隔不足 ===
void test_interval_short(void) {
    // 2:00, 第102天, 上次100 (差2天) -> 期望: false
    bool result = logic.shouldStart(2, 0, 102, 1, 100, false);
    TEST_ASSERT_FALSE(result);
}

// === 测试用例 5: 跨年逻辑 ===
void test_new_year(void) {
    // 2:00, 今年第2天, 上次去年第364天 (间隔3天) -> 期望: true
    bool result = logic.shouldStart(2, 0, 2, 5, 364, false);
    TEST_ASSERT_TRUE(result);
}

// === 测试用例 6: 次数超限 ===
void test_max_cycles(void) {
    // 已执行18次 -> 期望: false
    bool result = logic.shouldStart(2, 0, 200, 18, 150, false);
    TEST_ASSERT_FALSE(result);
}

// === [故意失败] 的测试用例 ===
void test_should_fail_on_wrong_time(void) {
    // 设定场景：
    // 目标时间是 02:00
    // 当前输入是 14:00 (下午2点)
    
    // 逻辑类应该返回: false (绝对不应该启动)
    bool result = logic.shouldStart(14, 0, 100, 0, -999, false);
    
    // 这里的逻辑是：result 实际上是 false。
    // 但我们强制断言它是 TRUE (我们告诉测试框架：如果不返回 true，就算失败)。
    // 因为 false != true，所以这个测试会【失败】。
    TEST_ASSERT_TRUE_MESSAGE(result, "Intentional Failure: Pump should NOT start at 14:00");
}

// ================= 共享的运行清单 =================
// 这个函数会在 native 和 embedded 里分别被调用
void run_shared_logic_tests() {
    RUN_TEST(test_time_not_reached);
    RUN_TEST(test_first_run);
    RUN_TEST(test_interval_ok);
    RUN_TEST(test_interval_short);
    RUN_TEST(test_new_year);
    RUN_TEST(test_max_cycles);
    // RUN_TEST(test_should_fail_on_wrong_time); // 故意失败的测试
}

#endif