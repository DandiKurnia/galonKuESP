#include <WiFi.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

const char* ssid = "ASUS";
const char* password = "12345678";

void setup() {
  Serial.begin(115200);

  // INIT TFT
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);

  tft.setCursor(10, 20);
  tft.println("Start...");
  delay(1000);

  // WIFI
  tft.setCursor(10, 50);
  tft.println("Connecting WiFi...");

  WiFi.begin(ssid, password);

  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 20) {
    delay(500);
    Serial.print(".");
    tft.print(".");
    retry++;
  }

  // 🔥 WAJIB: REINIT TFT
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Terhubung!");

    tft.setCursor(10, 20);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("WiFi Connected!");

    tft.setCursor(10, 60);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.print("IP: ");
    tft.println(WiFi.localIP());

  } else {
    Serial.println("\nGagal konek WiFi!");

    tft.setCursor(10, 20);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.println("WiFi Failed!");
  }
}

void loop() {}