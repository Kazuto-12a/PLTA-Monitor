#pragma once

const char app_js[] PROGMEM = R"rawliteral(
const $ = (id) => document.getElementById(id);
let inFlight = false;
let refreshTimer = null;
let refreshInterval = 250;
let lastSuccessfulUpdate = 0;
let dataMisses = 0;
let sessionEnergyMWh = 0;
let previousDataTime = 0;
let isOffline = false;
let endpointError = "";
let statusSnapshot = {
  oled_ok: false
};
let reliabilityStartTime = 0;
let reliabilityRunning = false;
const reliabilityRecords = [];
const reliabilityTargets = [120, 240, 360, 480, 600];
let latestLiveData = null;
const calibrationRecords = [
  { condition: "Rendah", multimeterVoltageV: null, multimeterCurrentMA: null,
    inaVoltageV: null, inaCurrentMA: null, inaPowerMW: null,
    multimeterPowerMW: null, voltageErrorPercent: null,
    currentErrorPercent: null, powerErrorPercent: null, status: "Belum diuji" },
  { condition: "Sedang", multimeterVoltageV: null, multimeterCurrentMA: null,
    inaVoltageV: null, inaCurrentMA: null, inaPowerMW: null,
    multimeterPowerMW: null, voltageErrorPercent: null,
    currentErrorPercent: null, powerErrorPercent: null, status: "Belum diuji" },
  { condition: "Penuh", multimeterVoltageV: null, multimeterCurrentMA: null,
    inaVoltageV: null, inaCurrentMA: null, inaPowerMW: null,
    multimeterPowerMW: null, voltageErrorPercent: null,
    currentErrorPercent: null, powerErrorPercent: null, status: "Belum diuji" }
];

const historyLimit = 120;
const history = [];
const enabledSeries = {
  voltage: true,
  current: true,
  power: true
};

function formatUptime(milliseconds) {
  const seconds = Math.max(0, Math.floor(milliseconds / 1000));
  const hours = String(Math.floor(seconds / 3600)).padStart(2, "0");
  const minutes = String(Math.floor((seconds % 3600) / 60)).padStart(2, "0");
  const secs = String(seconds % 60).padStart(2, "0");
  return `${hours}:${minutes}:${secs}`;
}

function formatClock(totalSeconds) {
  const seconds = Math.max(0, Math.floor(totalSeconds));
  const minutes = String(Math.floor(seconds / 60)).padStart(2, "0");
  const secs = String(seconds % 60).padStart(2, "0");
  return `${minutes}:${secs}`;
}

function errorPercent(measured, reference) {
  if (!Number.isFinite(measured) || !Number.isFinite(reference) ||
      reference === 0) return null;
  return Math.abs(measured - reference) / Math.abs(reference) * 100;
}

function saveCalibration(index) {
  const voltageInput = $("calV" + index);
  const currentInput = $("calI" + index);
  const multimeterVoltageV = Number(voltageInput.value);
  const multimeterCurrentMA = Number(currentInput.value);

  if (!latestLiveData || !latestLiveData.sensor_ok) {
    window.alert("Sensor INA219 belum memiliki pembacaan valid.");
    return;
  }
  if (!Number.isFinite(multimeterVoltageV) ||
      multimeterVoltageV <= 0 ||
      !Number.isFinite(multimeterCurrentMA) ||
      multimeterCurrentMA < 0) {
    window.alert("Masukkan tegangan dan arus multimeter yang valid.");
    return;
  }

  const inaVoltageV = Number(latestLiveData.voltage_v);
  const inaCurrentMA = Number(latestLiveData.current_ma);
  const inaPowerMW = inaVoltageV * inaCurrentMA;
  const multimeterPowerMW =
    multimeterVoltageV * multimeterCurrentMA;

  const record = calibrationRecords[index];
  record.multimeterVoltageV = multimeterVoltageV;
  record.multimeterCurrentMA = multimeterCurrentMA;
  record.inaVoltageV = inaVoltageV;
  record.inaCurrentMA = inaCurrentMA;
  record.inaPowerMW = inaPowerMW;
  record.multimeterPowerMW = multimeterPowerMW;
  record.voltageErrorPercent =
    errorPercent(inaVoltageV, multimeterVoltageV);
  record.currentErrorPercent =
    errorPercent(inaCurrentMA, multimeterCurrentMA);
  record.powerErrorPercent =
    errorPercent(inaPowerMW, multimeterPowerMW);

  const passed =
    record.voltageErrorPercent !== null &&
    record.voltageErrorPercent <= 5 &&
    (record.currentErrorPercent === null ||
      record.currentErrorPercent <= 5) &&
    (record.powerErrorPercent === null ||
      record.powerErrorPercent <= 5);
  record.status = passed ? "Tercapai" : "Tidak Tercapai";

  renderCalibration();
}

