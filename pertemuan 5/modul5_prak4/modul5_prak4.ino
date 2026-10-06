#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DHT.h>

// =====================================================
// KONFIGURASI WIFI
// =====================================================

const char* ssid = "KING_Emyu";
const char* password = "baktiari";

// =====================================================
// PIN
// =====================================================

// DHT22
const byte dhtPin = 2;       // D4 / GPIO2

// Push button
const byte buttonPin = 4;    // D2 / GPIO4

// LED ON/OFF lama
const byte ledPin = 12;      // D6 / GPIO12

// LED DIMMER baru
const byte pwmLedPin = 5;    // D1 / GPIO5

// =====================================================
// DHT22
// =====================================================

DHT dht(dhtPin, DHT22);

// =====================================================
// WEBSERVER & WEBSOCKET
// =====================================================

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// =====================================================
// STATUS LED ON/OFF
// =====================================================

bool ledState = false;

// =====================================================
// STATUS SENSOR
// =====================================================

String currentTemp = "--";
String currentHum = "--";

// =====================================================
// STATUS PWM
// Nilai 0 - 1023
// =====================================================

int pwmValue = 0;

// =====================================================
// TIMER DHT22
// =====================================================

unsigned long lastDhtRead = 0;
const unsigned long dhtInterval = 3000;

// =====================================================
// DEBOUNCE PUSH BUTTON
// =====================================================

unsigned long lastButtonPress = 0;
const unsigned long debounceDelay = 200;

// =====================================================
// HTML DASHBOARD
// =====================================================

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>

<head>

  <meta charset="UTF-8">

  <meta name="viewport"
        content="width=device-width, initial-scale=1">

  <title>Smart Lighting</title>

  <style>

    body {
      font-family: Arial, sans-serif;
      text-align: center;
      background: #f2f2f2;
      margin: 0;
      padding: 20px;
    }

    h1 {
      margin-bottom: 20px;
    }

    .card {
      background: white;
      max-width: 450px;
      margin: 15px auto;
      padding: 20px;
      border-radius: 12px;

      box-shadow:
        0 3px 10px rgba(0,0,0,0.15);
    }

    h2 {
      margin: 10px 0;
    }

    button {
      padding: 12px 25px;
      font-size: 16px;
      border: none;
      border-radius: 8px;

      cursor: pointer;

      background: #333;
      color: white;
    }

    button:hover {
      background: #555;
    }

    .slider {
      width: 90%;
      margin: 20px auto;
    }

    input[type="range"] {
      width: 100%;
    }

    #pwmValue {
      font-size: 24px;
      font-weight: bold;
    }

    #connection {
      color: gray;
      font-size: 14px;
    }

  </style>

</head>


