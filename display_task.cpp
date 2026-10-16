// Display task: renders the current state on a 128x64 SSD1306 OLED.

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "app.h"

void displayTask(void*) {
  Wire.begin(PIN_SDA, PIN_SCL);
  Adafruit_SSD1306 oled(128, 64, &Wire, -1);
  const bool ok = oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  if (!ok) logMsg("[display] SSD1306 not found - continuing without display");

  TickType_t lastWake = xTaskGetTickCount();
  for (;;) {
    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(DISPLAY_PERIOD_MS));
    const SystemState s = snapshotState();

    if (ok) {
      oled.clearDisplay();
      oled.setTextColor(SSD1306_WHITE);
      oled.setTextSize(1);
      oled.setCursor(0, 0);
      oled.println("FreeRTOS Monitor");
      oled.drawLine(0, 9, 127, 9, SSD1306_WHITE);
      oled.setCursor(0, 14);
      if (isnan(s.temperatureC)) oled.println("Temp:  --.- C");
      else oled.printf("Temp:  %5.1f C\n", s.temperatureC);
      if (isnan(s.humidityPct)) oled.println("Hum:   --.- %");
      else oled.printf("Hum:   %5.1f %%\n", s.humidityPct);
      oled.printf("Load:  %5.1f A\n", s.currentA);
      oled.setCursor(0, 48);
      if (s.faultAlarm)        oled.println("FAULT: task stalled");
      else if (s.tempAlarm || s.currentAlarm) {
        oled.printf("ALARM %s%s\n", s.tempAlarm ? "TEMP " : "", s.currentAlarm ? "LOAD" : "");
        oled.println(s.acknowledged ? "(acknowledged)" : "press button = ACK");
      } else                   oled.println("Status: OK");
      oled.display();
    }
    xEventGroupSetBits(g_heartbeats, HB_DISPLAY);
  }
}
