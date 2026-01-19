#ifndef TEST_INDICATOR_H
#define TEST_INDICATOR_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

class TestIndicator {
private:
    Adafruit_NeoPixel pixels;
    uint32_t colorGreen;
    uint32_t colorRed;
    uint32_t colorOff;
    int delayTime;

public:
    // 构造函数：传入引脚号 (默认为 ESP32-S3 DevKit 的 48), 展示时间 (默认为 60000 ms)
    TestIndicator(int pin = 48, int delayTime = 60000) : pixels(1, pin, NEO_GRB + NEO_KHZ800), delayTime(delayTime) {
        // 预定义颜色
        colorGreen = pixels.Color(0, 30, 0); // 绿色 (亮度30)
        colorRed   = pixels.Color(30, 0, 0); // 红色 (亮度30)
        colorOff   = pixels.Color(0, 0, 0);  // 灭
    }

    void begin() {
        pixels.begin();
        pixels.setBrightness(50);
        pixels.clear();
        pixels.show();
    }

    // 显示成功状态 (绿灯常亮 1 分钟)
    void showSuccess() {
        Serial.println("\n[Indicator] Result: SUCCESS (Green Solid)");
        pixels.setPixelColor(0, colorGreen);
        pixels.show();
        
        delay(delayTime); // 保持展示指定时间
        turnOff();
    }

    // 显示失败状态 (红灯闪烁 1 分钟)
    void showFailure(int errorCount) {
        Serial.printf("\n[Indicator] Result: %d FAILURES (Red Blinking)\n", errorCount);
        
        // 闪烁 60 次 (1分钟)
        for (int i = 0; i < delayTime / 1000; i++) {
            pixels.setPixelColor(0, colorRed);
            pixels.show();
            delay(500);

            pixels.setPixelColor(0, colorOff);
            pixels.show();
            delay(500);
        }
        turnOff();
    }

    // 关灯
    void turnOff() {
        Serial.println("[Indicator] LED OFF");
        pixels.setPixelColor(0, colorOff);
        pixels.show();
    }
};

#endif