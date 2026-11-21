#include <Arduino.h>
#include <esp_system.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_INA219.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ESPAsyncWebServer.h>
#include <FFat.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include "web_assets_gz.h"

#define OLED_I2C_SDA_PIN 8
#define OLED_I2C_SCL_PIN 9
#define INA219_I2C_SDA_PIN 16
#define INA219_I2C_SCL_PIN 17

#define OLED_WIDTH 128
#define OLED_HEIGHT 128
#define OLED_RESET -1
#define OLED_DEFAULT_ADDRESS 0x3C

#define INA219_DEFAULT_ADDRESS 0x40

#define WIFI_AP_SSID "PLTA-Monitor"
#define WIFI_AP_PASSWORD "ChangeThisPassword"
#define WIFI_AP_CHANNEL 1
#define WIFI_AP_MAX_CONNECTIONS 4
#define MDNS_HOSTNAME "plta-monitor"
#define RGB_LED_PIN 48
#define RGB_LED_COUNT 1
#define ALARM_LOW_VOLTAGE_V 1.5f
#define ALARM_HIGH_CURRENT_MA 1000.0f
#define ALARM_LOW_POWER_MW 50.0f

static const uint32_t I2C_CLOCK_HZ = 100000;
static const uint32_t SENSOR_INTERVAL_MS = 500;
static const uint32_t OLED_INTERVAL_MS = 1000;
static const uint32_t SERIAL_INTERVAL_MS = 2000;
static const uint32_t WIFI_RECONNECT_INTERVAL_MS = 5000;
static const uint32_t RGB_UPDATE_INTERVAL_MS = 500;
static const float CURRENT_ZERO_THRESHOLD_MA = 1.0f;
static const float VOLTAGE_ZERO_THRESHOLD_V = 0.05f;
static const float SHUNT_ZERO_THRESHOLD_MV = 0.10f;

Adafruit_SH1107 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);
Adafruit_INA219 ina219(INA219_DEFAULT_ADDRESS);

uint8_t oledAddress = 0;
bool oledOk = false;
bool ina219Ok = false;
bool sensorOk = false;

float generatorVoltage_V = 0.0f;
float current_mA = 0.0f;
float shuntVoltage_mV = 0.0f;
float busVoltage_V = 0.0f;
float generatorPower_mW = 0.0f;
float currentZeroOffset_mA = 0.0f;
float busVoltageZeroOffset_V = 0.0f;
float generatorVoltageZeroOffset_V = 0.0f;

uint32_t lastSensorReadMs = 0;
uint32_t lastOledUpdateMs = 0;
uint32_t lastSerialReportMs = 0;
uint32_t lastWifiReconnectMs = 0;
uint32_t lastRgbUpdateMs = 0;
bool mdnsStarted = false;
bool restartPending = false;
uint32_t restartAtMs = 0;
Adafruit_NeoPixel rgbLed(RGB_LED_COUNT, RGB_LED_PIN, NEO_GRB + NEO_KHZ800);

AsyncWebServer server(80);

struct MeasurementSnapshot {
  float voltageV;
  float currentMA;
  float powerMW;
  float busVoltageV;
  float shuntVoltageMV;
  bool sensorOk;
};

MeasurementSnapshot measurementSnapshot = {
  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, false
};

portMUX_TYPE measurementMux = portMUX_INITIALIZER_UNLOCKED;

void updateMeasurementSnapshot() {
  portENTER_CRITICAL(&measurementMux);
  measurementSnapshot.voltageV = generatorVoltage_V;
  measurementSnapshot.currentMA = current_mA;
  measurementSnapshot.powerMW = generatorPower_mW;
  measurementSnapshot.busVoltageV = busVoltage_V;
  measurementSnapshot.shuntVoltageMV = shuntVoltage_mV;
  measurementSnapshot.sensorOk = sensorOk;
  portEXIT_CRITICAL(&measurementMux);
}

void scanI2cBus() {
  Serial.println("OLED I2C scan:");
  uint8_t count = 0;

  for (uint8_t address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.printf("  0x%02X", address);

      if (address == OLED_DEFAULT_ADDRESS || address == 0x3D) {
        Serial.print(" OLED");
      }

      Serial.println();
      count++;

      if (address == OLED_DEFAULT_ADDRESS || address == 0x3D) {
        if (oledAddress == 0) {
          oledAddress = address;
        }
      }
    }
  }

  if (count == 0) {
    Serial.println("  No I2C devices found.");
  }
  Serial.printf("I2C devices found: %u\n", count);
}

bool ina219AddressFound() {
  Wire1.beginTransmission(INA219_DEFAULT_ADDRESS);
  return Wire1.endTransmission() == 0;
}

