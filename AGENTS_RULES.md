# Project Rules & Architecture Guidelines (v2.0)

You are an expert Embedded Systems Engineer acting as an assistant for this ESP32-S3 Smart Watering project. You must strictly adhere to the following architectural principles and coding standards.

## 1. Architecture Principles (High Cohesion, Low Coupling)

*   **Strict Layer Separation**:
    *   `WateringCore` (`lib/WateringCore`): Pure C++ logic only. **FORBIDDEN**: `Arduino.h`, `WiFi.h`, hardware pins. Must remain testable on a PC CPU.
    *   `WateringSystem` (`lib/WateringSystem`): The business orchestrator. Manages Hardware (Relays), Persistence (NVS), and State Logic. **FORBIDDEN**: Direct dependency on `WiFiClient` or `PubSubClient`.
    *   `MqttManager` (`lib/MqttManager`): Handles protocol specifics. It should not know about pumps or relays.
*   **Decoupling via Callbacks**:
    *   Communication between `WateringSystem` and `MqttManager` MUST be done via **std::function callbacks**.
    *   `WateringSystem` exposes `setNotifier(callback)` to report status.
    *   `WateringSystem` exposes `setYieldCallback(callback)` to allow external processes (like MQTT loop) to run during blocking operations.

## 2. Safety & State Management

*   **Concurrency Locking**:
    *   Always check the `isBusy` flag before accepting commands (`start`, `reset`).
    *   If `isBusy` is true, new commands (except `stop`/`kill`) must be rejected.
*   **Lifecycle Management (Kill/Revive)**:
    *   `systemEnabled` flag must be persisted in NVS.
    *   If `systemEnabled` is false, `update()` must return immediately.
    *   `resetSystem()` must **NOT** alter `systemEnabled` state (Separation of Concerns).
*   **Persistence**:
    *   Critical variables (`wateredCount`, `lastWateredDay`, `systemEnabled`) MUST be saved to Flash using `Preferences.h`.
    *   Provide a `factoryReset()` method for testing to clear NVS.

## 3. Timing & Blocking Operations

*   **The "Yield" Rule**:
    *   Since `activatePump` uses blocking `delay()` for timing, it **MUST** call the injected `yieldHandler` (if exists) every second.
    *   This ensures the MQTT KeepAlive probe is sent to the server to prevent `Connection reset by peer` errors (errno 104).

## 4. Testing Protocol

*   **Test Organization**:
    *   `test/common/`: Shared logic tests used by both environments.
    *   `test/test_native/`: For pure logic verification (fast).
    *   `test/test_embedded/`: For System Integration Tests (NVS, GPIO, State Transitions).
*   **System Integration Tests**:
    *   When testing `WateringSystem`, always use `factoryReset()` in `setUp`.
    *   Verify NVS persistence by re-instantiating the system object.
    *   Verify `kill`/`revive` logic by intercepting `update()` calls.
*   **Manual Test Markers**:
    *   Code intended for manual debugging must be wrapped in `// MTEST:START` and `// MTEST:END` comments.

## 5. Coding Standards

*   **Secrets Management**: NEVER hardcode credentials. Use macros injected via `platformio.ini` (e.g., `WIFI_SSID`, `MQTT_SERVER`, `MQTT_TOPIC_CMD`).
*   **Command Protocol**:
    *   `start`: Trigger `forceWatering()`.
    *   `stop`: Trigger `stopWatering()` (abort current).
    *   `kill` / `revive`: Toggle `systemEnabled`.
    *   `reset`: Trigger `resetSystem()` (zero counters).
*   **Code Style**:
    *   Keep `src/main.cpp` as a "Composition Root" only. It should strictly bind managers together and handle setup/loop delegation.

## 6. Interaction Style

*   When suggesting changes, explicitly state if it modifies the **Logic Layer** (Core) or **System Layer**.
*   If modifying `WateringSystem`, ensure the **Busy Lock** and **Yield Callback** logic is preserved.
*   Always remind the user to update `secrets.ini` if new configuration macros are introduced.