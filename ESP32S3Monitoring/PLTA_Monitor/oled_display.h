#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

namespace PltaOled {

inline void drawStartup(
  Adafruit_SH1107 &display,
  const char *statusLine
) {
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);

  display.drawRect(0, 0, 128, 128, SH110X_WHITE);
  display.drawFastHLine(8, 48, 112, SH110X_WHITE);

  display.setTextSize(2);
  display.setCursor(17, 11);
  display.print("PLTA");
  display.setCursor(8, 31);
  display.print("MONITOR");

  display.setTextSize(1);
  display.setCursor(16, 72);
  display.print("SYSTEM STATUS");

  display.setTextSize(1);
  display.setCursor(8, 91);
  display.print(statusLine);

  display.setCursor(8, 113);
  display.print("DB-268 / INA219");
  display.display();
}

inline void drawHeader(
  Adafruit_SH1107 &display,
  bool sensorOk,
  bool ina219Ok,
  uint32_t uptimeMs
) {
  const uint32_t uptimeSeconds = uptimeMs / 1000;
  const uint32_t hours = uptimeSeconds / 3600;
  const uint32_t minutes = (uptimeSeconds % 3600) / 60;
  const uint32_t seconds = uptimeSeconds % 60;
  char uptimeText[12];
  snprintf(
    uptimeText,
    sizeof(uptimeText),
    "%02lu:%02lu:%02lu",
    (unsigned long)hours,
    (unsigned long)minutes,
    (unsigned long)seconds
  );

  display.setTextSize(1);
  display.setCursor(5, 5);
  display.print("PLTA MONITOR");

  display.setCursor(5, 118);
  display.print("T:");
  display.print(uptimeText);

  display.setCursor(103, 5);
  display.print(!ina219Ok ? "ERR" : (sensorOk ? "OK" : "..."));

  display.drawFastHLine(0, 15, 128, SH110X_WHITE);
}

inline void drawMetric(
  Adafruit_SH1107 &display,
  int y,
  const char *label,
  const char *value,
  bool valid
) {
  display.setTextSize(1);
  display.setCursor(5, y);
  display.print(label);

  display.drawFastHLine(0, y + 10, 128, SH110X_WHITE);

  display.setTextSize(2);
  display.setCursor(5, y + 15);
  if (valid) {
    display.print(value);
  } else {
    display.print("---");
  }
}

inline void drawStatus(
  Adafruit_SH1107 &display,
  bool sensorOk,
  bool oledOk,
  bool wifiConnected
) {
  display.setTextSize(1);
  display.setCursor(70, 118);
  display.print("I:");
  display.print(oledOk ? "O" : "-");
  display.setCursor(91, 118);
  display.print("S:");
  display.print(sensorOk ? "O" : "-");
  display.setCursor(112, 118);
  display.print("W");
  display.print(wifiConnected ? "O" : "-");
}

inline void drawLive(
  Adafruit_SH1107 &display,
  float voltageV,
  float currentMA,
  float powerMW,
  bool sensorOk,
  bool ina219Ok,
  bool oledOk,
  bool wifiConnected,
  uint32_t uptimeMs
) {
  char voltageText[18];
  char currentText[18];
  char powerText[18];

  if (sensorOk) {
    snprintf(
      voltageText,
      sizeof(voltageText),
      "%.2f V",
      voltageV
    );
    snprintf(
      currentText,
      sizeof(currentText),
      "%.1f mA",
      currentMA
    );
    snprintf(
      powerText,
      sizeof(powerText),
      "%.0f mW",
      powerMW
    );
  } else {
    snprintf(voltageText, sizeof(voltageText), "---");
    snprintf(currentText, sizeof(currentText), "---");
    snprintf(powerText, sizeof(powerText), "---");
  }

  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);

  drawHeader(display, sensorOk, ina219Ok, uptimeMs);
  drawMetric(display, 21, "VOLTAGE", voltageText, sensorOk);
  drawMetric(display, 53, "CURRENT", currentText, sensorOk);
  drawMetric(display, 85, "POWER", powerText, sensorOk);
  drawStatus(display, sensorOk, oledOk, wifiConnected);

  display.display();
}

} // namespace PltaOled
