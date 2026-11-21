# PLTA DB-268 --- ESP32-S3 OLED + INA219 Firmware Specification

> **Purpose:** Use this document as the implementation prompt for an AI
> coding assistant. Generate a complete, compilable ESP32-S3 Arduino
> firmware for the PLTA monitoring prototype.
>
> **Implementation scope:** Read generator voltage/current/power from
> INA219, display the values on a 1.5-inch 128×128 I2C OLED, and expose
> the same measurements through an asynchronous Wi-Fi web server. Do not
> add generator-control outputs or connect generator power to the ESP32
> power input.

------------------------------------------------------------------------

## 1. Project overview

Build firmware for a small hydroelectric generator monitoring prototype.

### Main functions

1.  Initialize the ESP32-S3 and its I2C bus.
2.  Detect and initialize the INA219 current/voltage sensor.
3.  Detect and initialize the 1.5-inch 128×128 I2C OLED.
4.  Read generator-side voltage and current from INA219.
5.  Calculate generator-side electrical power.
6.  Display voltage, current, power, and device status on the OLED.
7.  Connect to Wi-Fi.
8.  Start an asynchronous HTTP server.
9.  Serve a browser dashboard and a JSON data endpoint.
10. Use FFat for a dashboard file if it exists; provide a fallback page
    if it does not.
11. Keep the sensor/display update loop responsive. Do not use long
    blocking delays.

The first firmware version is a **monitoring system only**. It must not
control the water valve, generator, boost converter, relay, or load.

------------------------------------------------------------------------

## 2. Hardware

### 2.1 Controller

-   Board: ESP32-S3 DevKit.
-   Framework: Arduino for ESP32.
-   I2C pins: configurable constants near the top of the source file.
-   Do not assume every ESP32-S3 DevKit has the same pin mapping.
-   GPIO8 (SDA) and GPIO9 (SCL) may be used as provisional defaults
    **only if confirmed available on the exact board**. Make both pins
    easy to change.

Example configuration:

``` cpp
#define I2C_SDA_PIN 8   // Verify against the exact ESP32-S3 board pinout
#define I2C_SCL_PIN 9   // Verify against the exact ESP32-S3 board pinout
```

### 2.2 OLED

Use the OLED described by the supplied product specification:

-   Screen size: 1.5 inch.
-   Resolution: 128 × 128 pixels.
-   Interface: I2C / IIC.
-   Connector: 4 pins.
-   Pins: VDD, GND, SCL, SDA.
-   Module supply range printed by seller: 3.3--5 V.
-   Controller/driver printed by seller: `GSH1107G-02` (verify
    compatibility with the SH1107 driver).
-   Use **3.3 V** for OLED VDD so its I2C pull-ups, if fitted to the
    module, do not pull ESP32-S3 signals to 5 V.
-   No OLED reset pin is available on the 4-pin connector.

Use the user's working OLED libraries: **Adafruit_GFX** and
**Adafruit_SH110X**.

```cpp
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
```

For a 128×128 SH1107-compatible panel, use the `Adafruit_SH1107` class
from Adafruit_SH110X. A typical hardware-I2C declaration is:

```cpp
#define OLED_WIDTH  128
#define OLED_HEIGHT 128
#define OLED_RESET  -1
#define OLED_I2C_ADDRESS 0x3C  // Confirm by I2C scan; may be 0x3D

Adafruit_SH1107 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);
```

Initialize it after `Wire.begin(...)`:

```cpp
if (!display.begin(OLED_I2C_ADDRESS, true)) {
    // Report OLED initialization failure.
}
```

Verify the constructor and `begin()` signature against the installed
Adafruit_SH110X version. The supplied module is marked `GSH1107G-02`;
confirm compatibility with the library's SH1107 implementation. Do not
substitute an SSD1306 class merely because it is a common OLED controller.

### 2.3 INA219

-   Module: INA219 I2C GY-219 bidirectional DC current sensor.
-   Typical I2C address: `0x40`; scan the bus instead of assuming it.
-   Power the INA219 logic VCC from ESP32-S3 3.3 V.
-   Connect INA219 GND to the monitoring circuit ground.
-   Use the INA219 high-side shunt terminals VIN+ and VIN− for
    generator-current measurement.
