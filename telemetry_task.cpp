// Telemetry task: streams one JSON line per second over serial, ready to
// be forwarded to a dashboard (e.g. a serial-to-MQTT bridge or Grafana).

#include "app.h"

void telemetryTask(void*) {
  TickType_t lastWake = xTaskGetTickCount();
  for (;;) {
    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(TELEMETRY_PERIOD_MS));
    const SystemState s = snapshotState();
    logMsg("{\"t_ms\":%lu,\"temp_c\":%.1f,\"hum_pct\":%.1f,\"current_a\":%.2f,"
         "\"alarm_temp\":%s,\"alarm_current\":%s,\"fault\":%s,\"ack\":%s,"
         "\"samples\":%lu,\"drops\":%lu,\"sensor_errors\":%lu}",
         (unsigned long)millis(),
         isnan(s.temperatureC) ? -99.0f : s.temperatureC,
         isnan(s.humidityPct) ? -99.0f : s.humidityPct,
         s.currentA,
         s.tempAlarm ? "true" : "false", s.currentAlarm ? "true" : "false",
         s.faultAlarm ? "true" : "false", s.acknowledged ? "true" : "false",
         (unsigned long)s.samplesProcessed, (unsigned long)s.queueDrops,
         (unsigned long)s.sensorErrors);
    xEventGroupSetBits(g_heartbeats, HB_TELEMETRY);
  }
}
