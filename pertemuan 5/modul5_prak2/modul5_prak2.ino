#include <ESP8266WiFi.h>
#include <espnow.h>

// MAC Address Hub / Penerima Pusat
uint8_t hubMacAddress[] = {
0xEC, 0xFA, 0xBC, 0x10, 0x62, 0x42};

// Identitas Node
#define BOARD_ID 1

// Struktur data sensor
typedef struct struct_sensor {
  int id;
  float pembacaan1;
  float pembacaan2;
} struct_sensor;

struct_sensor dataKirim;

unsigned long prevTime = 0;
const unsigned long sendInterval = 3000;

// Callback pengiriman
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {

  Serial.print("Pengiriman Node #");
  Serial.print(BOARD_ID);

  if (sendStatus == 0) {
    Serial.println(" -> Sukses Diterima Hub");
  } else {
    Serial.println(" -> Gagal Sampai");
  }
}

void setup() {

  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Inisialisasi ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Inisialisasi ESP-NOW gagal");
    return;
  }

  // Sebagai Controller / Pengirim
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);

  // Callback pengiriman
  esp_now_register_send_cb(OnDataSent);

  // Daftarkan Hub
  esp_now_add_peer(
    hubMacAddress,
    ESP_NOW_ROLE_SLAVE,
    1,
    NULL,
    0
  );

  Serial.println("Node Sensor #1 Siap!");
}

void loop() {

  unsigned long now = millis();

  if (now - prevTime >= sendInterval) {

    prevTime = now;

    // ID Node
    dataKirim.id = BOARD_ID;

    // Simulasi suhu 25.0 - 34.9 C
    dataKirim.pembacaan1 =
      random(250, 350) / 10.0;

    // Simulasi kelembapan 40.0 - 79.9 %
    dataKirim.pembacaan2 =
      random(400, 800) / 10.0;

    // Kirim data ke Hub
    esp_now_send(
      hubMacAddress,
      (uint8_t *) &dataKirim,
      sizeof(dataKirim)
    );
  }
}