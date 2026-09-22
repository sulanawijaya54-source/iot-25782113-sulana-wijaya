/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://docs.arduino.cc/hardware/

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  https://docs.arduino.cc/built-in-examples/basics/Blink/
*/

// the setup function runs once when you press reset or power the board
Latihan 1
ADC NodeMCU memiliki resolusi 10-bit, sehingga jumlah nilai yang dapat dihasilkan adalah:
2¹⁰ = 1024 nilai
Rentangnya mulai dari 0 sampai 1023.
Latihan 2
Persentase intensitas cahaya dapat dihitung dengan rumus:
Persentase = (nilai ADC / 1023) × 100
Kode:
C++
int ldrValue = analogRead(ldrPin);
int persen = (ldrValue * 100) / 1023;

Serial.print("Intensitas Cahaya: ");
Serial.print(persen);
Serial.println("%");
Latihan 3
Jika Relay bertipe Active-Low, maka:
C++
digitalWrite(relayPin, HIGH);
akan membuat Relay mati, sehingga aktuator seperti lampu tidak aktif.
Latihan 4
LED D1 menyala hanya jika nilai ADC LDR di bawah 200:
C++
int ldrValue = analogRead(A0);

if (ldrValue < 200) {
  digitalWrite(D1, HIGH);
} else {
  digitalWrite(D1, LOW);
}
Artinya: jika kondisi sangat gelap → LED menyala; jika tidak → LED mati.