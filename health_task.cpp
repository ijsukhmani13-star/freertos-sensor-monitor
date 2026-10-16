// Health task: a software watchdog. Every task sets a heartbeat bit; if any
// bit is missing within HEALTH_PERIOD_MS the task is reported as stalled and
// a FAULT alarm is raised. Also reports stack high-water marks and heap.

#include "app.h"

static void reportStack(const char* name, TaskHandle_t h) {
  if (h) logMsg("  %-11s stack free: %4u bytes", name, (unsigned)uxTaskGetStackHighWaterMark(h));
}

void healthTask(void*) {
  uint32_t lastStatsMs = 0;
  vTaskDelay(pdMS_TO_TICKS(HEALTH_PERIOD_MS));  // let all tasks start
  xEventGroupClearBits(g_heartbeats, HB_ALL);

  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(HEALTH_PERIOD_MS));
    const EventBits_t seen = xEventGroupClearBits(g_heartbeats, HB_ALL);  // returns bits before clear
    const EventBits_t missing = HB_ALL & ~seen;
    const bool fault = missing != 0;

    if (fault) {
      logMsg("[health] FAULT - missing heartbeats: %s%s%s%s%s",
           (missing & HB_SENSOR) ? "sensor " : "", (missing & HB_PROCESSING) ? "processing " : "",
           (missing & HB_DISPLAY) ? "display " : "", (missing & HB_TELEMETRY) ? "telemetry " : "",
           (missing & HB_ALARM) ? "alarm" : "");
      xEventGroupSetBits(g_alarmEvents, EVT_ALARM_FAULT);
    } else {
      xEventGroupClearBits(g_alarmEvents, EVT_ALARM_FAULT);
    }
    if (xSemaphoreTake(g_stateMutex, pdMS_TO_TICKS(20)) == pdTRUE) {
      g_state.faultAlarm = fault;
      xSemaphoreGive(g_stateMutex);
    }

    if (millis() - lastStatsMs >= STATS_PERIOD_MS) {
      lastStatsMs = millis();
      logMsg("[health] free heap: %u bytes (min ever %u)", (unsigned)ESP.getFreeHeap(),
           (unsigned)ESP.getMinFreeHeap());
      reportStack("sensor", g_sensorTask);
      reportStack("processing", g_processingTask);
      reportStack("alarm", g_alarmTask);
      reportStack("display", g_displayTask);
      reportStack("telemetry", g_telemetryTask);
      reportStack("health", g_healthTask);
    }
  }
}
