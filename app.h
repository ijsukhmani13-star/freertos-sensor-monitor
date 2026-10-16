#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <freertos/event_groups.h>
#include "app_config.h"

// Raw reading produced by the sensor task and consumed by the processing task.
struct SensorReading {
  uint32_t timestampMs;
  float temperatureC;
  float humidityPct;
  float currentA;
  bool dhtValid;
};

// Shared, filtered system state (protected by g_stateMutex).
struct SystemState {
  float temperatureC = NAN;
  float humidityPct = NAN;
  float currentA = 0.0f;
  bool tempAlarm = false;
  bool currentAlarm = false;
  bool faultAlarm = false;
  bool acknowledged = false;
  uint32_t samplesProcessed = 0;
  uint32_t queueDrops = 0;
  uint32_t sensorErrors = 0;
};

// Alarm event group bits
constexpr EventBits_t EVT_ALARM_TEMP    = BIT0;
constexpr EventBits_t EVT_ALARM_CURRENT = BIT1;
constexpr EventBits_t EVT_ALARM_FAULT   = BIT2;
constexpr EventBits_t EVT_ALARM_ANY     = EVT_ALARM_TEMP | EVT_ALARM_CURRENT | EVT_ALARM_FAULT;

// Heartbeat bits: every periodic task sets its bit; the health task checks them.
constexpr EventBits_t HB_SENSOR     = BIT0;
constexpr EventBits_t HB_PROCESSING = BIT1;
constexpr EventBits_t HB_DISPLAY    = BIT2;
constexpr EventBits_t HB_TELEMETRY  = BIT3;
constexpr EventBits_t HB_ALARM      = BIT4;
constexpr EventBits_t HB_ALL = HB_SENSOR | HB_PROCESSING | HB_DISPLAY | HB_TELEMETRY | HB_ALARM;

// ---------- Shared RTOS objects (created in main.cpp) ----------
extern QueueHandle_t      g_sensorQueue;
extern SemaphoreHandle_t  g_stateMutex;
extern SemaphoreHandle_t  g_serialMutex;
extern SemaphoreHandle_t  g_ackSemaphore;
extern EventGroupHandle_t g_alarmEvents;
extern EventGroupHandle_t g_heartbeats;
extern SystemState        g_state;

extern TaskHandle_t g_sensorTask, g_processingTask, g_alarmTask,
                    g_displayTask, g_telemetryTask, g_healthTask;

// Thread-safe logging and state helpers
void logMsg(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
SystemState snapshotState();

// Task entry points
void sensorTask(void*);
void processingTask(void*);
void alarmTask(void*);
void displayTask(void*);
void telemetryTask(void*);
void healthTask(void*);
