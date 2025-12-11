#include <WiFiS3.h>
#include <WebSocketsServer.h>

IPAddress staticIP(192, 168, 1, 50);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(192, 168, 1, 1);

const char* ssid = "Borisov Castle";
const char* password = "";

#define FLASH_PIN 4
#define BUZZ_PIN 6
#define MIC_PIN A0
#define SAMPLES 123

WiFiServer httpServer(80);
WebSocketsServer webSocket(81);

bool micActive = false;

const char* html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>ESP32 Lab</title>
    <style>
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body {
            font-family: -apple-system, sans-serif;
            background: #1a1a2e;
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 20px;
        }
        .panel {
            background: #16213e;
            border-radius: 30px;
            padding: 30px 20px;
            max-width: 300px;
            width: 100%;
        }
        h1 { color: #fff; text-align: center; font-size: 20px; margin-bottom: 20px; }
        h2 { color: #fff; margin: 20px 0 10px; font-size: 14px; }
        .status { color: #e94560; text-align: center; font-size: 12px; margin-bottom: 20px; }
        .status.connected { color: #4ade80; }
        .section { margin-bottom: 25px; }
        .row { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; }
        .btn {
            background: #0f3460;
            color: #fff;
            border: none;
            border-radius: 12px;
            padding: 16px;
            font-size: 16px;
            cursor: pointer;
            width: 100%;
        }
        .btn:active { background: #e94560; }
        .slider {
            width: 100%;
            height: 8px;
            border-radius: 4px;
            background: #0f3460;
            outline: none;
            -webkit-appearance: none;
            margin: 10px 0;
        }
        .slider::-webkit-slider-thumb {
            -webkit-appearance: none;
            width: 24px;
            height: 24px;
            border-radius: 50%;
            background: #e94560;
            cursor: pointer;
        }
        .value { color: #888; font-size: 12px; text-align: center; }
        .mic-bar {
            background: #0f3460;
            height: 24px;
            border-radius: 8px;
            overflow: hidden;
            margin: 10px 0;
        }
        .mic-level {
            background: linear-gradient(90deg, #4ade80, #e94560);
            height: 100%;
            width: 0%;
            transition: width 0.1s;
        }
    </style>
</head>
<body>
    <div class="panel">
        <h1>ESP32 Lab</h1>
        <div class="status" id="status">Connecting...</div>

        <div class="section">
            <h2>7-Color Flash</h2>
            <div class="row">
                <button class="btn" onclick="send('flash_on')">On</button>
                <button class="btn" onclick="send('flash_off')">Off</button>
            </div>
        </div>

        <div class="section">
            <h2>Buzzer</h2>
            <input type="range" class="slider" id="freq" min="100" max="5000" value="1000"
                   oninput="document.getElementById('freqVal').textContent = this.value + ' Hz'">
            <div class="value" id="freqVal">1000 Hz</div>
            <input type="range" class="slider" id="dur" min="50" max="2000" value="500"
                   oninput="document.getElementById('durVal').textContent = this.value + ' ms'">
            <div class="value" id="durVal">500 ms</div>
            <div class="row">
                <button class="btn" onclick="playTone()">Play</button>
                <button class="btn" onclick="send('buzz_stop')">Stop</button>
            </div>
        </div>

        <div class="section">
            <h2>Microphone</h2>
            <div class="mic-bar">
                <div class="mic-level" id="micBar"></div>
            </div>
            <div class="value" id="micVal">0</div>
            <div class="row">
                <button class="btn" onclick="send('mic_on')">Start</button>
                <button class="btn" onclick="send('mic_off')">Stop</button>
            </div>
        </div>
    </div>

    <script>
        let ws;
        const status = document.getElementById('status');

        function connect() {
            ws = new WebSocket('ws://' + location.hostname + ':81/');
            ws.onopen = () => {
                status.textContent = 'Connected';
                status.className = 'status connected';
            };
            ws.onclose = () => {
                status.textContent = 'Reconnecting...';
                status.className = 'status';
                setTimeout(connect, 1000);
            };
            ws.onmessage = (e) => {
                if (e.data.startsWith('mic_')) {
                    const level = parseInt(e.data.split('_')[1]);
                    document.getElementById('micBar').style.width = level + '%';
                    document.getElementById('micVal').textContent = level;
                }
            };
        }

        function send(cmd) {
            if (ws && ws.readyState === 1) ws.send(cmd);
        }

        function playTone() {
            const f = document.getElementById('freq').value;
            const d = document.getElementById('dur').value;
            send('buzz_' + f + '_' + d);
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

        if (cmd == "flash_on") {
            digitalWrite(FLASH_PIN, HIGH);
        }
        else if (cmd == "flash_off") {
            digitalWrite(FLASH_PIN, LOW);
        }
        else if (cmd.startsWith("buzz_") && cmd != "buzz_stop") {
            // Parse "buzz_1000_500" -> freq=1000, duration=500
            int firstUnderscore = cmd.indexOf('_');
            int secondUnderscore = cmd.indexOf('_', firstUnderscore + 1);
            int freq = cmd.substring(firstUnderscore + 1, secondUnderscore).toInt();
            int dur = cmd.substring(secondUnderscore + 1).toInt();
            tone(BUZZ_PIN, freq, dur);
        }
        else if (cmd == "buzz_stop") {
            noTone(BUZZ_PIN);
        }
        else if (cmd == "mic_on") {
            micActive = true;
        }
        else if (cmd == "mic_off") {
            micActive = false;
        }
    }
}

void setup() {
    Serial.begin(115200);
    
    WiFi.config(staticIP, dns, gateway, subnet);
    WiFi.begin(ssid, password);

    pinMode(FLASH_PIN, OUTPUT);
    pinMode(BUZZ_PIN, OUTPUT);
    pinMode(MIC_PIN, INPUT);

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print('.');
        delay(300);
    }
    Serial.println();
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    httpServer.begin();
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
}

unsigned long lastMicSend = 0;

void loop() {
    webSocket.loop();

    // Send mic data every 100ms when active
    if (micActive && millis() - lastMicSend > 100) {
        int minVal = 1000;
        int maxVal = 0;
        for (int i = 0; i < SAMPLES; i++) {
            int raw = analogRead(MIC_PIN);
            if (raw < minVal) minVal = raw;
            if (raw > maxVal) maxVal = raw;
            delayMicroseconds(67);
        }
        int level = map(maxVal - minVal, 0, 1024,0,100);
        webSocket.broadcastTXT("mic_" + String(level));
        if (level > 20){
            tone(BUZZ_PIN, 1968, 100 * level);
        }
        Serial.println(level, maxVal-minVal);
        lastMicSend = millis();
    }

    WiFiClient client = httpServer.available();
    if (client) {
        client.readStringUntil('\r');
        client.print("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
        client.print(html);
        client.stop();
    }
}
