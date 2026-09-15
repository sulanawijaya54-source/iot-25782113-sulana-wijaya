const int buttonPin = 4;
const int ledPin = 5;

int buttonState = 0;
int lastButtonState = LOW; // Menyimpan status tombol sebelumnya
bool ledStatus = false;     // Menyimpan status LED (true = ON, false = OFF)

void setup() {
  Serial.begin(115200);
  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  buttonState = digitalRead(buttonPin);

  // Deteksi ketika tombol BARU SAJA ditekan (perubahan dari LOW ke HIGH)
  if (buttonState == HIGH && lastButtonState == LOW) {
    ledStatus = !ledStatus; // Membalikkan status LED (ON jadi OFF, OFF jadi ON)
    digitalWrite(ledPin, ledStatus ? HIGH : LOW);

    if (ledStatus) {
      Serial.println("Tombol ditekan! -> LED ON");
    } else {
      Serial.println("Tombol ditekan! -> LED OFF");
    }

    delay(50); // Debouncing singkat untuk menghindari pembacaan ganda
  }

  // Simpan status tombol saat ini untuk iterasi berikutnya
  lastButtonState = buttonState;
}