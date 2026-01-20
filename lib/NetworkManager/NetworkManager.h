#include <WiFi.h>
class NetworkManager
{
private:
    const char *ssid;
    const char *password;

    // 新增：记录上次尝试连接的时间
    unsigned long lastConnectAttempt = 0;
    // 新增：冷却时间 (比如 60秒)
    const unsigned long RECONNECT_INTERVAL = 60000;

public:
    NetworkManager(const char *_ssid, const char *_pwd) : ssid(_ssid), password(_pwd) {}

    void connect()
    {
        // 1. 如果已经连上了，直接返回
        if (WiFi.status() == WL_CONNECTED)
            return;

        // 2. [关键修改] 检查是否还在冷却期
        // 如果距离上次尝试还不到 60秒，直接跳过，不要频繁骚扰 WiFi 芯片
        if (millis() - lastConnectAttempt < RECONNECT_INTERVAL && lastConnectAttempt != 0)
        {
            return;
        }

        // 更新尝试时间
        lastConnectAttempt = millis();

        Serial.printf("\n[Network] Connecting to %s ", ssid);

        // 建议：先断开旧连接，清理状态
        WiFi.disconnect();
        WiFi.mode(WIFI_STA); // 确保是 Station 模式
        WiFi.begin(ssid, password);

        int timeout = 20;
        while (WiFi.status() != WL_CONNECTED && timeout > 0)
        {
            delay(500);
            Serial.print(".");
            timeout--;
        }

        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("\n[Network] Connected!");
        }
        else
        {
            Serial.println("\n[Network] Connection failed (timeout). Will retry later.");
        }
    }

    bool isConnected()
    {
        return WiFi.status() == WL_CONNECTED;
    }
};