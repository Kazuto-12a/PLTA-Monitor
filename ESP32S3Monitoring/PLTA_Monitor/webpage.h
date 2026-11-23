#pragma once

const char index_html[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta name="theme-color" content="#0b1016">
  <title>PLTA DB-268 Monitoring</title>
  <link rel="stylesheet" href="/style.css?v=20261121">
</head>
<body>
  <main class="shell">
    <header class="topbar">
      <div>
        <p class="eyebrow">HYDRO GENERATOR</p>
        <h1>PLTA DB-268</h1>
      </div>
      <div class="top-actions">
        <label class="refresh-select">Refresh
          <select id="refreshRate">
            <option value="250" selected>250 ms</option>
            <option value="500">500 ms</option>
            <option value="1000">1 s</option>
            <option value="2000">2 s</option>
            <option value="5000">5 s</option>
          </select>
        </label>
        <button id="exportCsv" type="button" class="action-button">Export CSV</button>
        <button id="exportChart" type="button" class="action-button">Export Chart</button>
        <div id="connectionBadge" class="status-badge is-loading">
          <span class="status-dot"></span>
          <span id="connectionText">Connecting</span>
        </div>
      </div>
    </header>

    <section class="status-grid" aria-label="System status">
      <article class="status-card">
        <span class="status-label">Sensor</span>
        <strong id="sensorStatus">Checking</strong>
      </article>
      <article class="status-card">
        <span class="status-label">Wi-Fi</span>
        <strong id="wifiStatus">Checking</strong>
      </article>
      <article class="status-card">
        <span class="status-label">Uptime</span>
        <strong id="uptime">--:--:--</strong>
      </article>
    </section>

    <section class="metrics-grid" aria-label="Generator measurements">
      <article class="metric-card voltage">
        <div class="metric-head">
          <span>Generator Voltage</span>
          <span class="metric-mark">V</span>
        </div>
        <div class="metric-value"><strong id="voltage">--.-</strong><span>V</span></div>
        <div class="metric-meta">Bus <b id="busVoltage">--.- V</b></div>
      </article>

      <article class="metric-card current">
        <div class="metric-head">
          <span>Generator Current</span>
          <span class="metric-mark">A</span>
        </div>
        <div class="metric-value"><strong id="current">---.-</strong><span>mA</span></div>
        <div class="metric-meta">Shunt <b id="shuntVoltage">--.-- mV</b></div>
      </article>

      <article class="metric-card power">
        <div class="metric-head">
          <span>Generator Power</span>
          <span class="metric-mark">P</span>
        </div>
        <div class="metric-value"><strong id="power">----.-</strong><span>mW</span></div>
        <div class="metric-meta">Live reading</div>
      </article>

      <article class="metric-card energy">
        <div class="metric-head">
          <span>Session Energy</span>
          <span class="metric-mark">E</span>
        </div>
        <div class="metric-value"><strong id="energy">0.00</strong><span>mWh</span></div>
        <div class="metric-meta">Browser-estimated</div>
      </article>
    </section>

    <section class="content-grid">
      <article class="panel chart-panel">
        <div class="panel-head">
          <div>
            <p class="panel-kicker">REAL-TIME HISTORY</p>
            <h2>Generator Trend</h2>
          </div>
          <div class="chart-legend" aria-label="Chart legend">
            <button type="button" class="legend-item active" data-series="voltage"><span class="legend-dot voltage-dot"></span>Voltage</button>
            <button type="button" class="legend-item active" data-series="current"><span class="legend-dot current-dot"></span>Current</button>
            <button type="button" class="legend-item active" data-series="power"><span class="legend-dot power-dot"></span>Power</button>
          </div>
        </div>
        <canvas id="trendChart" width="960" height="300" aria-label="Live voltage, current, and power chart"></canvas>
        <div id="chartEmpty" class="chart-empty">Waiting for measurements</div>
      </article>

      <aside class="panel details-panel">
        <p class="panel-kicker">SYSTEM</p>
        <h2>Sensor Details</h2>
        <dl class="detail-list">
          <div><dt>Operating state</dt><dd id="operatingState">Checking</dd></div>
          <div><dt>INA219</dt><dd id="inaStatus">Checking</dd></div>
          <div><dt>OLED</dt><dd id="oledStatus">Checking</dd></div>
          <div><dt>Wi-Fi AP</dt><dd id="apStatus">Checking</dd></div>
          <div><dt>Address</dt><dd id="apAddress">--</dd></div>
          <div><dt>Bus voltage</dt><dd id="detailBus">-- V</dd></div>
          <div><dt>Shunt voltage</dt><dd id="detailShunt">-- mV</dd></div>
          <div><dt>Zero current</dt><dd id="detailCurrentOffset">-- mA</dd></div>
          <div><dt>Free heap</dt><dd id="freeHeap">-- KB</dd></div>
          <div><dt>Wi-Fi clients</dt><dd id="wifiClients">--</dd></div>
          <div><dt>RGB LED</dt><dd id="rgbStatus">GPIO 48</dd></div>
          <div><dt>Firmware</dt><dd id="firmwareVersion">--</dd></div>
        </dl>
        <div class="device-actions">
          <button id="calibrateButton" type="button" class="action-button">Calibrate Zero</button>
          <button id="restartButton" type="button" class="action-button restart-button">Restart ESP32</button>
        </div>
      </aside>
    </section>

    <section class="panel stats-panel">
      <div class="panel-head">
        <div>
          <p class="panel-kicker">CURRENT SESSION</p>
          <h2>Statistics</h2>
        </div>
        <button id="clearHistory" type="button" class="action-button secondary">Clear History</button>
      </div>
      <div class="stats-grid">
        <article><span>Voltage range</span><strong id="voltageRange">-- / -- V</strong></article>
        <article><span>Current range</span><strong id="currentRange">-- / -- mA</strong></article>
        <article><span>Power range</span><strong id="powerRange">-- / -- mW</strong></article>
        <article><span>Average power</span><strong id="averagePower">-- mW</strong></article>
        <article><span>Samples</span><strong id="sampleCount">0</strong></article>
        <article><span>Data misses</span><strong id="dataMisses">0</strong></article>
      </div>
    </section>

    <section class="panel reliability-panel">
      <div class="panel-head">
        <div>
          <p class="panel-kicker">TABLE TR.2 - ITEM 7</p>
          <h2>Reliability Test 10 Minutes</h2>
        </div>
        <strong id="reliabilityState" class="reliability-state">Ready</strong>
      </div>
      <div class="reliability-controls">
        <button id="startReliability" type="button" class="action-button">Start 10-Minute Test</button>
        <button id="stopReliability" type="button" class="action-button secondary" disabled>Stop Test</button>
        <button id="exportReliability" type="button" class="action-button" disabled>Export CSV</button>
      </div>
      <div class="reliability-progress">
        <div>
          <span>Elapsed</span>
          <strong id="reliabilityElapsed">00:00</strong>
        </div>
        <div>
          <span>Next record</span>
          <strong id="reliabilityNext">02:00</strong>
        </div>
        <div>
          <span>Recorded</span>
          <strong id="reliabilityCount">0 / 5</strong>
        </div>
      </div>
      <div class="reliability-table-wrap">
        <table class="reliability-table">
          <thead>
            <tr>
              <th>Interval</th>
              <th>Voltage</th>
              <th>Current</th>
              <th>Power</th>
              <th>Sensor</th>
              <th>Wi-Fi</th>
              <th>OLED</th>
            </tr>
          </thead>
          <tbody id="reliabilityRows">
            <tr><td colspan="7" class="empty-row">No test records</td></tr>
          </tbody>
        </table>
      </div>
    </section>

    <section class="panel calibration-panel">
      <div class="panel-head">
        <div>
          <p class="panel-kicker">TABLE TR.2 - ITEM 4</p>
          <h2>Kalibrasi Komparatif INA219</h2>
        </div>
        <strong id="calibrationSummary" class="reliability-state">Belum diuji</strong>
      </div>
      <p class="calibration-note">
        Catat tegangan dan arus dari multimeter untuk setiap bukaan katup, kemudian simpan hasil INA219 saat itu.
      </p>
      <div class="calibration-grid">
        <article class="calibration-row" data-index="0">
          <h3>1. Bukaan Rendah</h3>
          <label>Tegangan multimeter (V)<input id="calV0" type="number" step="0.001" inputmode="decimal"></label>
          <label>Arus multimeter (mA)<input id="calI0" type="number" step="0.1" inputmode="decimal"></label>
          <div class="calibration-values"><span>INA: <b id="calIna0">-- V / -- mA</b></span><span>Error V: <b id="calErrV0">--</b></span></div>
          <div class="calibration-values"><span>Error I: <b id="calErrI0">--</b></span><span>Error P: <b id="calErrP0">--</b></span></div>
          <button type="button" class="action-button save-calibration" data-index="0">Simpan Pengujian</button>
        </article>
        <article class="calibration-row" data-index="1">
          <h3>2. Bukaan Sedang</h3>
          <label>Tegangan multimeter (V)<input id="calV1" type="number" step="0.001" inputmode="decimal"></label>
          <label>Arus multimeter (mA)<input id="calI1" type="number" step="0.1" inputmode="decimal"></label>
          <div class="calibration-values"><span>INA: <b id="calIna1">-- V / -- mA</b></span><span>Error V: <b id="calErrV1">--</b></span></div>
          <div class="calibration-values"><span>Error I: <b id="calErrI1">--</b></span><span>Error P: <b id="calErrP1">--</b></span></div>
          <button type="button" class="action-button save-calibration" data-index="1">Simpan Pengujian</button>
        </article>
        <article class="calibration-row" data-index="2">
          <h3>3. Bukaan Penuh</h3>
          <label>Tegangan multimeter (V)<input id="calV2" type="number" step="0.001" inputmode="decimal"></label>
          <label>Arus multimeter (mA)<input id="calI2" type="number" step="0.1" inputmode="decimal"></label>
          <div class="calibration-values"><span>INA: <b id="calIna2">-- V / -- mA</b></span><span>Error V: <b id="calErrV2">--</b></span></div>
          <div class="calibration-values"><span>Error I: <b id="calErrI2">--</b></span><span>Error P: <b id="calErrP2">--</b></span></div>
          <button type="button" class="action-button save-calibration" data-index="2">Simpan Pengujian</button>
        </article>
      </div>
      <div class="calibration-actions">
        <button id="clearCalibration" type="button" class="action-button secondary">Hapus Hasil</button>
        <button id="exportCalibration" type="button" class="action-button">Export CSV Kalibrasi</button>
      </div>
    </section>

    <footer>
      <span id="lastUpdate">Waiting for data</span>
      <span>Refresh: 250 ms</span>
    </footer>
  </main>
  <script src="/app.js?v=20261121" defer></script>
</body>
</html>
)rawliteral";
