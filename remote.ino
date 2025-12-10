#include <WiFiS3.h>
#include <WebSocketsServer.h>
#include <IRremote.hpp>
IPAddress staticIP(192, 168, 1, 50);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(192, 168, 1, 1);

const char* ssid = "Borisov Castle";
const char* password = "";

#define IR_SEND_PIN 3

WiFiServer httpServer(80);
WebSocketsServer webSocket(81);  // WebSocket on different port

void sendCommand(uint8_t cmd) {
    IrSender.sendNEC(0x0707, cmd, 2);
}

const char* html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>TV Remote</title>
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
        .remote {
            background: #16213e;
            border-radius: 30px;
            padding: 30px 20px;
            max-width: 280px;
            width: 100%;
        }
        h1 { color: #fff; text-align: center; font-size: 18px; margin-bottom: 25px; }
        .status { color: #e94560; text-align: center; font-size: 12px; margin-bottom: 15px; }
        .status.connected { color: #4ade80; }
        .btn {
            background: #0f3460;
            color: #fff;
            border: none;
            border-radius: 12px;
            padding: 18px;
            font-size: 18px;
            cursor: pointer;
            width: 100%;
        }
        .btn:active { background: #e94560; }
        .power { background: #e94560; margin-bottom: 25px; }
        .dpad { display: grid; grid-template-columns: repeat(3, 1fr); gap: 8px; margin-bottom: 20px; }
        .dpad .btn { padding: 22px 0; }
        .empty { visibility: hidden; }
        .ok { background: #e94560; }
        .volume { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; margin-bottom: 15px; }
        .row { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; }
    </style>
</head>
<body>
    <div class="remote">
        <h1>TV Remote</h1>
        <div class="status" id="status">Connecting...</div>
        <button class="btn power" onclick="send('power')">Power</button>
        <div class="dpad">
            <div class="empty"></div>
            <button class="btn" onclick="send('up')">Up</button>
            <div class="empty"></div>
            <button class="btn" onclick="send('left')">Left</button>
            <button class="btn ok" onclick="send('ok')">OK</button>
            <button class="btn" onclick="send('right')">Right</button>
            <div class="empty"></div>
            <button class="btn" onclick="send('down')">Down</button>
            <div class="empty"></div>
        </div>
        <div class="volume">
            <button class="btn" onclick="send('volup')">Vol+</button>
            <button class="btn" onclick="send('voldown')">Vol-</button>
        </div>
        <div class="row">
            <button class="btn" onclick="send('home')">Home</button>
            <button class="btn" onclick="send('back')">Back</button>
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
        }
        
        function send(cmd) {
            if (ws && ws.readyState === 1) ws.send(cmd);
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
    }
}

void setup() {
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
    webSocket.loop();  // Handle WebSocket
    
    WiFiClient client = httpServer.available();
    if (client) {
        client.readStringUntil('\r');
        client.print("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
        client.print(html);
        client.stop();
    }
}
