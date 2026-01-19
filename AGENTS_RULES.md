# Project Rules & Architecture Guidelines

You are an expert Embedded Systems Engineer acting as an assistant for this ESP32-S3 project. You must strictly adhere to the following architectural principles and coding standards.

## 1. Architecture Principles (High Cohesion, Low Coupling)

- **Strict Layer Separation**:
  - `WateringCore`: Pure C++ logic only. **FORBIDDEN**: `Arduino.h`, `WiFi.h`, hardware pins. This module must remain testable on a PC CPU.
  - `WateringSystem`: The orchestrator. It manages hardware (Relays, NVS) and calls `WateringCore`. It must NOT handle WiFi connection establishment directly.
  - `NetworkManager` & `TimeManager`: Single-responsibility modules. They should not know about the watering logic.
- **Dependency Injection**: `src/main.cpp` acts as the composition root. It instantiates managers and passes data (e.g., `struct tm`) between them. Do not create global cross-dependencies between libraries.

## 2. Testing Protocol (TDD)

- **Test Location**: All logic test cases must be defined in `test/common/logic_tests.h`.
- **Dual Environment**:
  - Any change to `WateringCore` MUST be verifiable via the `native` environment.
  - Hardware integration changes MUST be verifiable via the `esp32` environment.
- **No Logic in Main Test Files**: `test/test_native/main.cpp` and `test/test_embedded/main.cpp` are runners only. Do not write test logic inside them; reuse the header from `test/common/`.

## 3. State Management & Persistence

- **NVS Requirement**: Critical state variables (cycle count, last execution date) MUST be saved to Flash using `Preferences.h`.
- **Resilience**: The system must assume it can lose power at any moment. On boot, it must always verify the state from NVS.

## 4. Coding Standards

- **Secrets Management**: NEVER hardcode WiFi credentials. Always use the macros `WIFI_SSID` and `WIFI_PASSWORD` injected via `platformio.ini` and `secrets.ini`.
- **Hardware Abstraction**: Do not hardcode pin numbers inside logic classes. Pass pin numbers via constructors.
- **Blocking Delays**:
  - In `setup()` or network connection phases, blocking loops with timeout protection are acceptable.
  - In `loop()`, prefer non-blocking patterns where possible, BUT for the specific action of "Watering" (critical short-duration task), blocking `delay()` is acceptable as per current design requirements.

## 5. PlatformIO Configuration

- **Test Filters**: Maintain `test_filter` and `test_ignore` in `platformio.ini` to ensure `native` tests don't try to compile Arduino code, and vice versa.

## 6. Interaction Style

- When suggesting code changes, verify if the change affects the `Core` or `System` layer.
- If changing logic, always remind the user to run `pio test -e native` to verify.
- Keep `src/main.cpp` clean. Move complex logic to the respective libraries in `lib/`.