function formatPercent(value) {
  return value === null ? "--" : value.toFixed(2) + "%";
}

function renderCalibration() {
  calibrationRecords.forEach((record, index) => {
    $("calIna" + index).textContent =
      record.inaVoltageV === null
        ? "-- V / -- mA"
        : `${record.inaVoltageV.toFixed(2)} V / ${record.inaCurrentMA.toFixed(1)} mA`;
    $("calErrV" + index).textContent =
      formatPercent(record.voltageErrorPercent);
    $("calErrI" + index).textContent =
      formatPercent(record.currentErrorPercent);
    $("calErrP" + index).textContent =
      formatPercent(record.powerErrorPercent);

    const button = document.querySelector(
      `.save-calibration[data-index="${index}"]`
    );
    if (button) {
      button.textContent =
        record.status === "Belum diuji"
          ? "Simpan Pengujian"
          : record.status;
      button.style.borderColor =
        record.status === "Tercapai"
          ? "#59d39a"
          : record.status === "Tidak Tercapai"
            ? "#ff7169"
            : "";
    }
  });

  const completed = calibrationRecords.filter((record) =>
    record.status !== "Belum diuji"
  );
  const passed = completed.length > 0 &&
    completed.every((record) => record.status === "Tercapai");

  if (completed.length === 0) {
    $("calibrationSummary").textContent = "Belum diuji";
    $("calibrationSummary").style.color = "#91a1b1";
  } else if (completed.length < 3) {
    $("calibrationSummary").textContent =
      `${completed.length} / 3 pengujian`;
    $("calibrationSummary").style.color = "#f2b84b";
  } else {
    $("calibrationSummary").textContent =
      passed ? "Tercapai" : "Tidak Tercapai";
    $("calibrationSummary").style.color =
      passed ? "#59d39a" : "#ff7169";
  }
}

function updateDeviceStatus(data) {
  if (data.free_heap_bytes !== undefined) {
    $("freeHeap").textContent =
      `${Math.round(Number(data.free_heap_bytes) / 1024)} KB`;
  }
  if (data.wifi_clients !== undefined) {
    $("wifiClients").textContent = String(data.wifi_clients);
  }
  if (data.rgb_led_pin !== undefined) {
    $("rgbStatus").textContent = `GPIO ${data.rgb_led_pin}`;
  }
  if (data.firmware_version) {
    $("firmwareVersion").textContent = data.firmware_version;
  }
}

function setOffline() {
  if (!isOffline) dataMisses++;
  isOffline = true;
  $("dataMisses").textContent = String(dataMisses);
  $("connectionBadge").className = "status-badge is-offline";
  $("connectionText").textContent = "Offline";
  $("wifiStatus").textContent = "Disconnected";
  $("wifiStatus").style.color = "#ff7169";
  $("lastUpdate").textContent = "Cannot reach /api/data";
  $("chartEmpty").textContent =
    endpointError || "Cannot reach /api/data";
}

window.addEventListener("load", () => {
  history.scrollRestoration = "manual";
  window.scrollTo({ top: 0, left: 0, behavior: "instant" });
});

window.addEventListener("error", (event) => {
  endpointError = event.message
    ? "Browser error: " + event.message
    : "Browser script error";
});

