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
            <option value="1000" selected>1 s</option>
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

    <footer>
      <span id="lastUpdate">Waiting for data</span>
      <span>Refresh: 1 s</span>
    </footer>
  </main>
  <script src="/app.js?v=20261121" defer></script>
</body>
</html>
)rawliteral";
