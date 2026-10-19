# LoRa Applications

STM32 mikrodenetleyiciler ve EBYTE E32 LoRa modülleriyle geliştirdiğim kablosuz haberleşme uygulamaları.

Her uygulama kendi klasöründe, bağımsız bir STM32CubeIDE projesi olarak duruyor. Ayrıntılar için ilgili klasöre girip oradaki README dosyasına bakabilirsiniz.

## Uygulamalar

| Uygulama | Açıklama |
|---|---|
| [LoRa-and-PC](LoRa-and-PC/) | Bilgisayardan LoRa üzerinden gönderilen komutlarla STM32'ye bağlı bir DC motoru ileri/geri sürme ve durdurma |

## Ortak Donanım ve Araçlar

- **Kart:** STM32F407G-DISC1 (STM32F407VGT6)
- **LoRa modülü:** EBYTE E32 serisi (UART arayüzlü)
- **Geliştirme ortamı:** STM32CubeIDE, HAL kütüphanesi

## Bir Projeyi Açmak

1. Repoyu indir:
   ```
   git clone git@github.com:KRMALC/LoRa-applications.git
   ```
2. STM32CubeIDE'de **File → Import → General → Existing Projects into Workspace** seç.
3. İstediğin uygulamanın klasörünü göster (ör. `LoRa-and-PC`), derle ve karta yükle.

`Debug/` klasörü ve derleme çıktıları repoda tutulmuyor; ilk derlemede otomatik oluşur.