function operatingState(data) {
  if (!data.sensor_ok) return "Sensor Error";
  if (Number(data.voltage_v) < 1.5 ||
      Number(data.current_ma) > 1000 ||
      Number(data.power_mw) < 50) return "Warning";
  if (Number(data.current_ma) === 0 && Number(data.voltage_v) < 0.05) return "No Flow";
  if (Number(data.power_mw) <= 0) return "No Load";
  if (Number(data.power_mw) < 100) return "Low Output";
  return "Generator Active";
}

function startReliabilityTest() {
  reliabilityRecords.splice(0, reliabilityRecords.length);
  reliabilityStartTime = Date.now();
  reliabilityRunning = true;

  $("reliabilityState").textContent = "Running";
  $("reliabilityState").style.color = "#59d39a";
  $("startReliability").disabled = true;
  $("stopReliability").disabled = false;
  $("exportReliability").disabled = true;
  renderReliabilityRecords();
  updateReliabilityTimer();
}

function stopReliabilityTest() {
  reliabilityRunning = false;
  $("reliabilityState").textContent =
    reliabilityRecords.length === 5 ? "Complete" : "Stopped";
  $("reliabilityState").style.color =
    reliabilityRecords.length === 5 ? "#59d39a" : "#f2b84b";
  $("startReliability").disabled = false;
  $("stopReliability").disabled = true;
  $("exportReliability").disabled = reliabilityRecords.length === 0;
  updateReliabilityTimer();
}

function renderReliabilityRecords() {
  const tbody = $("reliabilityRows");
  if (reliabilityRecords.length === 0) {
    tbody.innerHTML =
      '<tr><td colspan="7" class="empty-row">No test records</td></tr>';
  } else {
    tbody.innerHTML = reliabilityRecords.map((record) => `
      <tr>
        <td>${record.minute} minute${record.minute === 1 ? "" : "s"}</td>
        <td>${record.voltage.toFixed(2)} V</td>
        <td>${record.current.toFixed(1)} mA</td>
        <td>${record.power.toFixed(1)} mW</td>
        <td>${record.sensor_ok ? "OK" : "ERROR"}</td>
        <td>${record.wifi_connected ? "OK" : "DISCONNECTED"}</td>
        <td>${record.oled_ok ? "OK" : "ERROR"}</td>
      </tr>
    `).join("");
  }
  $("reliabilityCount").textContent =
    `${reliabilityRecords.length} / 5`;
  $("exportReliability").disabled =
    reliabilityRecords.length === 0 || reliabilityRunning;
}

function recordReliabilitySample(data, elapsedSeconds) {
  const minute = reliabilityTargets.findIndex((seconds) =>
    elapsedSeconds >= seconds
  );
  if (minute < 0) return;

  const intervalMinute = reliabilityTargets[minute] / 60;
  if (reliabilityRecords.some((record) =>
    record.minute === intervalMinute
  )) return;

  reliabilityRecords.push({
    minute: intervalMinute,
    timestamp: new Date().toISOString(),
    elapsed_seconds: reliabilityTargets[minute],
    voltage: Number(data.voltage_v),
    current: Number(data.current_ma),
    power: Number(data.power_mw),
    bus_voltage: Number(data.bus_voltage_v),
    shunt_voltage: Number(data.shunt_voltage_mv),
    sensor_ok: Boolean(data.sensor_ok),
    wifi_connected: Boolean(data.wifi_connected),
    oled_ok: Boolean(data.oled_ok),
  });
  reliabilityRecords.sort((a, b) => a.minute - b.minute);
  renderReliabilityRecords();

  if (reliabilityRecords.length === 5) {
    stopReliabilityTest();
  }
}

function updateReliabilityTimer() {
  if (!reliabilityRunning) {
    if (reliabilityRecords.length === 0) {
      $("reliabilityElapsed").textContent = "00:00";
      $("reliabilityNext").textContent = "02:00";
    }
    return;
  }

  const elapsed = (Date.now() - reliabilityStartTime) / 1000;
  $("reliabilityElapsed").textContent = formatClock(Math.min(elapsed, 600));

  const next = reliabilityTargets.find((seconds) =>
    elapsed < seconds
  );
  $("reliabilityNext").textContent =
    next ? formatClock(next) : "Complete";
}

