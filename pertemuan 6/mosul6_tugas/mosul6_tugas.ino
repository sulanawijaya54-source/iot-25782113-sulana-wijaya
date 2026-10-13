#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

// ========== KONFIGURASI WIFI ==========
const char* ssid = "kiww";
const char* password = "jaya12345";

// ========== KONFIGURASI MQTT ==========
const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;

const char* clientId = "ESP8266_LabIoT_25782113_Safety";

const char* topicTelemetri =
  "kampus/labiot/25782113/telemetri";
const char* topicKendali =
  "kampus/labiot/25782113/kendali";
const char* topicAlert =
  "kampus/labiot/25782113/alert";
const char* topicStatus =
  "kampus/labiot/25782113/status";

// Pesan LWT jika koneksi terputus tidak terduga
const char* willMessage =
  "{\"status\":\"OFFLINE\","
  "\"keterangan\":\"Perangkat Terputus Tak Terduga\"}";

const char* onlineMessage =
  "{\"status\":\"ONLINE\","
  "\"keterangan\":\"Perangkat Beroperasi Normal\"}";

// ========== KONFIGURASI PERANGKAT ==========
#define DHTPIN D4
#define DHTTYPE DHT22
#define LEDPIN D1

const float BATAS_SUHU = 31.0;

DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

// ========== VARIABEL SISTEM ==========
float suhu = NAN;
float kelembapan = NAN;

bool kondisiBahaya = false;
bool ledManual = false;

unsigned long waktuSensor = 0;
unsigned long waktuTelemetri = 0;
unsigned long waktuWiFi = 0;
unsigned long waktuMQTT = 0;

const unsigned long intervalSensor = 1000;
const unsigned long intervalTelemetri = 4000;
const unsigned long intervalWiFi = 10000;
const unsigned long intervalMQTT = 3000;

// ========== KENDALI LED ==========
void aturLED() {
  if (kondisiBahaya) {
    digitalWrite(LEDPIN, HIGH);
  } else {
    digitalWrite(LEDPIN, ledManual ? HIGH : LOW);
  }
}

// ========== CALLBACK PERINTAH MQTT ==========
void callback(char* topic, byte* payload,
              unsigned int length) {
  if (String(topic) != topicKendali) {
    return;
  }

  StaticJsonDocument<128> doc;

  DeserializationError error =
    deserializeJson(doc, payload, length);

  if (error) {
    Serial.println("Perintah JSON tidak valid!");
    return;
  }

  if (!doc["led"].is<int>()) {
    Serial.println("Perintah LED tidak valid!");
    return;
  }

  int perintah = doc["led"].as<int>();

  if (perintah != 0 && perintah != 1) {
    Serial.println("Nilai LED harus 0 atau 1!");
    return;
  }

  // Kondisi bahaya selalu mengutamakan proteksi otomatis
  if (kondisiBahaya) {
    Serial.println("Kendali manual ditolak: SUHU PANAS!");
    aturLED();
    return;
  }

  ledManual = (perintah == 1);
  aturLED();

  Serial.print("Kendali manual LED: ");
  Serial.println(ledManual ? "ON" : "OFF");
}

// ========== KONEKSI WIFI NON-BLOCKING ==========
void mulaiWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.println("Mencoba menghubungkan ke Wi-Fi...");
}

// ========== KONEKSI MQTT DENGAN LWT ==========
void cobaMQTT() {
  Serial.println("Mencoba menghubungkan ke MQTT...");

  bool berhasil = client.connect(
    clientId,
    topicStatus,
    1,
    true,
    willMessage
  );

  if (berhasil) {
    Serial.println("MQTT terhubung!");

    client.subscribe(topicKendali);

    // Publikasikan status ONLINE dengan retain
    client.publish(topicStatus, onlineMessage, true);

    Serial.println("Subscribe topik kendali berhasil.");
    Serial.println("Status ONLINE dikirim.");
  } else {
    Serial.print("Koneksi MQTT gagal. Kode: ");
    Serial.println(client.state());
  }
}

// ========== PEMBACAAN SENSOR DAN SAFETY ENGINE ==========
void bacaSensor() {
  float hasilSuhu = dht.readTemperature();
  float hasilKelembapan = dht.readHumidity();

  if (isnan(hasilSuhu) || isnan(hasilKelembapan)) {
    Serial.println("Gagal membaca DHT!");
    return;
  }

  suhu = hasilSuhu;
  kelembapan = hasilKelembapan;

  // Suhu di atas 31 C memicu proteksi otomatis
  if (suhu > BATAS_SUHU) {
    if (!kondisiBahaya) {
      kondisiBahaya = true;
      ledManual = false;
      aturLED();

      Serial.println("PERINGATAN: SUHU PANAS!");

      // Peringatan dikirim ketika bahaya pertama kali terdeteksi
      if (client.connected()) {
        StaticJsonDocument<128> alertDoc;
        alertDoc["status"] = "BAHAYA";
        alertDoc["parameter"] = "OVERHEAT";
        alertDoc["nilai_suhu"] = suhu;

        char pesanAlert[128];
        serializeJson(alertDoc, pesanAlert);

        client.publish(topicAlert, pesanAlert);
        Serial.print("Alert terkirim: ");
        Serial.println(pesanAlert);
      }
    }
  } else {
    // Kondisi kembali aman: reset latch bahaya
    if (kondisiBahaya) {
      Serial.println("Suhu kembali NORMAL.");
    }

    kondisiBahaya = false;
    aturLED();
  }
}

// ========== TELEMETRI JSON ==========
void kirimTelemetri() {
  if (isnan(suhu) || isnan(kelembapan)) {
    return;
  }

  StaticJsonDocument<256> doc;

  doc["node_id"] = clientId;
  doc["suhu"] = suhu;
  doc["kelembapan"] = kelembapan;
  doc["kondisi_suhu"] =
    (suhu > BATAS_SUHU) ? "PANAS" : "NORMAL";
  doc["status_led"] = digitalRead(LEDPIN);
  doc["uptime_detik"] = millis() / 1000UL;
  doc["ip_lokal"] = WiFi.localIP().toString();

  char pesan[256];
  serializeJson(doc, pesan);

  if (client.publish(topicTelemetri, pesan)) {
    Serial.print("Telemetri: ");
    Serial.println(pesan);
  } else {
    Serial.println("Gagal mengirim telemetri.");
  }
}

// ========== SETUP ==========
void setup() {
  Serial.begin(115200);

  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, LOW);

  dht.begin();

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  client.setKeepAlive(30);
  client.setBufferSize(512);

  mulaiWiFi();
}

// ========== LOOP UTAMA ==========
void loop() {
  unsigned long sekarang = millis();

  // Coba Wi-Fi kembali secara berkala
  if (WiFi.status() != WL_CONNECTED) {
    if (sekarang - waktuWiFi >= intervalWiFi) {
      waktuWiFi = sekarang;
      mulaiWiFi();
    }

    return;
  }

  // Coba MQTT kembali tanpa loop tunggu
  if (!client.connected()) {
    if (sekarang - waktuMQTT >= intervalMQTT) {
      waktuMQTT = sekarang;
      cobaMQTT();
    }
  } else {
    client.loop();
  }

  // Sensor diperiksa setiap 1 detik
  if (sekarang - waktuSensor >= intervalSensor) {
    waktuSensor = sekarang;
    bacaSensor();
  }

  // Telemetri dikirim setiap 4 detik
  if (sekarang - waktuTelemetri >= intervalTelemetri) {
    waktuTelemetri = sekarang;

    if (client.connected()) {
      kirimTelemetri();
    }
  }
}