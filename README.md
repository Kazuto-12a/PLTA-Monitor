# PLTA ESP32-S3 Monitoring

ESP32-S3 firmware and web dashboard for monitoring a DB-268 hydroelectric generator with an INA219 current/voltage sensor.

## Features

- Generator voltage, current, and power monitoring
- 128x128 I2C SH1107 OLED display
- ESP32-S3 Wi-Fi access point
- Responsive dark web dashboard
- Live voltage, current, and power chart
- Session statistics and estimated energy
- CSV measurement export
- PNG chart export
- INA219 zero-calibration controls
- RGB LED alarm states
- ESP32 restart control
- Free heap, Wi-Fi client, and firmware status
- 10-minute reliability test with 2-minute interval records
- Single CSV export for the reliability test
- Three-point INA219 calibration comparison card
- CSV export for multimeter calibration results
- Gzip-compressed embedded web assets

## Repository Structure

```text
ESP32S3Monitoring/
  HO_Tugas_Rancang_PLTA.md
  PLTA_ESP32S3_OLED_INA219_Code_Spec.md
  PLTA_Monitor/
    PLTA_Monitor.ino
    webpage.h
    style.h
    app.h
    web_assets_gz.h
    oled_display.h
    preview/
      gen-gzip.js
      server.js
```

## Hardware

- ESP32-S3 DevKit
- DB-268 micro-hydro DC generator
- INA219 GY-219 current sensor
- 128x128 I2C SH1107-compatible OLED
- DC-DC Step-Up converter
- Independent regulated power supply for the ESP32
- Onboard ESP32-S3 RGB LED

## Default I2C Pins

| Device | SDA | SCL |
|---|---:|---:|
| OLED | GPIO8 | GPIO9 |
| INA219 | GPIO8 | GPIO9 |

Verify all GPIO assignments against the exact ESP32-S3 board pinout.

## Required Arduino Libraries

- Adafruit GFX Library
- Adafruit SH110X
- Adafruit INA219
- Adafruit NeoPixel
- ESPAsyncWebServer
- ArduinoJson

The firmware also uses the ESP32 Arduino libraries for Wi-Fi, mDNS, I2C, FFat, and the Arduino framework.

## Build and Upload

1. Open `ESP32S3Monitoring/PLTA_Monitor/PLTA_Monitor.ino` in Arduino IDE.
2. Select the correct ESP32-S3 board and USB port.
3. Install the required libraries.
4. Change `WIFI_AP_SSID` and `WIFI_AP_PASSWORD` near the top of the sketch.
5. Upload the firmware.

Default access point:

- SSID: `PLTA-Monitor`
- Dashboard: `http://192.168.4.1/`
- mDNS: `http://plta-monitor.local/`

Change `WIFI_AP_PASSWORD` before deployment. The checked-in placeholder is not a secure credential.

## Dashboard Preview

```powershell
cd ESP32S3Monitoring/PLTA_Monitor
node preview/server.js
```

Open `http://localhost:3000`. The preview uses simulated measurement endpoints.

## Web Assets

Readable HTML, CSS, and JavaScript are stored in `webpage.h`, `style.h`, and `app.h`.

After editing an asset, regenerate compressed firmware assets:

```powershell
cd ESP32S3Monitoring/PLTA_Monitor
node preview/gen-gzip.js
```

The generated `web_assets_gz.h` file is embedded in the Arduino firmware.

## API Routes

- `GET /api/data` - latest measurements and device data
- `GET /api/status` - hardware, network, firmware, and calibration status
- `POST /api/calibrate` - recalculate INA219 zero offsets
- `POST /api/restart` - restart the ESP32-S3

## Alarm Thresholds

Defaults in `PLTA_Monitor.ino`:

- Low voltage: 1.5 V
- High current: 1000 mA
- Low power: 50 mW

RGB LED states:

- Green - normal
- Orange - warning threshold reached
- Red - sensor unavailable or measurement invalid

## Electrical Safety

- Never connect the DB-268 generator positive output directly to an ESP32 power pin or GPIO.
- Power the ESP32 independently.
- Verify the actual generator open-circuit voltage before connection.
- Confirm the INA219 shunt resistor and calibration range.
- Use a shared ground for the INA219 measurement circuit and ESP32.
- Shared ground is not galvanic isolation.
- Do not use an SS34 diode as an overvoltage regulator.

## Project Scope

This repository contains the firmware, compressed dashboard assets, preview tooling, specification, and the PLTA practicum handout.

Physical measurements and the 10-minute reliability test require hardware testing and should be documented separately.