function updateStats() {
  if (history.length === 0) return;
  const voltages = history.map((item) => item.voltage);
  const currents = history.map((item) => item.current);
  const powers = history.map((item) => item.power);

  const range = (values) => [
    Math.min(...values),
    Math.max(...values)
  ];
  const voltage = range(voltages);
  const current = range(currents);
  const power = range(powers);
  const averagePower =
    powers.reduce((sum, value) => sum + value, 0) / powers.length;

  $("voltageRange").textContent =
    `${voltage[0].toFixed(2)} / ${voltage[1].toFixed(2)} V`;
  $("currentRange").textContent =
    `${current[0].toFixed(1)} / ${current[1].toFixed(1)} mA`;
  $("powerRange").textContent =
    `${power[0].toFixed(1)} / ${power[1].toFixed(1)} mW`;
  $("averagePower").textContent = `${averagePower.toFixed(1)} mW`;
  $("sampleCount").textContent = String(history.length);
  $("dataMisses").textContent = String(dataMisses);
}

function drawChart() {
  const canvas = $("trendChart");
  const ctx = canvas.getContext("2d");
  const rect = canvas.getBoundingClientRect();
  const width = Math.max(320, rect.width);
  const height = rect.height || 300;
  const ratio = window.devicePixelRatio || 1;

  if (canvas.width !== Math.floor(width * ratio) ||
      canvas.height !== Math.floor(height * ratio)) {
    canvas.width = Math.floor(width * ratio);
    canvas.height = Math.floor(height * ratio);
  }
  ctx.setTransform(ratio, 0, 0, ratio, 0, 0);
  ctx.clearRect(0, 0, width, height);

  const left = 8;
  const right = 8;
  const top = 8;
  const bottom = 24;
  const plotWidth = width - left - right;
  const plotHeight = height - top - bottom;

  ctx.strokeStyle = "#263445";
  ctx.lineWidth = 1;
  for (let i = 0; i <= 4; i++) {
    const y = top + (plotHeight * i) / 4;
    ctx.beginPath();
    ctx.moveTo(left, y);
    ctx.lineTo(width - right, y);
    ctx.stroke();
  }

  ctx.fillStyle = "#91a1b1";
  ctx.font = "10px Segoe UI, Arial";
  ctx.textAlign = "left";
  ctx.fillText(history.length > 0 ? "-120 s" : "--", left, height - 5);
  ctx.textAlign = "right";
  ctx.fillText("Now", width - right, height - 5);

  if (history.length < 2) return;
  $("chartEmpty").style.display = "none";

  const colors = {
    voltage: "#41c7d9",
    current: "#f2b84b",
    power: "#59d39a"
  };

  for (const name of Object.keys(enabledSeries)) {
    if (!enabledSeries[name]) continue;
    const values = history.map((item) => item[name]);
    const min = Math.min(...values);
    const max = Math.max(...values);
    const spread = max - min;

    ctx.strokeStyle = colors[name];
    ctx.lineWidth = 2;
    ctx.beginPath();
    history.forEach((item, index) => {
      const x = history.length === 1
        ? left
        : left + (plotWidth * index) / (history.length - 1);
      const normalized =
        spread > 0 ? (item[name] - min) / spread : 0.5;
      const y = top + plotHeight * (1 - normalized);
      if (index === 0) ctx.moveTo(x, y);
      else ctx.lineTo(x, y);
    });
    ctx.stroke();
  }
}

