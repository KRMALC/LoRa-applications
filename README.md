# LoRa Applications

STM32, ESP32 ve EBYTE E32 LoRa modülleriyle geliştirdiğim kablosuz haberleşme uygulamaları.

Her uygulama kendi klasöründe duruyor. Ayrıntılar (malzemeler, bağlantı şemaları, kurulum ve sorun giderme) için ilgili klasöre girip oradaki README dosyasına bakabilirsiniz.

## Uygulamalar

| # | Uygulama | Açıklama |
|---|---|---|
| 1 | [PC-and-stm32-comm-by-LoRa](PC-and-stm32-comm-by-LoRa/) | Bilgisayardan (USB-TTL + Docklight) LoRa üzerinden gönderilen `F` / `B` / `S` komutlarıyla STM32F407'ye bağlı bir DC motoru ileri, geri sürme ve durdurma |
| 2 | [esp32-and-stm32-comm-by-LoRa](esp32-and-stm32-comm-by-LoRa/) | 1. projenin devamı: komutları gönderen taraf USB-TTL yerine **ESP32**. STM32 tarafı, kodu ve bağlantıları aynı kalıyor |
| 3 | [esp32-stm32-LoRa-reliable-protocol](esp32-stm32-LoRa-reliable-protocol/) | 2. projenin devamı: tek harf yerine başlık, sıra numarası (SEQ), komut, veri ve **CRC-8** içeren 5 baytlık paketler. STM32 her komutu **ACK** ile onaylıyor, onay gelmezse ESP32 tekrar gönderiyor. Donanım ve bağlantılar aynı |

## Ortak Donanım ve Araçlar

| | |
|---|---|
| **LoRa modülü** | EBYTE E32-433T20D (433 MHz, UART arayüzlü), TX433-JKD anten |
| **Alıcı taraf** | STM32F407 geliştirme kartı + L298N motor sürücü + DC motor |
| **Gönderici taraf** | Bilgisayar + USB-TTL (1. proje) veya ESP32 DevKit (2. ve 3. proje) |
| **Yazılım** | STM32CubeIDE (HAL), PlatformIO (ESP32), Docklight / Serial Monitor |

## Bir Projeyi Açmak

Repoyu indir:
```
git clone git@github.com:KRMALC/LoRa-applications.git
```

- **STM32 projeleri:** STM32CubeIDE'de **File → Import → General → Existing Projects into Workspace** seç, ilgili proje klasörünü göster, derle ve karta yükle.
- **ESP32 projesi:** VS Code + PlatformIO ile ESP32 klasörünü aç, derle ve karta yükle.

Derleme çıktıları (`Debug/` vb.) repoda tutulmuyor; ilk derlemede otomatik oluşur.
