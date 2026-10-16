// Sensor task: samples the "current" ADC at 10 Hz and the DHT22 every 2 s,
// then pushes a SensorReading into the queue. Uses vTaskDelayUntil for a
// fixed sampling period with no drift.

#include <DHTesp.h>
#include "app.h"
#include "signal_processing.h"

void sensorTask(void*) {
  DHTesp dht;
  dht.setup(PIN_DHT22, DHTesp::DHT22);

  float lastTemp = NAN, lastHum = NAN;
  bool dhtValid = false;
  uint32_t lastDhtMs = 0;
  TickType_t lastWake = xTaskGetTickCount();

  for (;;) {
    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(ADC_PERIOD_MS));
    const uint32_t now = millis();

    if (lastDhtMs == 0 || now - lastDhtMs >= DHT_PERIOD_MS) {
      lastDhtMs = now;
      TempAndHumidity th = dht.getTempAndHumidity();
      if (dht.getStatus() == DHTesp::ERROR_NONE) {
        lastTemp = th.temperature;
        lastHum = th.humidity;
        dhtValid = true;
      } else {
        dhtValid = false;
        if (xSemaphoreTake(g_stateMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
          g_state.sensorErrors++;
          xSemaphoreGive(g_stateMutex);
        }
      }
    }

    SensorReading r;
    r.timestampMs = now;
    r.temperatureC = lastTemp;
    r.humidityPct = lastHum;
    r.currentA = sp::adcToUnits(analogRead(PIN_POT), ADC_MAX, CURRENT_FULL_SCALE);
    r.dhtValid = dhtValid;

    // Never block the sampler: if the consumer falls behind, count the drop.
    if (xQueueSend(g_sensorQueue, &r, 0) != pdTRUE) {
      if (xSemaphoreTake(g_stateMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        g_state.queueDrops++;
        xSemaphoreGive(g_stateMutex);
      }
    }
    xEventGroupSetBits(g_heartbeats, HB_SENSOR);
  }
}