<body>

  <h1>Smart Lighting</h1>


  <!-- =============================== -->
  <!-- SENSOR SUHU -->
  <!-- =============================== -->

  <div class="card">

    <h2>
      Suhu:
      <span id="tempValue">--</span>
      °C
    </h2>

  </div>


  <!-- =============================== -->
  <!-- SENSOR KELEMBAPAN -->
  <!-- =============================== -->

  <div class="card">

    <h2>
      Kelembapan:
      <span id="humValue">--</span>
      %
    </h2>

  </div>


  <!-- =============================== -->
  <!-- LED ON / OFF LAMA -->
  <!-- =============================== -->

  <div class="card">

    <h2>
      LED Digital:
      <span id="ledStatus">OFF</span>
    </h2>

    <button onclick="toggleLED()">
      TOGGLE LED
    </button>

  </div>


  <!-- =============================== -->
  <!-- LED DIMMER -->
  <!-- =============================== -->

  <div class="card">

    <h2>
      Smart Lighting
    </h2>

    <p>
      Intensitas Lampu:
      <span id="pwmValue">0</span>
    </p>


    <div class="slider">

      <input
        type="range"
        min="0"
        max="1023"
        value="0"
        id="pwmSlider"
        oninput="sendPWM(this.value)"
      >

    </div>


    <p>
      0 = Mati &nbsp;&nbsp;&nbsp;
      1023 = Maksimal
    </p>

  </div>


  <p id="connection">
    Menghubungkan ke WebSocket...
  </p>


  <script>

    // =================================================
    // WEBSOCKET
    // =================================================

    var gateway =
      `ws://${window.location.hostname}/ws`;

    var websocket;


    // =================================================
    // MEMULAI WEBSOCKET
    // =================================================

    function initWebSocket() {

      console.log(
        "Mencoba menghubungkan WebSocket..."
      );

      websocket =
        new WebSocket(gateway);


      websocket.onopen = function() {

        console.log(
          "WebSocket terhubung"
        );

        document.getElementById(
          "connection"
        ).textContent =
          "WebSocket: Terhubung";
      };


      websocket.onclose = function() {

        console.log(
          "WebSocket terputus"
        );

        document.getElementById(
          "connection"
        ).textContent =
          "WebSocket: Terputus, mencoba kembali...";


        setTimeout(
          initWebSocket,
          2000
        );
      };


      websocket.onmessage =
        onMessage;
    }


    // =================================================
    // MENERIMA DATA DARI NODEMCU
    // =================================================

    function onMessage(event) {

      let data =
        JSON.parse(event.data);


      // -----------------------------
      // SUHU
      // -----------------------------

      document.getElementById(
        "tempValue"
      ).textContent =
        data.suhu;


      // -----------------------------
      // KELEMBAPAN
      // -----------------------------

      document.getElementById(
        "humValue"
      ).textContent =
        data.hum;


      // -----------------------------
      // LED DIGITAL
      // -----------------------------

      if (data.led == "1") {

        document.getElementById(
          "ledStatus"
        ).textContent =
          "ON";

      }

      else {

        document.getElementById(
          "ledStatus"
        ).textContent =
          "OFF";
      }


      // -----------------------------
      // PWM
      // -----------------------------

      if (data.pwm !== undefined) {

        document.getElementById(
          "pwmValue"
        ).textContent =
          data.pwm;


        document.getElementById(
          "pwmSlider"
        ).value =
          data.pwm;
      }

    }


    // =================================================
    // TOGGLE LED DIGITAL
    // =================================================

    function toggleLED() {

      if (
        websocket.readyState ===
        WebSocket.OPEN
      ) {

        websocket.send(
          "toggle"
        );
      }
    }


    // =================================================
    // KIRIM NILAI PWM
    // =================================================

    function sendPWM(value) {

      // Tampilkan nilai PWM
      // langsung pada halaman

      document.getElementById(
        "pwmValue"
      ).textContent =
        value;


      // Kirim ke NodeMCU
      // format:
      // pwm,512

      if (
        websocket.readyState ===
        WebSocket.OPEN
      ) {

        websocket.send(
          "pwm," + value
        );
      }
    }


    // =================================================
    // JALANKAN WEBSOCKET
    // =================================================

    window.onload =
      initWebSocket;

  </script>

</body>

</html>
)rawliteral";


// =====================================================
// MENGIRIM STATUS KE SEMUA CLIENT
// =====================================================

void notifyClients() {

  String message =
    "{\"led\":\"" +
    String(
      ledState ? "1" : "0"
    ) +

    "\",\"suhu\":\"" +
    currentTemp +

    "\",\"hum\":\"" +
    currentHum +

    "\",\"pwm\":\"" +
    String(pwmValue) +

    "\"}";


  ws.textAll(
    message
  );
}


// =====================================================
// MENGATUR LED DIGITAL
// =====================================================

void setLed(
  bool state
) {

  ledState =
    state;


  digitalWrite(
    ledPin,
    ledState
      ? HIGH
      : LOW
  );


  notifyClients();
}


// =====================================================
// MENANGANI PESAN WEBSOCKET
// =====================================================

void handleWebSocketMessage(
  void *arg,
  uint8_t *data,
  size_t len
) {

  AwsFrameInfo *info =
    (AwsFrameInfo *)arg;


  if (
    info->final &&
    info->index == 0 &&
    info->len == len &&
    info->opcode == WS_TEXT
  ) {

    // Mengakhiri string
    data[len] = 0;


    String message =
      String(
        (char*)data
      );


    // =================================================
    // PERINTAH TOGGLE LED
    // =================================================

    if (
      message == "toggle"
    ) {

      setLed(
        !ledState
      );
    }


    // =================================================
    // PERINTAH PWM
    // Format:
    // pwm,512
    // =================================================

    else if (
      message.startsWith(
        "pwm,"
      )
    ) {

      // Ambil bagian angka
      // setelah "pwm,"

      String pwmString =
        message.substring(4);


      // Konversi String menjadi integer

      int newPWM =
        pwmString.toInt();


      // Pastikan nilai berada
      // dalam rentang 0-1023

      if (newPWM < 0) {

        newPWM = 0;
      }


      if (newPWM > 1023) {

        newPWM = 1023;
      }


      // Simpan nilai PWM

      pwmValue =
        newPWM;


      // Terapkan PWM ke LED

      analogWrite(
        pwmLedPin,
        pwmValue
      );


      // Kirim status terbaru
      // ke semua browser

      notifyClients();


      // Tampilkan di Serial Monitor

      Serial.print(
        "PWM diterima: "
      );

      Serial.println(
        pwmValue
      );
    }
  }
}


// =====================================================
// EVENT WEBSOCKET
// =====================================================

