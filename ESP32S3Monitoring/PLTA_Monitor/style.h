#pragma once

const char style_css[] PROGMEM = R"rawliteral(
:root {
  color-scheme: dark;
  --bg: #0b1016;
  --panel: #131b24;
  --panel-2: #19232f;
  --line: #263445;
  --text: #edf5f7;
  --muted: #91a1b1;
  --cyan: #41c7d9;
  --green: #59d39a;
  --amber: #f2b84b;
  --red: #ff7169;
}

* { box-sizing: border-box; }

body {
  margin: 0;
  min-height: 100vh;
  overflow-x: hidden;
  background:
    radial-gradient(circle at 50% -20%, rgba(65, 199, 217, .12), transparent 34rem),
    var(--bg);
  color: var(--text);
  font-family: Inter, "Segoe UI", Arial, sans-serif;
}

.shell {
  width: min(1120px, calc(100% - 32px));
  max-width: 100%;
  margin: 0 auto;
  padding: 36px 0 24px;
}

.topbar, footer, .status-card, .metric-head, .metric-meta,
.top-actions, .panel-head {
  display: flex;
  align-items: center;
  justify-content: space-between;
}

.eyebrow {
  margin: 0 0 6px;
  color: var(--cyan);
  font-size: 12px;
  font-weight: 800;
  letter-spacing: .18em;
}

h1 { margin: 0; font-size: clamp(24px, 5vw, 38px); }

.status-badge {
  display: flex;
  gap: 9px;
  align-items: center;
  border: 1px solid var(--line);
  border-radius: 999px;
  background: rgba(19, 27, 36, .92);
  padding: 9px 13px;
  font-size: 13px;
  font-weight: 750;
}

.status-dot {
  width: 8px;
  height: 8px;
  border-radius: 50%;
  background: var(--amber);
  box-shadow: 0 0 12px currentColor;
}

.status-badge.is-online .status-dot { background: var(--green); color: var(--green); }
.status-badge.is-offline .status-dot { background: var(--red); color: var(--red); }
.top-actions { gap: 8px; flex-wrap: wrap; }

.action-button, .refresh-select {
  border: 1px solid var(--line);
  border-radius: 8px;
  background: var(--panel-2);
  color: var(--text);
  min-height: 38px;
  font: inherit;
  font-size: 12px;
  font-weight: 750;
}
.action-button { padding: 0 12px; cursor: pointer; }
.action-button:hover { border-color: var(--cyan); }
.action-button.secondary { background: transparent; }
.refresh-select { display: flex; align-items: center; gap: 6px; padding: 0 8px 0 10px; }
.refresh-select select { border: 0; background: transparent; color: var(--text); font: inherit; }
.refresh-select option { background: var(--panel); }

.status-grid {
  display: grid;
  grid-template-columns: repeat(3, 1fr);
  gap: 12px;
  margin: 28px 0 12px;
}

.status-card, .metric-card {
  border: 1px solid var(--line);
  background: linear-gradient(145deg, rgba(25,35,47,.96), rgba(19,27,36,.96));
  border-radius: 14px;
}

.status-card { padding: 14px 16px; }
.status-card strong { color: var(--green); }
.status-label { color: var(--muted); font-size: 13px; }

.metrics-grid {
  display: grid;
  grid-template-columns: repeat(4, 1fr);
  gap: 12px;
}

.metric-card { min-height: 220px; padding: 18px; }
.metric-head { color: var(--muted); font-size: 14px; font-weight: 700; }
.metric-mark {
  display: grid;
  width: 30px;
  height: 30px;
  place-items: center;
  border-radius: 8px;
  color: var(--cyan);
  background: rgba(65,199,217,.10);
  font-weight: 900;
}

.metric-value {
  display: flex;
  align-items: baseline;
  gap: 8px;
  margin: 46px 0 40px;
}
.metric-value strong {
  font-size: clamp(38px, 7vw, 64px);
  line-height: 1;
  letter-spacing: -.04em;
  font-variant-numeric: tabular-nums;
}
.metric-value span { color: var(--muted); font-size: 16px; }
.metric-meta { color: var(--muted); border-top: 1px solid var(--line); padding-top: 13px; font-size: 13px; }
.metric-meta b { color: var(--text); }

