// Processing task: filters readings with moving averages, applies
// hysteresis thresholds and publishes alarm state via an event group.

#include "app.h"
#include "signal_processing.h"

void processingTask(void*) {
  sp::MovingAverage<8> currentAvg;
  sp::MovingAverage<4> tempAvg;
  sp::HysteresisThreshold tempLimit(TEMP_ALARM_C, TEMP_HYSTERESIS_C);
  sp::HysteresisThreshold currentLimit(CURRENT_ALARM_A, CURRENT_HYST_A);
  uint32_t lastDhtSampleMs = 0;

  for (;;) {
    SensorReading r;
    if (xQueueReceive(g_sensorQueue, &r, pdMS_TO_TICKS(1000)) == pdTRUE) {
      const float current = currentAvg.add(r.currentA);
      const bool currentAlarm = currentLimit.update(current);

      float temp = tempAvg.value();
      bool tempAlarm = tempLimit.active();
      // Only feed the temperature filter when a fresh DHT sample arrives.
      if (r.dhtValid && (lastDhtSampleMs == 0 || r.timestampMs - lastDhtSampleMs >= DHT_PERIOD_MS)) {
        lastDhtSampleMs = r.timestampMs;
        temp = tempAvg.add(r.temperatureC);
        tempAlarm = tempLimit.update(temp);
      }

      if (xSemaphoreTake(g_stateMutex, pdMS_TO_TICKS(20)) == pdTRUE) {
        g_state.currentA = current;
        if (tempAvg.count() > 0) {
          g_state.temperatureC = temp;
          g_state.humidityPct = r.humidityPct;
        }
        g_state.tempAlarm = tempAlarm;
        g_state.currentAlarm = currentAlarm;
        g_state.samplesProcessed++;
        xSemaphoreGive(g_stateMutex);
      }

      if (tempAlarm) xEventGroupSetBits(g_alarmEvents, EVT_ALARM_TEMP);
      else           xEventGroupClearBits(g_alarmEvents, EVT_ALARM_TEMP);
      if (currentAlarm) xEventGroupSetBits(g_alarmEvents, EVT_ALARM_CURRENT);
      else              xEventGroupClearBits(g_alarmEvents, EVT_ALARM_CURRENT);
    }
    xEventGroupSetBits(g_heartbeats, HB_PROCESSING);
  }
}