-   Check the shunt resistor marking on the actual module. A common
    marking is `R100` (0.1 ohm), but do not assume this without
    inspection.
-   Use a calibration setting compatible with the actual shunt and
    expected voltage/current range.

Recommended library: **Adafruit INA219**. For a typical 0.1-ohm GY-219
module and a generator expected to remain below 26 V and 1 A,
`setCalibration_32V_1A()` is a reasonable starting configuration.
Confirm the actual shunt and operating range before relying on the
readings. If the installed library does not provide that method, explain
the compatible alternative.

### 2.4 Generator and load measurement path

The intended measurement topology is:

``` text
DB-268 Generator (+)
        |
        v
INA219 VIN+
INA219 VIN-
        |
        v
DC-DC Step-Up IN+ (if used)
        |
        v
Test load

DB-268 Generator (-) ---------------- Step-Up IN- / load (-)
```

The INA219 is installed **before the Step-Up** so it measures the
generator-side electrical output, not the Step-Up output.

If testing without the Step-Up, connect INA219 VIN− to the positive side
of the test load instead.

The ESP32-S3 must never receive generator positive voltage at its 3V3,
5V/VIN, USB, or GPIO pins.

The INA219 I2C logic is powered by the ESP32-S3 3.3 V rail. The
generator measurement terminals are separate from the I2C logic pins.
Observe the INA219 common-mode/input limits and verify generator maximum
voltage with a multimeter before connecting it.

**Grounding note:** INA219 measurement reference and ESP32 I2C logic
require a correctly designed common reference for a non-isolated
measurement circuit. Explain exactly where generator negative, INA219
GND, and ESP32 GND are connected in the final wiring notes. Do not imply
that the circuit is galvanically isolated.

### 2.5 ESP32-S3 power

-   Monitoring power source: 3 × AA battery holder (nominal 4.5 V).
-   Use a suitable, regulated supply for the exact ESP32-S3 DevKit.
-   If feeding the board through its 5V/VIN pin, use a dedicated
    regulated 5 V converter with adequate current capacity. Do not
    assume a 4.5 V battery pack is a stable 5 V input.
-   Do not use the DB-268 generator or its Step-Up output to power the
    ESP32.
-   OLED and INA219 logic should use the ESP32 3.3 V rail.
-   Do not add an SS34 as an overvoltage clamp. SS34 is a Schottky diode
    and does not regulate generator voltage to 3.3 V or 5 V.

------------------------------------------------------------------------

## 3. Required libraries

### 3.1 Existing project libraries --- keep these

The following includes are already working in the user's project.
Preserve them; do not replace the web-server architecture with another
framework.

``` cpp
#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ESPAsyncWebServer.h>
#include <FFat.h>
#include <ArduinoJson.h>
```

### 3.2 Hardware support libraries

Add the libraries needed for the OLED, I2C, and INA219:

``` cpp
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_INA219.h>
```

-   `Wire.h`: ESP32 Arduino I2C support.
-   `Adafruit_GFX.h`: graphics and text drawing primitives.
-   `Adafruit_SH110X.h`: SH1107-compatible OLED driver.
-   `Adafruit_INA219.h`: INA219 sensor support.

Install **Adafruit GFX Library**, **Adafruit SH110X**, and **Adafruit
INA219** through Arduino Library Manager. Install Adafruit BusIO if the
library manager indicates it is required.

Do not remove or rename the user's six existing libraries.

------------------------------------------------------------------------

## 4. I2C wiring

OLED and INA219 share one I2C bus. They are parallel devices, not
daisy-chained devices.

  ESP32-S3                 OLED 1.5-inch   INA219
  ------------------------ --------------- --------
  3V3                      VDD             VCC
  GND                      GND             GND
  GPIO configured as SDA   SDA             SDA
  GPIO configured as SCL   SCL             SCL

Use the same SDA and SCL pins for both modules.

Recommended initial I2C clock: **100 kHz** for reliable bring-up.

Initialize the bus using configurable pins:

``` cpp
Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
Wire.setClock(100000);
```

### Expected I2C addresses

  Device      Typical address Requirement
  -------- ------------------ -------------------
  INA219               `0x40` Detect at startup
  OLED       `0x3C` or `0x3D` Detect at startup

