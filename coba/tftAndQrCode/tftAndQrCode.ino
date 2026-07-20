#include <TFT_eSPI.h>
#include <SPI.h>
#include <ricmoo_qrcode.h>

TFT_eSPI tft = TFT_eSPI();

void drawQRCode(const char *text) {
  const uint8_t version = 3;
  const uint8_t border = 2;
  const int scale = 6;

  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(version)];
  qrcode_initText(&qrcode, qrcodeData, version, ECC_LOW, text);

  int qrSize = (qrcode.size + border * 2) * scale;
  int xOffset = (tft.width() - qrSize) / 2;
  int yOffset = (tft.height() - qrSize) / 2;

  tft.fillScreen(TFT_WHITE);

  for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        tft.fillRect(
          xOffset + (x + border) * scale,
          yOffset + (y + border) * scale,
          scale,
          scale,
          TFT_BLACK
        );
      }
    }
  }

  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(20, tft.height() - 30);
  tft.println("Dummy QR");
}

void setup() {
  tft.init();
  tft.setRotation(1);
  drawQRCode("https://example.com/dummy-qr");
}

void loop() {
}