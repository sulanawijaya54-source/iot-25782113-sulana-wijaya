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
Jika tombol Active-High/pull-down dipasang pada D8 (GPIO15) dan ditekan saat NodeMCU dinyalakan, GPIO15 menjadi HIGH. Hal ini dapat menyebabkan NodeMCU gagal booting, karena GPIO15 harus LOW saat proses boot.
Latihan 2
Ubah dua delay() menjadi:
C++
delay(200);
delay(800);
LED menyala 200 ms dan mati 800 ms.
Latihan 3
LED tidak menyala karena dipasang terbalik, sehingga arus tidak mengalir pada arah yang benar. Program tidak mengalami error kompilasi karena masalahnya terdapat pada rangkaian hardware.
Latihan 4
Ubah kondisi menjadi:
C++
if (buttonState == LOW) {
  digitalWrite(ledPin, HIGH);
} else {
  digitalWrite(ledPin, LOW);
}
Artinya, tombol tidak ditekan → LED menyala, dan tombol ditekan → LED mati.