Implement an I2C scanner or equivalent address detection at startup.
Print discovered addresses to Serial Monitor.

Do not assume that the OLED address is always `0x3C`. Configure the
Adafruit_SH110X display using the address found by the I2C scan. Check
the installed library's `begin()` signature.

If either device is not detected: - Print a clear diagnostic to
Serial. - Show an appropriate error/status message on the OLED if the
OLED itself is available. - Keep the web server running if possible. -
Do not repeatedly crash or restart the ESP32.

------------------------------------------------------------------------

## 5. INA219 measurement and calculations

The sensor is on the generator's positive output line, before the
Step-Up.

Read: - Bus voltage. - Shunt voltage. - Current. - Sensor
availability/status.

With the intended direction:

``` text
Generator (+) -> VIN+ [INA219] VIN- -> Step-Up/load (+)
```

The INA219 bus voltage is measured at VIN−. To estimate the voltage at
VIN+ (generator side), use:

``` cpp
generatorVoltage_V = busVoltage_V + (shuntVoltage_mV / 1000.0f);
```

This assumes the normal current direction from VIN+ to VIN−. Preserve
signed readings and handle reversed current direction; do not silently
convert all negative readings to positive.

Calculate generator-side power from the generator-side voltage and
measured current:

``` cpp
generatorPower_mW = generatorVoltage_V * current_mA;
```

The unit is mW because V × mA = mW.

Do not label the Step-Up output voltage as generator voltage. The INA219
is measuring the generator-side input to the Step-Up.

If using `Adafruit_INA219`, read the relevant values with its supported
methods, such as: - `getBusVoltage_V()` - `getShuntVoltage_mV()` -
`getCurrent_mA()`

Do not rely blindly on `getPower_mW()` for generator-side power if it
uses the bus-side voltage. Calculate generator-side power using the
formula above.

### Sensor validation

-   Check that `ina219.begin()` succeeds.
-   Check for NaN or implausible readings.
-   Do not report zero as a valid measurement if the sensor has failed.
-   Track `sensor_ok`.
-   Use a suitable sampling interval, initially 500 ms.
-   Optional: use a small moving average for display stability, but
    preserve raw values for debugging and do not introduce excessive
    delay.

------------------------------------------------------------------------

## 6. OLED user interface

Design a clean, readable monochrome layout for a 128×128 display.

Suggested content:

``` text
     PLTA MONITOR
--------------------
GENERATOR STATUS: OK

VOLTAGE
12.34 V

CURRENT
125.6 mA

POWER
1548 mW

I2C: OK   WIFI: OK
```

Requirements:
- Use fonts that fit the 128×128 resolution.
- Keep labels and values readable.
- Avoid drawing outside the display bounds.
- Refresh the display no slower than once per second.
- Use the Adafruit_GFX / Adafruit_SH110X drawing API.
- Clear and redraw using `display.clearDisplay()`, drawing calls such as
  `setTextSize()`, `setTextColor(SH110X_WHITE)`, `setCursor()`, and
  `print()`, then call `display.display()` to transfer the frame to OLED.
- Avoid unnecessary full-screen redraws and keep the layout stable to
  minimize flicker.
- Show `SENSOR ERROR` when INA219 is unavailable.
- Show a Wi-Fi disconnected indicator without blocking sensor updates.
- Show a sensible startup screen while hardware initializes.

Use correct voltage/current/power units: - Voltage: V - Current: mA -
Power: mW

------------------------------------------------------------------------

## 7. Wi-Fi, mDNS, and asynchronous web server

Use the existing libraries:

``` cpp
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ESPAsyncWebServer.h>
```

Requirements: - Define editable Wi-Fi SSID and password constants near
the top of the source. - Do not invent real credentials. - Use station
mode (`WIFI_STA`) by default. - Do not block the main sensor/display
loop while connecting. - Attempt reconnection without restarting the
ESP32. - Start mDNS with hostname `plta-monitor` if Wi-Fi connects. -
Target local address: `http://plta-monitor.local/`. - Also print the
assigned IP address to Serial Monitor. - Start
`AsyncWebServer server(80)`. - Keep HTTP handlers short and
non-blocking. - Do not use a synchronous `WebServer` in place of
`ESPAsyncWebServer`.