async function refreshStatus() {
  try {
    const response = await fetch("/api/status", { cache: "no-store" });
    if (!response.ok) throw new Error("HTTP " + response.status);
    const data = await response.json();
    updateDeviceStatus(data);
    statusSnapshot = data;

    $("inaStatus").textContent = data.ina219_ok ? "Online" : "Error";
    $("inaStatus").style.color = data.ina219_ok ? "#59d39a" : "#ff7169";
    $("oledStatus").textContent = data.oled_ok ? "Online" : "Unavailable";
    $("oledStatus").style.color = data.oled_ok ? "#59d39a" : "#f2b84b";
    $("apStatus").textContent = data.wifi_connected ? "Active" : "Offline";
    $("apAddress").textContent = data.ip_address || "--";
    $("detailCurrentOffset").textContent =
      Number(data.zero_current_ma).toFixed(3) + " mA";
    $("freeHeap").textContent =
      data.free_heap_bytes
        ? `${Math.round(Number(data.free_heap_bytes) / 1024)} KB`
        : "-- KB";
    $("wifiClients").textContent =
      data.wifi_clients !== undefined ? String(data.wifi_clients) : "--";
    $("rgbStatus").textContent =
      data.rgb_led_pin !== undefined ? `GPIO ${data.rgb_led_pin}` : "GPIO 48";
    $("firmwareVersion").textContent =
      data.firmware_version || "--";
  } catch (_) {
    $("inaStatus").textContent = "Unavailable";
    $("oledStatus").textContent = "Unknown";
    $("apStatus").textContent = "Cannot verify";
  }
}

async function refreshData() {
  if (inFlight) return;
  inFlight = true;

  try {
    const response = await fetch("/api/data", { cache: "no-store" });
    if (!response.ok) throw new Error("HTTP " + response.status);
    const data = await response.json();
    isOffline = false;
    updateDeviceStatus(data);
    latestLiveData = data;
    const now = Date.now();
    const elapsedHours =
      previousDataTime > 0
        ? Math.max(0, (now - previousDataTime) / 3600000)
        : 0;

    const voltage = Number(data.voltage_v);
    const current = Number(data.current_ma);
    const power = Number(data.power_mw);
    sessionEnergyMWh += power * elapsedHours;
    previousDataTime = now;

    $("voltage").textContent =
      data.sensor_ok ? voltage.toFixed(2) : "--.-";
    $("current").textContent =
      data.sensor_ok ? current.toFixed(1) : "---.-";
    $("power").textContent =
      data.sensor_ok ? power.toFixed(1) : "----.-";
    $("energy").textContent = sessionEnergyMWh.toFixed(2);
    $("busVoltage").textContent =
      data.sensor_ok ? Number(data.bus_voltage_v).toFixed(2) + " V" : "--.- V";
    $("shuntVoltage").textContent =
      data.sensor_ok ? Number(data.shunt_voltage_mv).toFixed(2) + " mV" : "--.-- mV";
    $("uptime").textContent = formatUptime(data.uptime_ms);
    $("detailBus").textContent =
      data.sensor_ok ? Number(data.bus_voltage_v).toFixed(2) + " V" : "-- V";
    $("detailShunt").textContent =
      data.sensor_ok ? Number(data.shunt_voltage_mv).toFixed(2) + " mV" : "-- mV";

    $("sensorStatus").textContent = data.sensor_ok ? "Online" : "Error";
    $("sensorStatus").style.color =
      data.sensor_ok ? "#59d39a" : "#ff7169";
    $("wifiStatus").textContent =
      data.wifi_connected ? "Connected" : "Disconnected";
    $("wifiStatus").style.color =
      data.wifi_connected ? "#59d39a" : "#ff7169";
    $("operatingState").textContent = operatingState(data);

    $("connectionBadge").className =
      "status-badge " + (data.wifi_connected ? "is-online" : "is-offline");
    $("connectionText").textContent =
      data.wifi_connected ? "Live" : "Device offline";
    $("lastUpdate").textContent =
      "Updated " + new Date().toLocaleTimeString();

    if (data.sensor_ok) {
      history.push({
        time: new Date(),
        voltage,
        current,
        power
      });
      if (history.length > historyLimit) history.shift();
    }

    if (reliabilityRunning) {
      const elapsedSeconds =
        (Date.now() - reliabilityStartTime) / 1000;
      recordReliabilitySample(
        {
          ...data,
          oled_ok: statusSnapshot.oled_ok === true
        },
        elapsedSeconds
      );
      updateReliabilityTimer();
    }

    lastSuccessfulUpdate = now;
    updateStats();
    drawChart();
  } catch (error) {
    endpointError =
      error && error.message ? error.message : "Request failed";
    setOffline();
  } finally {
    inFlight = false;
  }
}

function startRefresh() {
  if (refreshTimer) clearInterval(refreshTimer);
  refreshTimer = setInterval(refreshData, refreshInterval);
}

