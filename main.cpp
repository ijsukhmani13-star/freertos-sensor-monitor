// FreeRTOS Multi-Task Sensor Monitor for ESP32
// Six cooperating tasks communicate through a queue, mutexes, a binary
// semaphore (from an ISR) and event groups. See README.md for the design.

#include <stdarg.h>
#include "app.h"

QueueHandle_t      g_sensorQueue  = nullptr;
SemaphoreHandle_t  g_stateMutex   = nullptr;
SemaphoreHandle_t  g_serialMutex  = nullptr;
SemaphoreHandle_t  g_ackSemaphore = nullptr;
EventGroupHandle_t g_alarmEvents  = nullptr;
EventGroupHandle_t g_heartbeats   = nullptr;
SystemState        g_state;

TaskHandle_t g_sensorTask = nullptr, g_processingTask = nullptr, g_alarmTask = nullptr,
             g_displayTask = nullptr, g_telemetryTask = nullptr, g_healthTask = nullptr;

void logMsg(const char* fmt, ...) {
  char buf[256];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  if (xSemaphoreTake(g_serialMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
    Serial.println(buf);
    xSemaphoreGive(g_serialMutex);
  }
}

SystemState snapshotState() {
  SystemState copy;
  if (xSemaphoreTake(g_stateMutex, pdMS_TO_TICKS(20)) == pdTRUE) {
    copy = g_state;
    xSemaphoreGive(g_stateMutex);
  }
  return copy;
}

static void IRAM_ATTR onButtonIsr() {
  BaseType_t higherPriorityWoken = pdFALSE;
  xSemaphoreGiveFromISR(g_ackSemaphore, &higherPriorityWoken);
  if (higherPriorityWoken) portYIELD_FROM_ISR();
}

static void createTask(TaskFunction_t fn, const char* name, uint32_t stack,
                       UBaseType_t prio, TaskHandle_t* handle, BaseType_t core) {
  if (xTaskCreatePinnedToCore(fn, name, stack, nullptr, prio, handle, core) != pdPASS) {
    Serial.printf("FATAL: could not create task %s\n", name);
    while (true) delay(1000);
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n=== FreeRTOS Multi-Task Sensor Monitor ===");

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  analogReadResolution(12);

  g_sensorQueue  = xQueueCreate(SENSOR_QUEUE_LEN, sizeof(SensorReading));
  g_stateMutex   = xSemaphoreCreateMutex();
  g_serialMutex  = xSemaphoreCreateMutex();
  g_ackSemaphore = xSemaphoreCreateBinary();
  g_alarmEvents  = xEventGroupCreate();
  g_heartbeats   = xEventGroupCreate();

  if (!g_sensorQueue || !g_stateMutex || !g_serialMutex || !g_ackSemaphore ||
      !g_alarmEvents || !g_heartbeats) {
    Serial.println("FATAL: RTOS object allocation failed");
    while (true) delay(1000);
  }

  attachInterrupt(digitalPinToInterrupt(PIN_BUTTON), onButtonIsr, FALLING);

  // Time-critical work on core 1, user-interface/telemetry on core 0.
  createTask(alarmTask,      "alarm",      STACK_ALARM,      PRIO_ALARM,      &g_alarmTask,      1);
  createTask(sensorTask,     "sensor",     STACK_SENSOR,     PRIO_SENSOR,     &g_sensorTask,     1);
  createTask(processingTask, "processing", STACK_PROCESSING, PRIO_PROCESSING, &g_processingTask, 1);
  createTask(healthTask,     "health",     STACK_HEALTH,     PRIO_HEALTH,     &g_healthTask,     0);
  createTask(displayTask,    "display",    STACK_DISPLAY,    PRIO_DISPLAY,    &g_displayTask,    0);
  createTask(telemetryTask,  "telemetry",  STACK_TELEMETRY,  PRIO_TELEMETRY,  &g_telemetryTask,  0);

  logMsg("All tasks started. Free heap: %u bytes", (unsigned)ESP.getFreeHeap());
}

void loop() {
  // All work happens in FreeRTOS tasks; remove the Arduino loop task.
  vTaskDelete(nullptr);
}