### Required routes

  Route           Method   Purpose
  --------------- -------- ----------------------------------
  `/`             GET      Dashboard HTML
  `/api/data`     GET      Latest measurement as JSON
  `/api/status`   GET      Device, sensor, and Wi-Fi status

### `/api/data` JSON example

Use ArduinoJson to serialize the current measurement snapshot.

``` json
{
  "voltage_v": 12.34,
  "current_ma": 125.6,
  "power_mw": 1548.0,
  "sensor_ok": true,
  "wifi_connected": true,
  "uptime_ms": 123456
}
```

The example values above are illustrative only. Always return actual
sensor values.

Requirements: - Use an ArduinoJson API compatible with the installed
version. Prefer an API that works with the user's installed ArduinoJson
version. - Return `Content-Type: application/json`. - Do not return
stale values as valid if the sensor has failed; include
`sensor_ok: false`. - Keep a consistent JSON schema.

### Dashboard

Create a simple responsive HTML dashboard: - Title:
`PLTA DB-268 Monitoring`. - Show voltage, current, and power. - Show
sensor and Wi-Fi connection status. - Fetch `/api/data` periodically
using browser JavaScript. - Refresh around once per second. - Display a
clear error state if the endpoint cannot be reached. - Do not require an
external CDN or internet connection; the dashboard must work on the
local network.

------------------------------------------------------------------------

## 8. FFat file handling

Use the existing FFat library:

``` cpp
#include <FFat.h>
```

Requirements: - Attempt to mount FFat during startup. - Prefer
`FFat.begin(false)` initially. - Do not automatically format FFat on
mount failure; formatting can erase stored files. - If `/index.html`
exists, serve it at `/`. - If the file is missing or FFat cannot mount,
serve a small built-in fallback dashboard. - Print FFat mount and file
status to Serial Monitor. - Explain how to upload `index.html` to FFat
if a separate dashboard file is generated.

If the implementation uses only an embedded HTML page, explain that FFat
is initialized but not required for that mode. Do not pretend FFat is
serving a file when it is not.

------------------------------------------------------------------------

## 9. Firmware architecture and timing

Use a non-blocking `millis()`-based loop.

Suggested intervals:

  Task                              Interval
  --------------------------- --------------
  INA219 measurement                  500 ms
  OLED refresh                  500--1000 ms
  Browser dashboard polling          1000 ms
  Wi-Fi reconnect check              5000 ms
  Serial diagnostic                  2000 ms

Requirements: - No long `delay()` calls in the main loop. - Do not
restart the ESP32 merely because Wi-Fi is disconnected. - Do not restart
the ESP32 merely because INA219 is not detected. - Keep sensor
acquisition independent from web requests. - Use a safe measurement
snapshot when asynchronous HTTP callbacks read data that the main loop
updates. Use an appropriate critical section, mutex, or short protected
copy for shared data. - Do not perform slow I2C operations inside HTTP
request callbacks. - Update the shared measurement snapshot in the main
loop; HTTP routes should return the latest snapshot.

Suggested high-level flow:

``` text
START
  |
  v
Initialize Serial
  |
  v
Initialize I2C
  |
  +--> Scan I2C addresses
  |
  +--> Initialize OLED
  |
  +--> Initialize INA219
  |
  +--> Mount FFat
  |
  +--> Start Wi-Fi connection attempt
  |
  +--> Start asynchronous web server
  |
  v
MAIN LOOP
  |
  +--> Every 500 ms: read INA219
  |
  +--> Calculate generator voltage/current/power
  |
  +--> Update shared measurement snapshot
  |
  +--> Every 500–1000 ms: refresh OLED
  |
  +--> Check Wi-Fi reconnection without blocking
  |
  v
Repeat
```

------------------------------------------------------------------------

## 10. Electrical safety and protection requirements

These are hardware requirements, not software substitutes:

1.  Never connect DB-268 generator positive directly to ESP32-S3 3V3,
    5V/VIN, USB, or GPIO.
2.  The INA219 measurement terminals must remain within the INA219
    module/chip limits. The INA219 IC bus common-mode range is up to 26
    V; verify the actual generator's maximum open-circuit voltage with a
    multimeter before connection.
3.  Do not assume a seller's nominal 12 V rating is the maximum
    open-circuit voltage.
