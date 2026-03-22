#ifndef WEBSURFACE_H
#define WEBSURFACE_H

#include <WiFi.h>
#include <WebServer.h>

// External functions from main.cpp
extern void homeAllAxes();
extern void drawSquare();
extern void drawNikolausHouse();
extern void moveToCenter();
extern void stopAllMotors();
extern bool isHomed;
extern String currentStatus;

const char* ssid = "ESP32-PLOTTER";
const char* password = "12345678";
WebServer server(80);

const char MAIN_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Pen Plotter Control</title>
  <style>
    body {
      font-family: 'Segoe UI', sans-serif;
      margin: 0;
      padding: 0;
      background-image: url('campus1.jpg');
      background-size: cover;
      background-position: center;
      background-repeat: no-repeat;
    }
    h1 {
      background: rgba(0,0,0,0.7);
      color: white;
      padding: 20px;
      margin: 0;
      text-align: center;
    }
    .layout {
      display: flex;
      justify-content: space-between;
      align-items: flex-start;
      padding: 20px;
      flex-wrap: wrap;
    }
    canvas {
      border: 2px solid #888;
      background: white;
      box-shadow: 0 0 10px rgba(0,0,0,0.1);
      touch-action: none;
    }
    .controls {
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 15px;
    }
    .controls button {
      font-size: 20px;
      padding: 16px 28px;
      min-width: 200px;
      border: none;
      border-radius: 12px;
      color: white;
      cursor: pointer;
      box-shadow: 0 4px 6px rgba(0,0,0,0.2);
      transition: transform 0.2s ease, background 0.3s ease;
      backdrop-filter: blur(10px);
    }
    .controls button:active {
      transform: scale(0.95);
    }
    .controls button:nth-child(1) { background: #007BFF; }  /* Home */
    .controls button:nth-child(2) { background: #343A40; }  /* Square */
    .controls button:nth-child(3) { background: #28a745; }  /* Nikolaus */
    .controls button:nth-child(4) { background: #17a2b8; }  /* Center */
    .controls button:nth-child(5) { background: #dc3545; }  /* Emergency Stop */
    .controls button:nth-child(6) { background: #ffc107; }  /* Reset */

    #status {
      font-size: 18px;
      margin: 20px;
      text-align: center;
      color: #fff;
      text-shadow: 1px 1px 2px black;
    }
  </style>
</head>
<body>
  <h1>🖊️ Pen Plotter Control</h1>
  <div id="status">Status: Loading...</div>
  <div class="layout">
    <canvas id="preview" width="300" height="300"></canvas>

    <div class="controls">
      <button onclick="sendCommand('home')">🏠 Home</button>
      <button onclick="sendCommand('square'); animateSquare();">⬛ Draw Square</button>
      <button onclick="sendCommand('nikolaus'); animateNikolaus();">🏡 Nikolaus House</button>
      <button onclick="sendCommand('center')">🎯 Center</button>
      <button onclick="sendCommand('stop')">🛑 Emergency Stop</button>
      <button onclick="sendCommand('reset')">🔄 Reset ESP</button>
    </div>
  </div>

  <script>
    const canvas = document.getElementById("preview");
    const ctx = canvas.getContext("2d");

    function sendCommand(cmd) {
      fetch('/command?cmd=' + cmd);
    }

    async function updateStatus() {
      try {
        const res = await fetch('/status');
        const text = await res.text();
        document.getElementById('status').innerText = 'Status: ' + text;
      } catch (e) {
        document.getElementById('status').innerText = 'Status: Offline';
      }
    }
    setInterval(updateStatus, 1000);

    function clearCanvas() {
      ctx.clearRect(0, 0, canvas.width, canvas.height);
    }

    function animateNikolaus() {
      clearCanvas();
      ctx.lineWidth = 2;

      const points = [
        {x:50,y:250}, {x:250,y:250},
        {x:250,y:50}, {x:50,y:50},
        {x:50,y:250},
        {x:50,y:250}, {x:250,y:50},
        {x:250,y:250}, {x:50,y:50},
        {x:50,y:50}, {x:150,y:0}, {x:250,y:50}
      ];

      let i = 0;
      function drawStep() {
        if (i >= points.length - 1) return;
        ctx.beginPath();
        ctx.moveTo(points[i].x, points[i].y);
        ctx.lineTo(points[i+1].x, points[i+1].y);

        if (i < 4) ctx.strokeStyle = '#007BFF';
        else if (i < 8) ctx.strokeStyle = '#28a745';
        else ctx.strokeStyle = '#FF5733';

        ctx.stroke();
        i++;
        setTimeout(drawStep, 400);
      }
      drawStep();
    }

    function animateSquare() {
      clearCanvas();
      ctx.lineWidth = 2;
      ctx.strokeStyle = '#007BFF';
      ctx.beginPath();
      ctx.moveTo(50,250);
      ctx.lineTo(250,250);
      ctx.lineTo(250,50);
      ctx.lineTo(50,50);
      ctx.lineTo(50,250);
      ctx.stroke();
    }
  </script>
</body>
</html>
)rawliteral";

void initWebSurface() {
  WiFi.softAP(ssid, password);
  delay(100);

  server.on("/", HTTP_GET, []() {
    server.send_P(200, "text/html", MAIN_page);
  });

  server.on("/status", HTTP_GET, []() {
    server.send(200, "text/plain", currentStatus);
  });

  server.on("/command", HTTP_GET, []() {
    String cmd = server.arg("cmd");
    if (cmd == "home") homeAllAxes();
    else if (cmd == "square") drawSquare();
    else if (cmd == "nikolaus") drawNikolausHouse();
    else if (cmd == "center") moveToCenter();
    else if (cmd == "stop") stopAllMotors();
    else if (cmd == "reset") ESP.restart();
    server.send(200, "text/plain", "OK");
  });

  server.begin();
}

void handleWebSurface() {
  server.handleClient();
}

#endif