function exportCsv() {
  if (history.length === 0) return;
  const rows = [
    ["timestamp", "voltage_v", "current_ma", "power_mw"]
  ];
  history.forEach((item) => {
    rows.push([
      item.time.toISOString(),
      item.voltage.toFixed(3),
      item.current.toFixed(3),
      item.power.toFixed(3)
    ]);
  });
  const csv = rows.map((row) => row.join(",")).join("\n");
  const url = URL.createObjectURL(
    new Blob([csv], { type: "text/csv;charset=utf-8" })
  );
  const link = document.createElement("a");
  link.href = url;
  link.download = "plta-db268-session.csv";
  link.click();
  URL.revokeObjectURL(url);
}

function exportChart() {
  const canvas = $("trendChart");
  if (canvas.width === 0 || canvas.height === 0) return;
  canvas.toBlob((blob) => {
    if (!blob) return;
    const url = URL.createObjectURL(blob);
    const link = document.createElement("a");
    link.href = url;
    link.download = "plta-db268-chart.png";
    link.click();
    URL.revokeObjectURL(url);
  }, "image/png");
}

function exportReliabilityCsv() {
  if (reliabilityRecords.length === 0) return;

  const rows = [
    [
      "interval_minute",
      "timestamp",
      "elapsed_seconds",
      "voltage_v",
      "current_ma",
      "power_mw",
      "bus_voltage_v",
      "shunt_voltage_mv",
      "sensor_status",
      "wifi_status",
      "oled_status"
    ]
  ];

  reliabilityRecords.forEach((record) => {
    rows.push([
      record.minute,
      record.timestamp,
      record.elapsed_seconds,
      record.voltage.toFixed(3),
      record.current.toFixed(3),
      record.power.toFixed(3),
      record.bus_voltage.toFixed(3),
      record.shunt_voltage.toFixed(3),
      record.sensor_ok ? "OK" : "ERROR",
      record.wifi_connected ? "CONNECTED" : "DISCONNECTED",
      record.oled_ok ? "OK" : "ERROR"
    ]);
  });

  const csv = rows.map((row) =>
    row.map((value) => `"${String(value).replaceAll('"', '""')}"`).join(",")
  ).join("\n");

  const url = URL.createObjectURL(
    new Blob([csv], { type: "text/csv;charset=utf-8" })
  );
  const link = document.createElement("a");
  link.href = url;
  link.download = "plta-db268-10-minute-reliability.csv";
  link.click();
  URL.revokeObjectURL(url);
}

function exportCalibrationCsv() {
  const rows = [
    [
      "no",
      "bukaan_katup",
      "v_multimeter",
      "v_ina219",
      "error_v_percent",
      "i_multimeter_ma",
      "i_ina219_ma",
      "error_i_percent",
      "p_multimeter_mw",
      "p_ina219_mw",
      "error_p_percent",
      "status"
    ]
  ];

  calibrationRecords.forEach((record, index) => {
    rows.push([
      index + 1,
      record.condition,
      record.multimeterVoltageV === null ? "" : record.multimeterVoltageV.toFixed(3),
      record.inaVoltageV === null ? "" : record.inaVoltageV.toFixed(3),
      record.voltageErrorPercent === null ? "" : record.voltageErrorPercent.toFixed(2),
      record.multimeterCurrentMA === null ? "" : record.multimeterCurrentMA.toFixed(3),
      record.inaCurrentMA === null ? "" : record.inaCurrentMA.toFixed(3),
      record.currentErrorPercent === null ? "" : record.currentErrorPercent.toFixed(2),
      record.multimeterPowerMW === null ? "" : record.multimeterPowerMW.toFixed(3),
      record.inaPowerMW === null ? "" : record.inaPowerMW.toFixed(3),
      record.powerErrorPercent === null ? "" : record.powerErrorPercent.toFixed(2),
      record.status
    ]);
  });

  const csv = rows.map((row) =>
    row.map((value) => `"${String(value).replaceAll('"', '""')}"`).join(",")
  ).join("\n");
  const url = URL.createObjectURL(
    new Blob([csv], { type: "text/csv;charset=utf-8" })
  );
  const link = document.createElement("a");
  link.href = url;
  link.download = "plta-db268-ina219-calibration.csv";
  link.click();
  URL.revokeObjectURL(url);
}