void onWebSocketEvent(
  AsyncWebSocket *server,
  AsyncWebSocketClient *client,
  AwsEventType type,
  void *arg,
  uint8_t *data,
  size_t len
) {

  switch (
    type
  ) {

    case WS_EVT_CONNECT:

      Serial.printf(
        "WebSocket Client #%u terhubung\n",
        client->id()
      );


      // Kirim status awal

      client->text(
        "{\"led\":\"" +
        String(
          ledState ? "1" : "0"
        ) +

        "\",\"suhu\":\"" +
        currentTemp +

        "\",\"hum\":\"" +
        currentHum +

        "\",\"pwm\":\"" +
        String(pwmValue) +

        "\"}"
      );

      break;


    case WS_EVT_DISCONNECT:

      Serial.printf(
        "WebSocket Client #%u terputus\n",
        client->id()
      );

      break;


    case WS_EVT_DATA:

      handleWebSocketMessage(
        arg,
        data,
        len
      );

      break;


    default:

      break;
  }
}


// =====================================================
// SETUP
// =====================================================

void setup() {

  // ===================================================
  // SERIAL
  // ===================================================

  Serial.begin(
    115200
  );

  Serial.println();

  Serial.println(
    "================================"
  );

  Serial.println(
    "SMART LIGHTING ESP8266"
  );

  Serial.println(
    "================================"
  );


  // ===================================================
  // PIN LED DIGITAL
  // ===================================================

  pinMode(
    ledPin,
    OUTPUT
  );


  digitalWrite(
    ledPin,
    LOW
  );


  // ===================================================
  // PIN LED PWM
  // ===================================================

  pinMode(
    pwmLedPin,
    OUTPUT
  );


  analogWrite(
    pwmLedPin,
    0
  );


  // ===================================================
  // PUSH BUTTON
  // ===================================================

  pinMode(
    buttonPin,
    INPUT
  );


  // ===================================================
  // DHT22
  // ===================================================

  dht.begin();


  // ===================================================
  // WIFI
  // ===================================================

  WiFi.begin(
    ssid,
    password
  );


  Serial.print(
    "Menghubungkan ke WiFi"
  );


  while (
    WiFi.status() !=
    WL_CONNECTED
  ) {

    delay(500);

    Serial.print(
      "."
    );
  }


  Serial.println();

  Serial.println(
    "WiFi terhubung!"
  );


  Serial.print(
    "IP Address: "
  );


  Serial.println(
    WiFi.localIP()
  );


  // ===================================================
  // WEBSOCKET
  // ===================================================

  ws.onEvent(
    onWebSocketEvent
  );


  server.addHandler(
    &ws
  );


  // ===================================================
  // WEB SERVER
  // ===================================================

  server.on(
    "/",
    HTTP_GET,
    [](AsyncWebServerRequest *request) {

      request->send_P(
        200,
        "text/html",
        index_html
      );

    }
  );


  // ===================================================
  // MULAI SERVER
  // ===================================================

  server.begin();


  Serial.println(
    "Web server aktif!"
  );

  Serial.println(
    "Buka IP Address di browser."
  );
}


// =====================================================
// LOOP
// =====================================================

void loop() {

  // ===================================================
  // BACA DHT22 SETIAP 3 DETIK
  // ===================================================

  if (
    millis() -
    lastDhtRead >=
    dhtInterval
  ) {

    lastDhtRead =
      millis();


    // -----------------------------
    // BACA SUHU
    // -----------------------------

    float temperature =
      dht.readTemperature();


    // -----------------------------
    // BACA KELEMBAPAN
    // -----------------------------

    float humidity =
      dht.readHumidity();


    // -----------------------------
    // SIMPAN SUHU
    // -----------------------------

    if (
      !isnan(
        temperature
      )
    ) {

      currentTemp =
        String(
          temperature,
          1
        );
    }


    // -----------------------------
    // SIMPAN KELEMBAPAN
    // -----------------------------

    if (
      !isnan(
        humidity
      )
    ) {

      currentHum =
        String(
          humidity,
          1
        );
    }


    // Kirim data sensor
    // ke browser

    notifyClients();


    // Serial Monitor

    Serial.print(
      "Suhu: "
    );

    Serial.print(
      currentTemp
    );

    Serial.print(
      " C | Kelembapan: "
    );

    Serial.print(
      currentHum
    );

    Serial.print(
      " % | PWM: "
    );

    Serial.println(
      pwmValue
    );
  }


  // ===================================================
  // PUSH BUTTON
  // ===================================================

  int buttonState =
    digitalRead(
      buttonPin
    );


  if (
    buttonState == HIGH &&
    millis() -
    lastButtonPress >
    debounceDelay
  ) {

    lastButtonPress =
      millis();


    // Toggle LED digital

    setLed(
      !ledState
    );
  }


  // ===================================================
  // CLEANUP WEBSOCKET
  // ===================================================

  ws.cleanupClients();
}