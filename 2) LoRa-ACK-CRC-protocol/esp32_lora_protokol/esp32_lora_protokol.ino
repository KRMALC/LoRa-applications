/*
 * ESP32 — LoRa motor kontrolü, güvenilir protokol (ACK + CRC + sıra numarası + tekrar)
 *
 * Serial Monitor (115200, "Newline") komutları:
 *   F = ileri   B = geri   S = dur   D = durum sor
 *   I = istatistikleri göster        R = istatistikleri sıfırla
 *   T = bağlantı testi (50 paket, tekrar denemesiz → ham paket kaybı)
 *
 * Komut paketi (ESP32 -> STM32): [0xAA] [SEQ] [CMD] [VERI] [CRC8]
 * Onay paketi  (STM32 -> ESP32): [0x55] [SEQ] [DURUM_KODU] [MOTOR_DURUMU] [CRC8]
 * CRC8: polinom 0x07, başlangıç 0x00, SEQ..VERI (3 bayt) üzerinden.
 */

#define E32_RX 16   // ESP32 RX2  <- E32 TXD
#define E32_TX 17   // ESP32 TX2  -> E32 RXD

const uint8_t  PKT_CMD_BASLIK = 0xAA;
const uint8_t  PKT_ACK_BASLIK = 0x55;
const uint8_t  PKT_UZUNLUK    = 5;

const uint8_t  CMD_ILERI = 0x01;
const uint8_t  CMD_GERI  = 0x02;
const uint8_t  CMD_DUR   = 0x03;
const uint8_t  CMD_DURUM = 0x04;

const uint32_t ACK_ZAMAN_ASIMI_MS = 1000;  // 2,4 kbps'de gidiş-dönüş ~0,3-0,5 s sürer
const uint8_t  MAX_DENEME         = 3;     // ilk gönderim + 2 tekrar

uint8_t seq;

struct Istatistik {
  uint32_t komut;        // kullanıcının verdiği komut sayısı
  uint32_t basarili;     // ACK alınan komutlar
  uint32_t basarisiz;    // tüm denemelere rağmen ACK gelmeyen komutlar
  uint32_t gonderim;     // havaya çıkan toplam paket (tekrarlar dahil)
  uint32_t tekrar;       // tekrar gönderim sayısı
  uint32_t crc_hata;     // bozuk gelen ACK
  uint32_t rtt_toplam, rtt_min, rtt_max;
} ist;

uint8_t crc8(const uint8_t *d, size_t n) {
  uint8_t c = 0x00;
  while (n--) {
    c ^= *d++;
    for (uint8_t i = 0; i < 8; i++)
      c = (c & 0x80) ? (uint8_t)((c << 1) ^ 0x07) : (uint8_t)(c << 1);
  }
  return c;
}

const char *motor_adi(uint8_t d) {
  switch (d) {
    case 0: return "DURUYOR";
    case 1: return "ILERI";
    case 2: return "GERI";
    default: return "?";
  }
}

void istatistik_sifirla() {
  memset(&ist, 0, sizeof(ist));
  ist.rtt_min = UINT32_MAX;
}

// Beklenen sıra numaralı ACK'yi bekler. Gelirse durum kodunu ve motor durumunu döndürür.
bool ack_bekle(uint8_t beklenen_seq, uint8_t &durum_kodu, uint8_t &motor) {
  uint8_t buf[PKT_UZUNLUK];
  uint8_t idx = 0;
  uint32_t t0 = millis();

  while (millis() - t0 < ACK_ZAMAN_ASIMI_MS) {
    while (Serial2.available()) {
      uint8_t b = Serial2.read();
      if (idx == 0 && b != PKT_ACK_BASLIK) continue;   // başlığı bekle
      buf[idx++] = b;
      if (idx < PKT_UZUNLUK) continue;
      idx = 0;

      if (crc8(&buf[1], 3) != buf[4]) { ist.crc_hata++; continue; }
      if (buf[1] != beklenen_seq) continue;            // eski bir ACK, yok say

      durum_kodu = buf[2];
      motor      = buf[3];
      return true;
    }
    delay(1);
  }
  return false;
}