void calibrateZeroOffsets() {
  if (!ina219Ok) {
    currentZeroOffset_mA = 0.0f;
    busVoltageZeroOffset_V = 0.0f;
    generatorVoltageZeroOffset_V = 0.0f;
    return;
  }

  constexpr uint8_t SAMPLE_COUNT = 20;
  float currentOffsetSum = 0.0f;
  float busOffsetSum = 0.0f;
  float generatorOffsetSum = 0.0f;
  uint8_t validSamples = 0;

  for (uint8_t i = 0; i < SAMPLE_COUNT; i++) {
    const float rawCurrentMA = ina219.getCurrent_mA();
    const float rawBusV = ina219.getBusVoltage_V();
    const float rawShuntMV = ina219.getShuntVoltage_mV();
    const float rawGeneratorV = rawBusV + (rawShuntMV / 1000.0f);

    if (!isnan(rawCurrentMA) && !isinf(rawCurrentMA) &&
        rawCurrentMA > -100.0f && rawCurrentMA < 100.0f &&
        !isnan(rawBusV) && !isinf(rawBusV) && rawBusV >= 0.0f &&
        rawBusV <= 26.0f &&
        !isnan(rawShuntMV) && !isinf(rawShuntMV)) {
      currentOffsetSum += rawCurrentMA;
      busOffsetSum += rawBusV;
      generatorOffsetSum += rawGeneratorV;
      validSamples++;
    }
    delay(20);
  }

  if (validSamples > 0) {
    currentZeroOffset_mA = currentOffsetSum / validSamples;
    busVoltageZeroOffset_V = busOffsetSum / validSamples;
    generatorVoltageZeroOffset_V = generatorOffsetSum / validSamples;
  } else {
    currentZeroOffset_mA = 0.0f;
    busVoltageZeroOffset_V = 0.0f;
    generatorVoltageZeroOffset_V = 0.0f;
  }

  Serial.printf(
    "Zero calibration: bus %.3f V | generator %.3f V | current %.3f mA (%u samples)\n",
    busVoltageZeroOffset_V,
    generatorVoltageZeroOffset_V,
    currentZeroOffset_mA,
    validSamples
  );
}

void scanIna219Bus() {
  Serial.println("INA219 I2C scan:");
  bool found = false;

  for (uint8_t address = 1; address < 127; address++) {
    Wire1.beginTransmission(address);
    if (Wire1.endTransmission() == 0) {
      Serial.printf("  0x%02X", address);
      if (address == INA219_DEFAULT_ADDRESS) {
        Serial.print(" INA219");
        found = true;
      }
      Serial.println();
    }
  }

  if (!found) {
    Serial.printf(
      "  INA219 address 0x%02X not found.\n",
      INA219_DEFAULT_ADDRESS
    );
  }
}

bool measurementsAreValid(float busV, float shuntMV, float currentMA) {
  if (isnan(busV) || isnan(shuntMV) || isnan(currentMA)) {
    return false;
  }
  if (isinf(busV) || isinf(shuntMV) || isinf(currentMA)) {
    return false;
  }
  if (busV < 0.0f || busV > 26.0f) {
    return false;
  }

  const float shuntVoltageEstimate = busV + (shuntMV / 1000.0f);
  if (shuntVoltageEstimate < 0.0f || shuntVoltageEstimate > 26.5f) {
    return false;
  }
  if (currentMA < -10000.0f || currentMA > 10000.0f) {
    return false;
  }

  return true;
}

void readMeasurements() {
  if (!ina219Ok) {
    sensorOk = false;
    updateMeasurementSnapshot();
    return;
  }

  const float newBusV = ina219.getBusVoltage_V();
  const float newShuntMV = ina219.getShuntVoltage_mV();
  const float newCurrentMA = ina219.getCurrent_mA();

  if (!measurementsAreValid(newBusV, newShuntMV, newCurrentMA)) {
    sensorOk = false;
    Serial.println("INA219: invalid or implausible reading");
    updateMeasurementSnapshot();
    return;
  }

  const float calibratedBusV = newBusV - busVoltageZeroOffset_V;
  if (fabs(calibratedBusV) < VOLTAGE_ZERO_THRESHOLD_V) {
    busVoltage_V = 0.0f;
  } else {
    busVoltage_V = calibratedBusV;
  }

  if (fabs(newShuntMV) < SHUNT_ZERO_THRESHOLD_MV) {
    shuntVoltage_mV = 0.0f;
  } else {
    shuntVoltage_mV = newShuntMV;
  }

  const float calibratedCurrentMA = newCurrentMA - currentZeroOffset_mA;

  if (fabs(calibratedCurrentMA) < CURRENT_ZERO_THRESHOLD_MA) {
    current_mA = 0.0f;
  } else {
    current_mA = calibratedCurrentMA;
  }
  const float rawGeneratorV = newBusV + (newShuntMV / 1000.0f);
  const float calibratedGeneratorV =
    rawGeneratorV - generatorVoltageZeroOffset_V;
  if (fabs(calibratedGeneratorV) < VOLTAGE_ZERO_THRESHOLD_V) {
    generatorVoltage_V = 0.0f;
  } else {
    generatorVoltage_V = calibratedGeneratorV;
  }
  generatorPower_mW = generatorVoltage_V * current_mA;
  sensorOk = true;
  updateMeasurementSnapshot();
}

