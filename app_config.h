#pragma once
// ---------- Pin map (matches diagram.json for the Wokwi simulator) ----------
constexpr int PIN_DHT22   = 15;  // temperature / humidity sensor
constexpr int PIN_POT     = 34;  // ADC1 input simulating a load-current sensor
constexpr int PIN_LED     = 4;   // alarm LED
constexpr int PIN_BUTTON  = 18;  // alarm acknowledge button (active low)
constexpr int PIN_SDA     = 21;  // SSD1306 OLED (I2C)
constexpr int PIN_SCL     = 22;

// ---------- Timing ----------
constexpr uint32_t ADC_PERIOD_MS        = 100;   // 10 Hz current sampling
constexpr uint32_t DHT_PERIOD_MS        = 2000;  // DHT22 minimum read interval
constexpr uint32_t DISPLAY_PERIOD_MS    = 500;
constexpr uint32_t TELEMETRY_PERIOD_MS  = 1000;
constexpr uint32_t HEALTH_PERIOD_MS     = 3000;  // software watchdog window
constexpr uint32_t STATS_PERIOD_MS      = 10000;
constexpr uint32_t BUTTON_DEBOUNCE_MS   = 200;

// ---------- Thresholds ----------
constexpr float TEMP_ALARM_C        = 30.0f;
constexpr float TEMP_HYSTERESIS_C   = 1.0f;
constexpr float CURRENT_ALARM_A     = 15.0f;
constexpr float CURRENT_HYST_A      = 1.0f;
constexpr float CURRENT_FULL_SCALE  = 20.0f;  // amps at ADC max
constexpr int   ADC_MAX             = 4095;

// ---------- RTOS configuration ----------
constexpr int SENSOR_QUEUE_LEN = 10;

// Task priorities (higher number = higher priority)
constexpr UBaseType_t PRIO_ALARM      = 4;
constexpr UBaseType_t PRIO_SENSOR     = 3;
constexpr UBaseType_t PRIO_PROCESSING = 2;
constexpr UBaseType_t PRIO_HEALTH     = 2;
constexpr UBaseType_t PRIO_DISPLAY    = 1;
constexpr UBaseType_t PRIO_TELEMETRY  = 1;

// Stack sizes in bytes (ESP32 FreeRTOS uses bytes, not words)
constexpr uint32_t STACK_SENSOR     = 3072;
constexpr uint32_t STACK_PROCESSING = 3072;
constexpr uint32_t STACK_ALARM      = 2048;
constexpr uint32_t STACK_DISPLAY    = 4096;
constexpr uint32_t STACK_TELEMETRY  = 3072;
constexpr uint32_t STACK_HEALTH     = 3072;
