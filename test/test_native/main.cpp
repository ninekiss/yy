#include <unity.h>
// 引入上一级目录的 common
#include "../common/logic_tests.h" 

// Native 环境必须显式定义这两个钩子
void setUp(void) {}
void tearDown(void) {}

int main(int argc, char **argv) {
    UNITY_BEGIN();

    // 直接运行共享清单
    run_shared_logic_tests();

    UNITY_END();
    return 0;
}