void setupFFat() {
  Serial.println(
    "FFat: not mounted (dashboard uses embedded gzip assets)"
  );
}

void setRgbAlarm(const char *state) {
  if (strcmp(state, "sensor") == 0) {
    rgbLed.setPixelColor(0, rgbLed.Color(255, 0, 0));
  } else if (strcmp(state, "warning") == 0) {
    rgbLed.setPixelColor(0, rgbLed.Color(255, 96, 0));
  } else if (strcmp(state, "connected") == 0) {
    rgbLed.setPixelColor(0, rgbLed.Color(0, 255, 80));
  } else {
    rgbLed.setPixelColor(0, 0, 0, 0);
  }
  rgbLed.show();
}

void updateRgbAlarm() {
  if (!sensorOk || !ina219Ok) {
    setRgbAlarm("sensor");
  } else if (
    generatorVoltage_V < ALARM_LOW_VOLTAGE_V ||
    current_mA > ALARM_HIGH_CURRENT_MA ||
    generatorPower_mW < ALARM_LOW_POWER_MW
  ) {
    setRgbAlarm("warning");
  } else {
    setRgbAlarm("connected");
  }
}

void sendGzipAsset(
  AsyncWebServerRequest *request,
  const uint8_t *asset,
  unsigned int length,
  const char *contentType,
  const char *cacheControl
) {
  AsyncWebServerResponse *response =
    request->beginResponse_P(200, contentType, asset, length);
  response->addHeader("Content-Encoding", "gzip");
  response->addHeader("Cache-Control", cacheControl);
  response->addHeader("X-Content-Type-Options", "nosniff");
  request->send(response);
}

void setupWebServer() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    sendGzipAsset(
      request,
      index_html_gz,
      index_html_gz_len,
      "text/html",
      "max-age=60"
    );
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    sendGzipAsset(
      request,
      style_css_gz,
      style_css_gz_len,
      "text/css",
      "max-age=604800, public"
    );
  });

  server.on("/app.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    sendGzipAsset(
      request,
      app_js_gz,
      app_js_gz_len,
      "application/javascript",
      "max-age=604800, public"
    );
  });

  server.on("/api/data", HTTP_GET, [](AsyncWebServerRequest *request) {
    MeasurementSnapshot snapshot;
    portENTER_CRITICAL(&measurementMux);
    snapshot = measurementSnapshot;
    portEXIT_CRITICAL(&measurementMux);

    StaticJsonDocument<256> json;
    json["voltage_v"] = snapshot.voltageV;
    json["current_ma"] = snapshot.currentMA;
    json["power_mw"] = snapshot.powerMW;
    json["bus_voltage_v"] = snapshot.busVoltageV;
    json["shunt_voltage_mv"] = snapshot.shuntVoltageMV;
    json["sensor_ok"] = snapshot.sensorOk;
    json["wifi_connected"] = WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA;
    json["uptime_ms"] = millis();
    json["free_heap_bytes"] = ESP.getFreeHeap();
    json["wifi_clients"] = WiFi.softAPgetStationNum();
    json["rgb_led_pin"] = RGB_LED_PIN;
    json["firmware_version"] = "2.0.0";

    AsyncResponseStream *response =
      request->beginResponseStream("application/json");
    response->addHeader("Cache-Control", "no-store");
    response->addHeader("X-Content-Type-Options", "nosniff");
    serializeJson(json, *response);
    request->send(response);
  });

  server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    MeasurementSnapshot snapshot;
    portENTER_CRITICAL(&measurementMux);
    snapshot = measurementSnapshot;
    portEXIT_CRITICAL(&measurementMux);

    StaticJsonDocument<512> json;
    json["sensor_ok"] = snapshot.sensorOk;
    json["ina219_ok"] = ina219Ok;
    json["oled_ok"] = oledOk;
    json["wifi_connected"] =
      WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA;
    json["wifi_ssid"] = WiFi.softAPSSID();
    json["ip_address"] = WiFi.softAPIP().toString();
    json["zero_voltage_v"] =
      generatorVoltageZeroOffset_V;
    json["zero_bus_voltage_v"] =
      busVoltageZeroOffset_V;
    json["zero_current_ma"] =
      currentZeroOffset_mA;
    json["uptime_ms"] = millis();
    json["free_heap_bytes"] = ESP.getFreeHeap();
    json["wifi_clients"] = WiFi.softAPgetStationNum();
    json["rgb_led_pin"] = RGB_LED_PIN;
    json["alarm_low_voltage_v"] = ALARM_LOW_VOLTAGE_V;
    json["alarm_high_current_ma"] = ALARM_HIGH_CURRENT_MA;
    json["alarm_low_power_mw"] = ALARM_LOW_POWER_MW;
    json["firmware_version"] = "2.0.0";

    AsyncResponseStream *response =
      request->beginResponseStream("application/json");
    response->addHeader("Cache-Control", "no-store");
    response->addHeader("X-Content-Type-Options", "nosniff");
    serializeJson(json, *response);
    request->send(response);
  });

  server.on("/api/calibrate", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!ina219Ok) {
      request->send(503, "application/json",
                    "{\"calibrated\":false,\"error\":\"INA219 unavailable\"}");
      return;
    }

    calibrateZeroOffsets();
    request->send(200, "application/json",
                  "{\"calibrated\":true}");
  });

  server.on("/api/restart", HTTP_POST, [](AsyncWebServerRequest *request) {
    Serial.println("Restart requested from Web dashboard");
    restartPending = true;
    restartAtMs = millis() + 500;
    AsyncResponseStream *response =
      request->beginResponseStream("application/json");
    response->addHeader("Cache-Control", "no-store");
    StaticJsonDocument<64> json;
    json["restart_scheduled"] = true;
    serializeJson(json, *response);
    request->send(response);
  });

  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
  });

  server.begin();
  Serial.println("Async web server: started on port 80");
}

