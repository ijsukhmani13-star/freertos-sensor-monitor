// Alarm task (highest priority): blinks the LED while an alarm is active
// and unacknowledged, holds it solid once acknowledged via the button ISR,
// and resets the acknowledgement when all alarms clear.

#include "app.h"

void alarmTask(void*) {
  bool ledOn = false;
  bool acknowledged = false;
  uint32_t lastAckMs = 0;
  uint32_t lastToggleMs = 0;

  for (;;) {
    // Button press from the ISR (debounced here, not in the ISR).
    if (xSemaphoreTake(g_ackSemaphore, 0) == pdTRUE) {
      const uint32_t now = millis();
      if (now - lastAckMs > BUTTON_DEBOUNCE_MS) {
        lastAckMs = now;
        if (xEventGroupGetBits(g_alarmEvents) & EVT_ALARM_ANY) {
          acknowledged = true;
          logMsg("[alarm] acknowledged by operator");
        }
      }
    }

    const EventBits_t bits = xEventGroupGetBits(g_alarmEvents);
    const bool anyAlarm = bits & EVT_ALARM_ANY;

    if (!anyAlarm) {
      if (acknowledged) logMsg("[alarm] all alarms cleared");
      acknowledged = false;
      ledOn = false;
    } else if (acknowledged) {
      ledOn = true;                                  // solid = acknowledged
    } else if (millis() - lastToggleMs >= 125) {     // 4 Hz blink = new alarm
      lastToggleMs = millis();
      ledOn = !ledOn;
    }
    digitalWrite(PIN_LED, ledOn ? HIGH : LOW);

    if (xSemaphoreTake(g_stateMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
      g_state.acknowledged = acknowledged;
      xSemaphoreGive(g_stateMutex);
    }

    xEventGroupSetBits(g_heartbeats, HB_ALARM);
    vTaskDelay(pdMS_TO_TICKS(25));
  }
}