.current .metric-mark { color: var(--amber); background: rgba(242,184,75,.10); }
.power .metric-mark { color: var(--green); background: rgba(89,211,154,.10); }
.energy .metric-mark { color: #b58cff; background: rgba(181,140,255,.10); }

.content-grid {
  display: grid;
  grid-template-columns: minmax(0, 2fr) minmax(260px, 1fr);
  gap: 12px;
  margin-top: 12px;
}
.panel {
  border: 1px solid var(--line);
  border-radius: 14px;
  background: linear-gradient(145deg, rgba(25,35,47,.96), rgba(19,27,36,.96));
  padding: 18px;
}
.panel-head { align-items: flex-start; }
.panel h2 { margin: 3px 0 0; font-size: 18px; }
.panel-kicker {
  margin: 0;
  color: var(--cyan);
  font-size: 10px;
  font-weight: 850;
  letter-spacing: .14em;
}
.chart-panel { position: relative; min-height: 390px; }
#trendChart { width: 100%; height: 300px; display: block; margin-top: 16px; }
.chart-empty {
  position: absolute;
  inset: 92px 18px 18px;
  display: grid;
  place-items: center;
  color: var(--muted);
  font-size: 13px;
}
.chart-legend { display: flex; gap: 5px; flex-wrap: wrap; justify-content: flex-end; }
.legend-item {
  border: 0;
  background: transparent;
  color: var(--muted);
  font-size: 11px;
  cursor: pointer;
  opacity: .45;
}
.legend-item.active { opacity: 1; }
.legend-dot { display: inline-block; width: 7px; height: 7px; border-radius: 50%; margin-right: 4px; }
.voltage-dot { background: var(--cyan); }
.current-dot { background: var(--amber); }
.power-dot { background: var(--green); }

.detail-list { margin: 12px 0 0; }
.detail-list div {
  display: flex;
  justify-content: space-between;
  gap: 12px;
  border-top: 1px solid var(--line);
  padding: 10px 0;
  font-size: 12px;
}
.detail-list dt { color: var(--muted); }
.detail-list dd { margin: 0; text-align: right; font-weight: 750; }
.device-actions {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 8px;
  margin-top: 12px;
}
.restart-button {
  border-color: rgba(255,113,105,.55);
  color: #ff7169;
}

.stats-panel { margin-top: 12px; }
.stats-grid {
  display: grid;
  grid-template-columns: repeat(6, 1fr);
  gap: 8px;
  margin-top: 14px;
}
.stats-grid article {
  min-width: 0;
  border: 1px solid var(--line);
  border-radius: 10px;
  padding: 12px;
}
.stats-grid span { display: block; color: var(--muted); font-size: 11px; margin-bottom: 7px; }
.stats-grid strong { font-size: 13px; font-variant-numeric: tabular-nums; }
.reliability-panel { margin-top: 12px; }
.reliability-state { color: var(--green); font-size: 12px; }
.reliability-controls {
  display: flex;
  flex-wrap: wrap;
  gap: 8px;
  margin-top: 14px;
}
.reliability-progress {
  display: grid;
  grid-template-columns: repeat(3, 1fr);
  gap: 8px;
  margin-top: 12px;
}
.reliability-progress div {
  border: 1px solid var(--line);
  border-radius: 10px;
  padding: 10px;
}
.reliability-progress span {
  display: block;
  color: var(--muted);
  font-size: 11px;
  margin-bottom: 5px;
}
.reliability-progress strong { font-variant-numeric: tabular-nums; }
.reliability-table-wrap {
  overflow-x: auto;
  margin-top: 12px;
}
.reliability-table {
  width: 100%;
  min-width: 620px;
  border-collapse: collapse;
  font-size: 12px;
}
.reliability-table th,
.reliability-table td {
  border-bottom: 1px solid var(--line);
  padding: 9px 8px;
  text-align: left;
  white-space: nowrap;
}
.reliability-table th { color: var(--muted); font-weight: 700; }
.empty-row { text-align: center !important; color: var(--muted); }
.calibration-panel { margin-top: 12px; }
.calibration-note {
  color: var(--muted);
  font-size: 12px;
  line-height: 1.5;
  margin: 12px 0;
}
.calibration-grid {
  display: grid;
  grid-template-columns: repeat(3, minmax(0, 1fr));
  gap: 8px;
}
.calibration-row {
  border: 1px solid var(--line);
  border-radius: 10px;
  padding: 12px;
}
.calibration-row h3 { margin: 0 0 10px; font-size: 14px; }
.calibration-row label {
  display: block;
  color: var(--muted);
  font-size: 11px;
  margin-top: 8px;
}
.calibration-row input {
  width: 100%;
  min-height: 38px;
  margin-top: 5px;
  border: 1px solid var(--line);
  border-radius: 8px;
  background: rgba(11,16,22,.65);
  color: var(--text);
  padding: 8px;
  font: inherit;
}
.calibration-values {
  display: flex;
  justify-content: space-between;
  gap: 8px;
  color: var(--muted);
  font-size: 11px;
  margin-top: 9px;
}
.calibration-values b { color: var(--text); }
.calibration-row .action-button { width: 100%; margin-top: 12px; }
.calibration-actions {
  display: flex;
  justify-content: flex-end;
  gap: 8px;
  margin-top: 12px;
}

footer { color: var(--muted); font-size: 12px; padding: 16px 2px 0; }

@media (max-width: 760px) {
  .shell { width: min(100% - 20px, 1120px); padding-top: 18px; }
  .topbar { flex-direction: column; align-items: stretch; gap: 14px; }
  .top-actions { width: 100%; justify-content: space-between; }
  .top-actions .status-badge { width: 100%; justify-content: center; }
  .status-grid { grid-template-columns: 1fr; }
  .metrics-grid { grid-template-columns: 1fr 1fr; }
  .content-grid { grid-template-columns: 1fr; }
  .stats-grid { grid-template-columns: repeat(2, 1fr); }
  .metric-card { min-height: 180px; }
  .metric-value { margin: 32px 0; }
}

@media (max-width: 480px) {
  .metrics-grid { grid-template-columns: 1fr; }
  .chart-panel { min-height: 280px; }
  #trendChart { height: 200px; }
  .chart-legend { max-width: 150px; }
  .detail-list div {
    display: grid;
    grid-template-columns: minmax(0, 1fr) auto;
  }
  .detail-list dd { min-width: 92px; }
  .stats-grid { grid-template-columns: repeat(2, minmax(0, 1fr)); }
  .stats-grid article { padding: 10px 8px; }
  .stats-grid strong { font-size: 12px; overflow-wrap: anywhere; }
  .panel-head { gap: 8px; }
  .chart-legend { min-width: 0; }
  .legend-item { padding: 3px 2px; }
  .reliability-progress { grid-template-columns: 1fr; }
  .reliability-controls .action-button { flex: 1 1 140px; }
  .calibration-grid { grid-template-columns: 1fr; }
  .calibration-actions { flex-direction: column; }
}
)rawliteral";