void drawStartupScreen(const char *statusLine) {
  if (!oledOk) {
    return;
  }

  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(2);
  display.setCursor(12, 8);
  display.print("PLTA");
  display.setCursor(12, 34);
  display.print("MONITOR");
  display.setTextSize(1);
  display.setCursor(12, 66);
  display.print(statusLine);
  display.display();
}

void drawOledStatus() {
  if (!oledOk) {
    return;
  }

  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);

  display.setTextSize(1);
  display.setCursor(24, 8);
  display.print("PLTA MONITOR");

  display.drawFastHLine(0, 24, OLED_WIDTH, SH110X_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 30);
  if (sensorOk) {
    display.print("GENERATOR STATUS: OK");
  } else if (ina219Ok) {
    display.print("GENERATOR STATUS: INVALID");
  } else {
    display.print("SENSOR ERROR");
  }

  display.setCursor(0, 43);
  display.print("VOLTAGE");
  display.setTextSize(2);
  display.setCursor(0, 54);
  if (sensorOk) {
    display.print(generatorVoltage_V, 2);
    display.print(" V");
  } else {
    display.print("--.- V");
  }

  display.setTextSize(1);
  display.setCursor(0, 78);
  display.print("CURRENT");
  display.setTextSize(2);
  display.setCursor(0, 89);
  if (sensorOk) {
    display.print(current_mA, 1);
    display.print(" mA");
  } else {
    display.print("--- mA");
  }

  display.setTextSize(1);
  display.setCursor(0, 113);
  if (sensorOk) {
    display.print("P:");
    display.print(generatorPower_mW, 0);
    display.print(" mW");
  } else {
    display.print("P: --- mW");
  }

  display.display();
}

