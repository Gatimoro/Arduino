//My actual remote as of now. I was experimenting with the queue to handle button spams.
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <IRremote.hpp>
IPAddress staticIP(192, 168, 1, 51);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(192, 168, 1, 1);

const char* ssid = "Borisov Castle";
const char* password = "";

#define IR_SEND_PIN 13

WiFiServer httpServer(80);
WebSocketsServer webSocket(81); 

void sendCommand(uint8_t cmd) {
    IrSender.sendNEC(0x0707, cmd, 2);
}

const char* html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Remote</title>
    <style>
        * { box-sizing: border-box; margin: 0; padding: 0; }
        
        body {
            font-family: 'SF Pro Display', -apple-system, sans-serif;
            background: linear-gradient(135deg, #0f0f1a 0%, #1a1a2e 50%, #16213e 100%);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 20px;
        }
        
        .remote {
            background: rgba(22, 33, 62, 0.8);
            backdrop-filter: blur(20px);
            border: 1px solid rgba(255, 255, 255, 0.1);
            border-radius: 40px;
            padding: 40px 30px;
            max-width: 320px;
            width: 100%;
            box-shadow: 
                0 25px 50px rgba(0, 0, 0, 0.5),
                inset 0 1px 0 rgba(255, 255, 255, 0.1);
        }
        
        .header {
            text-align: center;
            margin-bottom: 30px;
        }
        
        .header h1 {
            color: #fff;
            font-size: 24px;
            font-weight: 600;
            letter-spacing: -0.5px;
        }
        
        .status {
            display: inline-flex;
            align-items: center;
            gap: 6px;
            color: #e94560;
            font-size: 11px;
            text-transform: uppercase;
            letter-spacing: 1px;
            margin-top: 8px;
        }
        
        .status::before {
            content: '';
            width: 6px;
            height: 6px;
            border-radius: 50%;
            background: currentColor;
            animation: pulse 2s infinite;
        }
        
        .status.connected { color: #4ade80; }
        
        @keyframes pulse {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.4; }
        }
        
        .btn {
            background: linear-gradient(145deg, #1e3a5f, #0f2744);
            color: #fff;
            border: 1px solid rgba(255, 255, 255, 0.05);
            border-radius: 16px;
            padding: 20px;
            font-size: 15px;
            font-weight: 500;
            cursor: pointer;
            width: 100%;
            transition: all 0.15s ease;
            box-shadow: 
                0 4px 15px rgba(0, 0, 0, 0.3),
                inset 0 1px 0 rgba(255, 255, 255, 0.1);
        }
        
        .btn:active {
            transform: scale(0.96);
            background: linear-gradient(145deg, #e94560, #d63d56);
            box-shadow: 0 2px 10px rgba(233, 69, 96, 0.4);
        }
        
        .power {
            background: linear-gradient(145deg, #e94560, #c73e54);
            margin-bottom: 30px;
            padding: 22px;
            font-size: 16px;
            box-shadow: 0 8px 25px rgba(233, 69, 96, 0.3);
        }
        
        .power:active {
            background: linear-gradient(145deg, #ff5a75, #e94560);
        }
        
        .dpad {
            display: grid;
            grid-template-columns: repeat(3, 1fr);
            gap: 10px;
            margin-bottom: 25px;
        }
        
        .dpad .btn {
            padding: 24px 0;
            font-size: 18px;
        }
        
        .empty { visibility: hidden; }
        
        .ok {
            background: linear-gradient(145deg, #e94560, #c73e54);
            border-radius: 50%;
            aspect-ratio: 1;
            display: flex;
            align-items: center;
            justify-content: center;
            padding: 0;
            box-shadow: 0 6px 20px rgba(233, 69, 96, 0.35);
        }
        
        .controls {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 12px;
            margin-bottom: 15px;
        }
        
        .queue {
            background: rgba(0, 0, 0, 0.3);
            border-radius: 12px;
            padding: 12px;
            margin-top: 25px;
            border: 1px solid rgba(255, 255, 255, 0.05);
        }
        
        .queue-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 8px;
        }
        
        .queue-title {
            color: #888;
            font-size: 11px;
            text-transform: uppercase;
            letter-spacing: 1px;
        }
        
        .queue-count {
            color: #e94560;
            font-size: 12px;
            font-weight: 600;
        }
        
        .queue-items {
            color: #fff;
            font-size: 13px;
            min-height: 20px;
            word-break: break-all;
        }
        
        .queue-clear {
            margin-top: 12px;
            background: rgba(255, 255, 255, 0.1);
            padding: 12px;
            font-size: 12px;
            border-radius: 10px;
        }
        
        .multiplier {
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 15px;
            margin-top: 20px;
            padding: 15px;
            background: rgba(0, 0, 0, 0.2);
            border-radius: 12px;
        }
        
        .mult-btn {
            background: rgba(255, 255, 255, 0.1);
            color: #fff;
            border: none;
            border-radius: 8px;
            width: 36px;
            height: 36px;
            font-size: 18px;
            cursor: pointer;
        }
        
        .mult-btn:active { background: #e94560; }
        
        .mult-value {
            color: #fff;
            font-size: 18px;
            font-weight: 600;
            min-width: 40px;
            text-align: center;
        }
        
        .mult-label {
            color: #888;
            font-size: 11px;
            text-transform: uppercase;
            letter-spacing: 1px;
        }
        .toggle-btn {
          background: rgba(255, 255, 255, 0.1);
          color: #888;
          border: none;
          border-radius: 8px;
          padding: 8px 12px;
          font-size: 11px;
          cursor: pointer;
          margin-top: 20px;
          width: auto;
      }
      
      .toggle-btn:active {
          background: rgba(255, 255, 255, 0.2);
      }
      
      .extras {
          overflow: hidden;
          transition: max-height 0.3s ease, opacity 0.3s ease;
          max-height: 500px;
          opacity: 1;
      }
      
      .extras.hidden {
          max-height: 0;
          opacity: 0;
      }
    </style>
</head>
<body>
    <div class="remote">
        <div class="header">
            <h1>Remote</h1>
            <div class="status" id="status">Connecting</div>
        </div>
        
        <button class="btn power" onclick="q('power')">Power</button>
        
        <div class="dpad">
            <div class="empty"></div>
            <button class="btn" onclick="q('up')">Up</button>
            <div class="empty"></div>
            <button class="btn" onclick="q('left')">Left</button>
            <button class="btn ok" onclick="q('ok')">OK</button>
            <button class="btn" onclick="q('right')">Right</button>
            <div class="empty"></div>
            <button class="btn" onclick="q('down')">Down</button>
            <div class="empty"></div>
        </div>
        
        <div class="controls">
            <button class="btn" onclick="q('volup')">Vol +</button>
            <button class="btn" onclick="q('voldown')">Vol -</button>
        </div>
        
        <div class="controls">
    <button class="btn" onclick="q('home')">Home</button>
    <button class="btn" onclick="q('back')">Back</button>
</div>

<button class="toggle-btn" onclick="toggleExtras()">More</button>

<div class="extras hidden" id="extras">
    <div class="multiplier">
        <span class="mult-label">Repeat</span>
        <button class="mult-btn" onclick="setMult(-1)">-</button>
        <span class="mult-value" id="mult">1</span>
        <button class="mult-btn" onclick="setMult(1)">+</button>
    </div>
    
    <div class="queue">
        <div class="queue-header">
            <span class="queue-title">Queue</span>
            <span class="queue-count" id="qcount">0</span>
        </div>
        <div class="queue-items" id="qitems">-</div>
        <button class="btn queue-clear" onclick="clearQueue()">Clear</button>
    </div>
</div>
    </div>
    
    <script>
        let ws;
        let queue = [];
        let mult = 1;
        let sending = false;
        const status = document.getElementById('status');
        
        function connect() {
            ws = new WebSocket('ws://' + location.hostname + ':81/');
            ws.onopen = () => {
                status.textContent = 'Connected';
                status.className = 'status connected';
            };
            ws.onclose = () => {
                status.textContent = 'Reconnecting';
                status.className = 'status';
                setTimeout(connect, 1000);
            };
            ws.onmessage = (e) => {
                if (e.data === 'done') {
                    sending = false;
                    sendNext();
                }
            };
        }
        
        function q(cmd) {
            for (let i = 0; i < mult; i++) {
                queue.push(cmd);
            }
            updateDisplay();
            sendNext();
        }
        
        function sendNext() {
            if (sending || queue.length === 0) return;
            if (!ws || ws.readyState !== 1) return;
            
            sending = true;
            const cmd = queue.shift();
            updateDisplay();
            ws.send(cmd);
        }
        
        function updateDisplay() {
            document.getElementById('qcount').textContent = queue.length;
            document.getElementById('qitems').textContent = queue.length ? queue.join(' > ') : '-';
        }
        
        function clearQueue() {
            queue = [];
            updateDisplay();
        }
        
        function setMult(delta) {
            mult = Math.max(1, Math.min(20, mult + delta));
            document.getElementById('mult').textContent = mult;
        }
        function toggleExtras() {
            const extras = document.getElementById('extras');
            const btn = event.target;
            extras.classList.toggle('hidden');
            btn.textContent = extras.classList.contains('hidden') ? 'More' : 'Less';
        }
        
        connect();
    </script>
</body>
</html>
)rawliteral";

void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    if (type == WStype_TEXT) {
        String cmd = String((char*)payload);
        Serial.println(cmd);
        
        if (cmd == "power") sendCommand(0x02);
        else if (cmd == "up") sendCommand(0x60);
        else if (cmd == "down") sendCommand(0x61);
        else if (cmd == "left") sendCommand(0x65);
        else if (cmd == "right") sendCommand(0x62);
        else if (cmd == "ok") sendCommand(0x68);
        else if (cmd == "volup") sendCommand(0x07);
        else if (cmd == "voldown") sendCommand(0x0B);
        else if (cmd == "mute") sendCommand(0x0F);
        else if (cmd == "back") sendCommand(0x58);
        else if (cmd == "home") sendCommand(0x79);
        delay(30);
        webSocket.sendTXT(num, "done");
    }
}

void setup() {
    pinMode(IR_SEND_PIN, OUTPUT);
    Serial.begin(115200);
    delay(2000);
    
    WiFi.config(staticIP, dns, gateway, subnet);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) delay(500);
    delay(2000);
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    
    IrSender.begin(IR_SEND_PIN, DISABLE_LED_FEEDBACK);
    
    httpServer.begin();
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
}

void loop() {
    webSocket.loop();
    
    WiFiClient client = httpServer.available();
    if (client) {
        client.readStringUntil('\r');
        client.print("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
        client.print(html);
        client.stop();
    }
}
