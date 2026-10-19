#include <Arduino.h>

// ESP32 <-> E32-433T20D köprüsü
// Serial Monitor'den yazılanı E32'ye yollar, E32'den geleni ekrana yazar.

#define E32_RX 16 // ESP32 RX2  <- E32 TXD
#define E32_TX 17 // ESP32 TX2  -> E32 RXD

void setup() {
  Serial.begin(115200); // bilgisayar (Serial Monitor)
  Serial2.begin(9600, SERIAL_8N1, E32_RX, E32_TX); // E32
  delay(500);                                      // E32'nin açılışını bekle
  Serial.println("ESP32 hazir. F (ileri), B (geri), S (dur) gonder.");
}

void loop() {
  // Bilgisayardan gelen -> E32 -> STM32
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\r' || c == '\n')
      continue; // satır sonlarını gönderme
    Serial2.write(c);
    Serial.print("[GONDERILDI] ");
    Serial.println(c);
  }

  // STM32 -> E32 -> ESP32 -> ekrana
  while (Serial2.available()) {
    Serial.write(Serial2.read());
  }
}