void setup() {
  Serial.begin(115200);
  delay(300);

  Serial.println();
  Serial.println("PLTA DB-268 monitoring firmware");
  Serial.println("MONITORING ONLY - no generator control outputs");
  Serial.printf(
    "OLED I2C: SDA GPIO%d, SCL GPIO%d\n",
    OLED_I2C_SDA_PIN,
    OLED_I2C_SCL_PIN
  );
  Serial.printf(
    "INA219 I2C: SDA GPIO%d, SCL GPIO%d\n",
    INA219_I2C_SDA_PIN,
    INA219_I2C_SCL_PIN
  );

  Wire.begin(OLED_I2C_SDA_PIN, OLED_I2C_SCL_PIN);
  Wire.setClock(I2C_CLOCK_HZ);
  Wire1.begin(INA219_I2C_SDA_PIN, INA219_I2C_SCL_PIN);
  Wire1.setClock(I2C_CLOCK_HZ);

  scanI2cBus();
  scanIna219Bus();

  if (oledAddress != 0) {
    Serial.printf("Initializing OLED at 0x%02X\n", oledAddress);
    oledOk = display.begin(oledAddress, true);
  } else {
    Serial.println("OLED: not detected");
  }

  if (!oledOk) {
    Serial.println("OLED: initialization failed");
  } else {
    Serial.println("OLED: OK");
    drawStartupScreen("Initializing INA219...");
  }

  if (!ina219AddressFound()) {
    Serial.printf("INA219: not detected at 0x%02X\n", INA219_DEFAULT_ADDRESS);
  } else {
    ina219Ok = ina219.begin(&Wire1);
    if (!ina219Ok) {
      Serial.println("INA219: detected but initialization failed");
    } else {
      ina219.setCalibration_32V_1A();
      Serial.printf(
        "INA219: OK at 0x%02X (32 V / 1 A calibration)\n",
        INA219_DEFAULT_ADDRESS
      );
      calibrateZeroOffsets();

      if (oledOk) {
        drawStartupScreen("INA219 OK");
      }
    }
  }

  if (!ina219Ok && oledOk) {
    drawStartupScreen("SENSOR ERROR");
  }

  setupFFat();

  rgbLed.begin();
  rgbLed.setBrightness(64);
  rgbLed.clear();
  rgbLed.show();
  setRgbAlarm("warning");

  WiFi.mode(WIFI_AP);
  WiFi.softAP(
    WIFI_AP_SSID,
    WIFI_AP_PASSWORD,
    WIFI_AP_CHANNEL,
    false,
    WIFI_AP_MAX_CONNECTIONS
  );
  Serial.printf("Wi-Fi AP: %s\n", WIFI_AP_SSID);
  Serial.printf("Wi-Fi AP IP: %s\n", WiFi.softAPIP().toString().c_str());
  Serial.printf("Wi-Fi password: %s\n", WIFI_AP_PASSWORD);

  if (MDNS.begin(MDNS_HOSTNAME)) {
    MDNS.addService("http", "tcp", 80);
    Serial.printf("mDNS: http://%s.local/\n", MDNS_HOSTNAME);
    mdnsStarted = true;
  } else {
    Serial.println("mDNS: failed to start");
  }

  setupWebServer();

  lastSensorReadMs = millis();
  lastOledUpdateMs = millis();
  lastSerialReportMs = millis();
  lastWifiReconnectMs = millis();
  lastRgbUpdateMs = millis();
  readMeasurements();
  drawOledStatus();
}

void loop() {
  const uint32_t now = millis();

  if ((uint32_t)(now - lastSensorReadMs) >= SENSOR_INTERVAL_MS) {
    lastSensorReadMs = now;
    readMeasurements();
  }

  if ((uint32_t)(now - lastOledUpdateMs) >= OLED_INTERVAL_MS) {
    lastOledUpdateMs = now;
    drawOledStatus();
  }

  if ((uint32_t)(now - lastRgbUpdateMs) >= RGB_UPDATE_INTERVAL_MS) {
    lastRgbUpdateMs = now;
    updateRgbAlarm();
  }

  if (restartPending &&
      (int32_t)(now - restartAtMs) >= 0) {
    rgbLed.clear();
    rgbLed.show();
    ESP.restart();
  }

  if ((uint32_t)(now - lastWifiReconnectMs) >= WIFI_RECONNECT_INTERVAL_MS) {
    lastWifiReconnectMs = now;

    if (!mdnsStarted) {
      if (MDNS.begin(MDNS_HOSTNAME)) {
        MDNS.addService("http", "tcp", 80);
        mdnsStarted = true;
        Serial.printf(
          "mDNS: http://%s.local/ at %s\n",
          MDNS_HOSTNAME,
          WiFi.softAPIP().toString().c_str()
        );
      }
    }
  }

  if ((uint32_t)(now - lastSerialReportMs) >= SERIAL_INTERVAL_MS) {
    lastSerialReportMs = now;

    if (!ina219Ok) {
      Serial.println("INA219: unavailable");
    } else if (!sensorOk) {
      Serial.println("INA219: invalid measurement");
    } else {
      Serial.printf(
        "Generator: %.3f V | %.3f mA | %.3f mW | bus %.3f V | shunt %.3f mV\n",
        generatorVoltage_V,
        current_mA,
        generatorPower_mW,
        busVoltage_V,
        shuntVoltage_mV
      );
    }
  }
}
