const http = require("http");
const fs = require("fs");
const path = require("path");

const root = path.join(__dirname, "..");
const port = Number(process.env.PORT || 3000);

function extract(file, varName) {
  const source = fs.readFileSync(path.join(root, file), "utf8");
  const pattern = new RegExp(
    varName + '\\s*\\[\\]\\s*PROGMEM\\s*=\\s*R"rawliteral\\(([\\s\\S]*?)\\)rawliteral"',
  );
  const match = source.match(pattern);
  if (!match) throw new Error(`Could not extract ${varName} from ${file}`);
  return match[1];
}

const assets = {
  "/": ["text/html", extract("webpage.h", "index_html")],
  "/index.html": ["text/html", extract("webpage.h", "index_html")],
  "/style.css": ["text/css", extract("style.h", "style_css")],
  "/app.js": ["application/javascript", extract("app.h", "app_js")],
};

let mockVoltage = 12.34;
let mockCurrent = 125.6;

const server = http.createServer((request, response) => {
  const url = new URL(request.url, `http://${request.headers.host}`);

  if (url.pathname === "/api/data") {
    mockVoltage += (Math.random() - 0.5) * 0.08;
    mockCurrent += (Math.random() - 0.5) * 2.0;
    const power = mockVoltage * mockCurrent;
    response.writeHead(200, {
      "Content-Type": "application/json",
      "Cache-Control": "no-store",
    });
    response.end(JSON.stringify({
      voltage_v: mockVoltage,
      current_ma: mockCurrent,
      power_mw: power,
      bus_voltage_v: mockVoltage - 0.02,
      shunt_voltage_mv: mockCurrent * 0.1,
      sensor_ok: true,
      wifi_connected: true,
      free_heap_bytes: 178000,
      wifi_clients: 1,
      rgb_led_pin: 48,
      firmware_version: "2.0.0",
      uptime_ms: Date.now() % 86400000,
    }));
    return;
  }

  if (url.pathname === "/api/status") {
    response.writeHead(200, {
      "Content-Type": "application/json",
      "Cache-Control": "no-store",
    });
    response.end(JSON.stringify({
      sensor_ok: true,
      ina219_ok: true,
      oled_ok: true,
      wifi_connected: true,
      wifi_ssid: "PLTA-Monitor",
      ip_address: "192.168.4.1",
      zero_voltage_v: 0.840,
      zero_bus_voltage_v: 0.840,
      zero_current_ma: -0.260,
      free_heap_bytes: 178000,
      wifi_clients: 1,
      rgb_led_pin: 48,
      alarm_low_voltage_v: 1.5,
      alarm_high_current_ma: 1000,
      alarm_low_power_mw: 50,
      firmware_version: "2.0.0",
      uptime_ms: Date.now() % 86400000,
    }));
    return;
  }

  if (url.pathname === "/api/calibrate" && request.method === "POST") {
    response.writeHead(200, {
      "Content-Type": "application/json",
      "Cache-Control": "no-store",
    });
    response.end(JSON.stringify({ calibrated: true }));
    return;
  }

  if (url.pathname === "/api/restart" && request.method === "POST") {
    response.writeHead(200, {
      "Content-Type": "application/json",
      "Cache-Control": "no-store",
    });
    response.end(JSON.stringify({ restart_scheduled: true }));
    return;
  }

  const asset = assets[url.pathname];
  if (!asset) {
    response.writeHead(404, { "Content-Type": "text/plain" });
    response.end("Not found");
    return;
  }

  response.writeHead(200, {
    "Content-Type": asset[0],
    "Cache-Control": "no-store",
  });
  response.end(asset[1]);
});

server.listen(port, "0.0.0.0", () => {
  console.log(`Dashboard preview: http://localhost:${port}`);
  console.log(`Mobile LAN preview: http://<this-PC-IP>:${port}`);
});