// Komutu gönderir, ACK gelmezse tekrar dener. Başarılıysa true döner.
bool komut_gonder(uint8_t cmd, uint8_t veri, uint8_t deneme_sayisi, bool sessiz) {
  seq++;
  uint8_t p[PKT_UZUNLUK] = { PKT_CMD_BASLIK, seq, cmd, veri, 0 };
  p[4] = crc8(&p[1], 3);
  ist.komut++;

  for (uint8_t deneme = 1; deneme <= deneme_sayisi; deneme++) {
    while (Serial2.available()) Serial2.read();   // eski artıkları temizle

    uint32_t t0 = millis();
    Serial2.write(p, PKT_UZUNLUK);
    ist.gonderim++;
    if (deneme > 1) ist.tekrar++;

    uint8_t durum_kodu = 0, motor = 0;
    if (ack_bekle(seq, durum_kodu, motor)) {
      uint32_t rtt = millis() - t0;
      ist.basarili++;
      ist.rtt_toplam += rtt;
      if (rtt < ist.rtt_min) ist.rtt_min = rtt;
      if (rtt > ist.rtt_max) ist.rtt_max = rtt;

      if (!sessiz) {
        Serial.printf("[ACK] seq=%u  %s  motor=%s  sure=%lu ms  deneme=%u\n",
                      seq, durum_kodu == 0 ? "OK" : "BILINMEYEN KOMUT",
                      motor_adi(motor), (unsigned long)rtt, deneme);
      }
      return true;
    }

    if (!sessiz) Serial.printf("[ZAMAN ASIMI] seq=%u  deneme %u/%u\n", seq, deneme, deneme_sayisi);
  }

  ist.basarisiz++;
  if (!sessiz) Serial.printf("[BASARISIZ] seq=%u  komut karsiya ulasmadi!\n", seq);
  return false;
}

void istatistik_yaz() {
  Serial.println("------ ISTATISTIK ------");
  Serial.printf("Komut            : %lu\n", (unsigned long)ist.komut);
  Serial.printf("Basarili (ACK)   : %lu\n", (unsigned long)ist.basarili);
  Serial.printf("Basarisiz        : %lu\n", (unsigned long)ist.basarisiz);
  Serial.printf("Toplam gonderim  : %lu (tekrar: %lu)\n",
                (unsigned long)ist.gonderim, (unsigned long)ist.tekrar);
  Serial.printf("Bozuk ACK (CRC)  : %lu\n", (unsigned long)ist.crc_hata);
  if (ist.gonderim > 0) {
    float kayip = 100.0f * (ist.gonderim - ist.basarili) / ist.gonderim;
    Serial.printf("Paket kaybi      : %%%.1f (gidis veya donus)\n", kayip);
  }
  if (ist.basarili > 0) {
    Serial.printf("Gidis-donus (ms) : ort %lu, min %lu, max %lu\n",
                  (unsigned long)(ist.rtt_toplam / ist.basarili),
                  (unsigned long)ist.rtt_min, (unsigned long)ist.rtt_max);
  }
  Serial.println("------------------------");
}

void baglanti_testi(uint16_t adet) {
  Serial.printf("Baglanti testi: %u paket, tekrar denemesi yok...\n", adet);
  istatistik_sifirla();
  for (uint16_t i = 0; i < adet; i++) {
    komut_gonder(CMD_DURUM, 0, 1, true);
    if ((i + 1) % 10 == 0) Serial.printf("  %u/%u\n", i + 1, adet);
    delay(200);
  }
  istatistik_yaz();
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, E32_RX, E32_TX);
  delay(500);

  seq = (uint8_t)esp_random();   // her açılışta farklı sıra numarasıyla başla
  istatistik_sifirla();

  Serial.println();
  Serial.println("ESP32 hazir.");
  Serial.println("F=ileri  B=geri  S=dur  D=durum  I=istatistik  R=sifirla  T=baglanti testi");
}

void loop() {
  while (Serial.available()) {
    char c = toupper(Serial.read());
    switch (c) {
      case 'F': komut_gonder(CMD_ILERI, 0, MAX_DENEME, false); break;
      case 'B': komut_gonder(CMD_GERI,  0, MAX_DENEME, false); break;
      case 'S': komut_gonder(CMD_DUR,   0, MAX_DENEME, false); break;
      case 'D': komut_gonder(CMD_DURUM, 0, MAX_DENEME, false); break;
      case 'I': istatistik_yaz(); break;
      case 'R': istatistik_sifirla(); Serial.println("Istatistik sifirlandi."); break;
      case 'T': baglanti_testi(50); break;
      case '\r': case '\n': case ' ': break;
      default: Serial.printf("Bilinmeyen komut: %c\n", c); break;
    }
  }
}