function clearCalibration() {
  calibrationRecords.forEach((record, index) => {
    record.multimeterVoltageV = null;
    record.multimeterCurrentMA = null;
    record.inaVoltageV = null;
    record.inaCurrentMA = null;
    record.inaPowerMW = null;
    record.multimeterPowerMW = null;
    record.voltageErrorPercent = null;
    record.currentErrorPercent = null;
    record.powerErrorPercent = null;
    record.status = "Belum diuji";
    $("calV" + index).value = "";
    $("calI" + index).value = "";
  });
  renderCalibration();
}

async function calibrateZero() {
  const button = $("calibrateButton");
  button.disabled = true;
  button.textContent = "Calibrating...";
  try {
    const response = await fetch("/api/calibrate", {
      method: "POST",
      cache: "no-store"
    });
    const data = await response.json();
    button.textContent =
      data.calibrated ? "Calibrated" : "Calibration Failed";
  } catch (_) {
    button.textContent = "Calibration Failed";
  }
  setTimeout(() => {
    button.disabled = false;
    button.textContent = "Calibrate Zero";
  }, 2500);
}

async function restartEsp32() {
  if (!window.confirm(
    "Restart the ESP32? Live measurements will pause briefly."
  )) return;

  const button = $("restartButton");
  button.disabled = true;
  button.textContent = "Restarting...";
  try {
    const response = await fetch("/api/restart", {
      method: "POST",
      cache: "no-store"
    });
    if (!response.ok) throw new Error("Restart request failed");
    await response.json();
  } catch (error) {
    button.disabled = false;
    button.textContent = "Restart Failed";
    setTimeout(() => {
      button.textContent = "Restart ESP32";
    }, 2500);
    return;
  }
  setTimeout(() => window.location.reload(), 1800);
}

$("refreshRate").addEventListener("change", (event) => {
  refreshInterval = Number(event.target.value);
  const selected = event.target.options[event.target.selectedIndex].text;
  const footer = document.querySelector("footer span:last-child");
  if (footer) footer.textContent = "Refresh: " + selected;
  startRefresh();
});
$("exportCsv").addEventListener("click", exportCsv);
$("exportChart").addEventListener("click", exportChart);
$("calibrateButton").addEventListener("click", calibrateZero);
$("restartButton").addEventListener("click", restartEsp32);
$("startReliability").addEventListener("click", startReliabilityTest);
$("stopReliability").addEventListener("click", stopReliabilityTest);
$("exportReliability").addEventListener("click", exportReliabilityCsv);
$("clearCalibration").addEventListener("click", clearCalibration);
$("exportCalibration").addEventListener("click", exportCalibrationCsv);
document.querySelectorAll(".save-calibration").forEach((button) => {
  button.addEventListener("click", () => {
    saveCalibration(Number(button.dataset.index));
  });
});
$("clearHistory").addEventListener("click", () => {
  history.splice(0, history.length);
  sessionEnergyMWh = 0;
  previousDataTime = 0;
  dataMisses = 0;
  $("energy").textContent = "0.00";
  $("sampleCount").textContent = "0";
  $("dataMisses").textContent = "0";
  $("chartEmpty").style.display = "grid";
  drawChart();
});
document.querySelectorAll(".legend-item").forEach((button) => {
  button.addEventListener("click", () => {
    const name = button.dataset.series;
    enabledSeries[name] = !enabledSeries[name];
    button.classList.toggle("active", enabledSeries[name]);
    drawChart();
  });
});
window.addEventListener("resize", drawChart);

setInterval(() => {
  if (lastSuccessfulUpdate > 0 &&
      Date.now() - lastSuccessfulUpdate > refreshInterval * 3) {
    setOffline();
  }
  if (reliabilityRunning) updateReliabilityTimer();
}, 1000);

refreshStatus();
setInterval(refreshStatus, 5000);
renderCalibration();
refreshData();
startRefresh();
)rawliteral";