4.  Power INA219 logic and OLED from ESP32 3.3 V for compatible I2C
    logic levels.
5.  Do not power OLED I2C pull-ups from 5 V when connected directly to
    ESP32-S3 GPIO.
6.  If the ESP32 is OFF while the generator is still producing voltage,
    verify the INA219 module's powered-off input behavior against its
    datasheet. If this cannot be verified, provide a suitable hardware
    disconnect for the INA219 measurement input when its logic supply is
    OFF.
7.  SS34 is not an overvoltage regulator or a 3.3 V clamp. Do not claim
    that SS34 alone protects ESP32 from generator overvoltage.
8.  I2C isolation or a powered-off-protected bus switch may be required
    if another powered device can drive SDA/SCL while the ESP32 is OFF.
9.  A shared ground is not galvanic isolation. State the grounding
    arrangement clearly.
10. Do not select a TVS diode or fuse rating without measuring the
    generator's actual maximum voltage/current and checking the ratings
    of the connected modules.

------------------------------------------------------------------------

## 11. Required code deliverables

Generate the following:

### A. Complete firmware

Provide a complete Arduino `.ino` file that: - Compiles for the ESP32-S3
Arduino framework. - Preserves all six existing project libraries. -
Adds the required I2C/OLED/INA219 support. - Implements all required
sensor, OLED, Wi-Fi, mDNS, AsyncWebServer, FFat, and JSON functions. -
Has clearly marked editable constants for GPIO pins and Wi-Fi
credentials. - Contains useful comments for hardware connections and
configuration.

Do not provide pseudocode in place of the working firmware.

OLED implementation requirement:
- Use `Adafruit_SH1107` from `Adafruit_SH110X.h` with `Adafruit_GFX.h`.
- Use the `Adafruit_SH1107` class from `Adafruit_SH110X.h`.
- Use the 128×128 dimensions and the scanned/configured I2C address.

### B. Dashboard file

Provide a complete `index.html` if using FFat, including its CSS and
JavaScript. It must work offline on the local network and fetch live
data from `/api/data`.

### C. Wiring table

Provide a final pin-to-pin wiring table for: - ESP32-S3 ↔ OLED. -
ESP32-S3 ↔ INA219 I2C. - DB-268 ↔ INA219 VIN+/VIN−. - Generator negative
/ circuit ground. - Battery / regulated ESP32 supply.

Clearly mark any GPIO assignments that must be verified against the
exact ESP32-S3 board.

### D. Library and build instructions

List: - Existing libraries retained. - Additional libraries to
install. - Board selection and relevant Arduino IDE settings. - How to
upload the firmware. - How to upload the FFat dashboard file, if
applicable.

### E. Test procedure

Provide a staged test: 1. I2C scanner only. 2. OLED test only. 3. INA219
detection and serial readings. 4. OLED + INA219 together. 5. Wi-Fi and
mDNS. 6. `/api/data` JSON endpoint. 7. Browser dashboard. 8. Generator
measurement with a multimeter comparison.

Include expected diagnostic messages and common troubleshooting steps.

------------------------------------------------------------------------

## 12. Important constraints for the coding assistant

-   Do not silently change the user's hardware.
-   Do not assume the exact ESP32-S3 DevKit pinout.
-   Do not assume the OLED is SSD1306.
-   Do not assume OLED address `0x3C` without scanning.
-   Do not assume INA219 shunt value without checking the board.
-   Do not power ESP32 from DB-268.
-   Do not connect generator output to an ESP32 GPIO or power pin.
-   Do not use the Step-Up output as the generator-side measurement.
-   Do not remove the user's existing Wi-Fi/web/FFat/JSON libraries.
-   Do not claim the firmware has been compiled or tested unless it
    actually has.
-   If a required library API differs by version, identify the version
    assumption and provide the compatible implementation.
-   Before finalizing, review the generated code for compile errors,
    blocking calls, invalid pin assumptions, incorrect INA219 power
    calculations, and unsafe electrical connections.

**Final goal:** A stable ESP32-S3 PLTA monitoring firmware that reads
generator-side measurements through INA219, displays them on the
1.5-inch SH1107-compatible 128×128 I2C OLED, and publishes the same
measurements through a local asynchronous web dashboard.
