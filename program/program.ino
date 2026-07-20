// =============================================================================
// GalonKu - Sistem Dispenser Air Galon Otomatis
// Board   : ESP32-S3 DevKit
// Display : TFT LCD ILI9341 / ST7789 (320x240) HORIZONTAL via TFT_eSPI
// Flow    : YF-S201 (1 liter = 450 pulsa)
// Libs    : TFT_eSPI, qrcode (ricmoo), WiFi, HTTPClient, ArduinoJson
// =============================================================================

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <ricmoo_qrcode.h>
#include <WiFiClientSecure.h>

// =============================================================================
// KONFIGURASI
// =============================================================================

const char* WIFI_SSID     = "ASUS";
const char* WIFI_PASSWORD = "12345678";

const char* BASE_URL      = "https://api.galonku.my.id";
const char* DEVICE_CODE   = "DEV-44";
const char* DEVICE_TOKEN  = "dtkn_6f8ec5ea186260fdcb8c894d4ec73191b4398ee2b5f83623a7ecd68b1d9312ea";

const unsigned long POLL_INTERVAL    = 3000;
const int           HTTP_TIMEOUT_MS  = 5000;

// Flow sensor
#define PIN_FLOW_SENSOR 17
const float TARGET_LITERS = 1.0;
const unsigned long PULSES_PER_LITER = 450;
const unsigned long TARGET_PULSES = (unsigned long)(TARGET_LITERS * PULSES_PER_LITER);
const unsigned long MAX_FILL_TIMEOUT_MS = 30000; // safety timeout 30 detik

// Hardware
#define PIN_IR_SENSOR  18
#define PIN_BUTTON     19
#define PIN_RELAY      20

#define TFT_W 320
#define TFT_H 240

// =============================================================================
// WARNA TFT (RGB565)
// =============================================================================

#define CLR_WHITE       0xFFFF
#define CLR_BLACK       0x0000
#define CLR_RED         0xF800
#define CLR_GREEN       0x07E0
#define CLR_BLUE        0x001F
#define CLR_LIGHT_BLUE   0xAEDC
#define CLR_LIGHT_GREEN  0x9FE0
#define CLR_DARK_GREEN   0x03E0
#define CLR_ORANGE       0xFD20
#define CLR_GRAY         0xC618
#define CLR_DARK_GRAY    0x7BEF
#define CLR_YELLOW       0xFFE0
#define CLR_CYAN         0x07FF

// =============================================================================
// STATE MACHINE
// =============================================================================

enum AppState {
  STATE_WIFI_CONNECTING,
  STATE_IDLE,
  STATE_PREPARE_FILL,
  STATE_CONFIRM_FILL,
  STATE_PROCESSING,
  STATE_CHECK_NEXT,
  STATE_DONE,
  STATE_ERROR
};

// =============================================================================
// VARIABEL GLOBAL
// =============================================================================

TFT_eSPI tft = TFT_eSPI();

AppState currentState   = STATE_WIFI_CONNECTING;
AppState lastDrawnState = (AppState)(-1);

unsigned long lastPollTime     = 0;
unsigned long fillingStart      = 0;
bool          wifiWasConnected  = false;

int           totalGallons      = 0;
int           filledGallons     = 0;
bool          isPaused          = false;
unsigned long remainingTimeMs   = 0;

char qrContent[32];

// Flow counter
volatile unsigned long flowPulseCount = 0;
unsigned long lastPulseCheckTime = 0;
unsigned long lastPulseCount = 0;

// =============================================================================
// DEKLARASI FUNGSI
// =============================================================================

void drawWiFiConnectingScreen();
void drawQRCodeScreen(const char* content);
void drawScannedScreen();
void drawPrepareFillScreen(int current, int total);
void drawConfirmFillScreen(int current, int total);
void drawGalonLiftedScreen();
void drawProcessingScreen(int current, int total);
void drawCheckNextScreen(int filled, int total);
void drawDoneScreen();
void drawErrorScreen(const char* message);
void drawProgressBar(int x, int y, int w, int h, int percent, uint16_t fillColor, uint16_t bgColor);
void drawCenteredText(const char* text, int y, uint8_t size, uint16_t color, uint16_t bg);
String getDeviceStatus();
bool   updateDeviceStatus(const char* status);
void   connectWiFi();
void IRAM_ATTR flowPulseISR();
void startFilling();
void stopFilling();
void resetFlowCounter();

// =============================================================================
// SETUP
// =============================================================================

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== GalonKu Dispenser v3.0 ===");
  Serial.printf("Device Code : %s\n", DEVICE_CODE);

  pinMode(PIN_IR_SENSOR, INPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, LOW);

  pinMode(PIN_FLOW_SENSOR, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_FLOW_SENSOR), flowPulseISR, FALLING);

  Serial.println("[HW] IR Sensor  : GPIO 18");
  Serial.println("[HW] Button     : GPIO 19 (PULLUP)");
  Serial.println("[HW] Relay/Pompa: GPIO 20");
  Serial.println("[HW] Flow Sensor: GPIO 17");

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(CLR_BLACK);
  Serial.println("[TFT] Initialized - 320x240 Horizontal");

  snprintf(qrContent, sizeof(qrContent), "%s", DEVICE_CODE);

  currentState = STATE_WIFI_CONNECTING;
  drawWiFiConnectingScreen();
  connectWiFi();
}

// =============================================================================
// LOOP UTAMA
// =============================================================================

void loop() {
  unsigned long now = millis();

  if (WiFi.status() != WL_CONNECTED) {
    if (wifiWasConnected) {
      Serial.println("[WiFi] Terputus! Reconnecting...");
      wifiWasConnected = false;
      currentState = STATE_WIFI_CONNECTING;
      digitalWrite(PIN_RELAY, LOW);
    }
    if (lastDrawnState != STATE_WIFI_CONNECTING) {
      drawWiFiConnectingScreen();
      lastDrawnState = STATE_WIFI_CONNECTING;
    }
    connectWiFi();
    return;
  }
  wifiWasConnected = true;

  switch (currentState) {

    case STATE_IDLE: {
      if (lastDrawnState != STATE_IDLE) {
        drawQRCodeScreen(qrContent);
        lastDrawnState = STATE_IDLE;
        Serial.println("[STATE] IDLE");
      }

      if (now - lastPollTime >= POLL_INTERVAL) {
        lastPollTime = now;
        String status = getDeviceStatus();
        Serial.printf("[POLL] status = %s\n", status.c_str());

        if (status == "SCANNED") {
          filledGallons   = 0;
          isPaused        = false;
          remainingTimeMs = 0;
          Serial.printf("[INFO] totalGallons = %d\n", totalGallons);
          currentState    = STATE_PREPARE_FILL;
          lastDrawnState  = (AppState)(-1);
        } else if (status == "PROCESSING") {
          Serial.println("[RECOVERY] Stale PROCESSING -> kirim DONE");
          updateDeviceStatus("DONE");
          currentState    = STATE_IDLE;
          lastDrawnState  = (AppState)(-1);
        } else if (status == "ERROR" || status == "FAILED") {
          currentState = STATE_ERROR;
        } else if (status == "CANCELLED") {
          Serial.println("[INFO] Transaksi dibatalkan");
        }
      }
      break;
    }

    case STATE_PREPARE_FILL: {
      if (lastDrawnState != STATE_PREPARE_FILL) {
        drawPrepareFillScreen(filledGallons + 1, totalGallons);
        lastDrawnState = STATE_PREPARE_FILL;
        Serial.printf("[STATE] PREPARE_FILL - Galon %d dari %d\n", filledGallons + 1, totalGallons);
      }

      if (digitalRead(PIN_IR_SENSOR) == LOW) {
        Serial.println("[IR] Galon terdeteksi -> CONFIRM_FILL");
        currentState   = STATE_CONFIRM_FILL;
        lastDrawnState = (AppState)(-1);
      }
      break;
    }

    case STATE_CONFIRM_FILL: {
      if (lastDrawnState != STATE_CONFIRM_FILL) {
        drawConfirmFillScreen(filledGallons + 1, totalGallons);
        lastDrawnState = STATE_CONFIRM_FILL;
        Serial.println("[STATE] CONFIRM_FILL - Menunggu tombol...");
      }

      bool irNow  = digitalRead(PIN_IR_SENSOR);
      bool btnNow = digitalRead(PIN_BUTTON);

      if (irNow == HIGH) {
        Serial.println("[IR] Galon diangkat sebelum konfirmasi -> PREPARE_FILL");
        isPaused = false;
        remainingTimeMs = 0;
        currentState   = STATE_PREPARE_FILL;
        lastDrawnState = (AppState)(-1);
        break;
      }

      if (btnNow == LOW && irNow == LOW) {
        Serial.println("[BTN] Tombol ditekan, IR valid -> Mulai isi!");
        digitalWrite(PIN_RELAY, HIGH);

        if (isPaused && remainingTimeMs > 0) {
          fillingStart = millis() - (MAX_FILL_TIMEOUT_MS - remainingTimeMs);
          Serial.printf("[RESUME] Melanjutkan sisa timeout %lums\n", remainingTimeMs);
          isPaused = false;
          remainingTimeMs = 0;
        } else {
          fillingStart = millis();
        }

        resetFlowCounter();

        if (filledGallons == 0) {
          Serial.println("[API] Mengirim PROCESSING...");
          if (updateDeviceStatus("PROCESSING")) {
            Serial.println("[API] PROCESSING terkirim");
          } else {
            Serial.println("[WARN] Gagal kirim PROCESSING");
          }
        }

        currentState   = STATE_PROCESSING;
        lastDrawnState = (AppState)(-1);
      }
      break;
    }

    case STATE_PROCESSING: {
      unsigned long elapsed = now - fillingStart;

      if (digitalRead(PIN_IR_SENSOR) == HIGH) {
        digitalWrite(PIN_RELAY, LOW);
        remainingTimeMs = (elapsed < MAX_FILL_TIMEOUT_MS) ? (MAX_FILL_TIMEOUT_MS - elapsed) : 0;
        isPaused = true;
        Serial.printf("[SAFETY] Galon diangkat! Sisa timeout: %lums\n", remainingTimeMs);
        drawGalonLiftedScreen();
        delay(2000);
        currentState   = STATE_CONFIRM_FILL;
        lastDrawnState = (AppState)(-1);
        break;
      }

      if (lastDrawnState != STATE_PROCESSING) {
        drawProcessingScreen(filledGallons + 1, totalGallons);
        lastDrawnState = STATE_PROCESSING;
        lastPulseCheckTime = millis();
        lastPulseCount = flowPulseCount;
      } else {
        unsigned long pulseCount = flowPulseCount;
        unsigned long pulses = pulseCount;
        int progress = (int)((pulses * 100UL) / TARGET_PULSES);
        if (progress > 100) progress = 100;

        drawProgressBar(10, 148, 300, 22, progress, CLR_BLUE, CLR_DARK_GRAY);

        float liters = (float)pulses / (float)PULSES_PER_LITER;
        char text[32];
        snprintf(text, sizeof(text), "Terkirim: %.2f L", liters);
        tft.setTextColor(CLR_WHITE, CLR_LIGHT_BLUE);
        tft.setTextSize(1);
        int tx = (TFT_W - tft.textWidth(text, 1)) / 2;
        tft.setCursor(tx, 178);
        tft.print(text);
      }

      if (flowPulseCount >= TARGET_PULSES) {
        digitalWrite(PIN_RELAY, LOW);
        filledGallons++;
        Serial.printf("[DONE] Target 1 liter tercapai. Pulses=%lu\n", flowPulseCount);
        currentState   = STATE_CHECK_NEXT;
        lastDrawnState = (AppState)(-1);
      }

      if (elapsed >= MAX_FILL_TIMEOUT_MS) {
        digitalWrite(PIN_RELAY, LOW);
        Serial.println("[WARN] Timeout tercapai sebelum 1 liter");
        currentState   = STATE_ERROR;
        lastDrawnState = (AppState)(-1);
      }

      delay(200);
      break;
    }

    case STATE_CHECK_NEXT: {
      if (lastDrawnState != STATE_CHECK_NEXT) {
        drawCheckNextScreen(filledGallons, totalGallons);
        lastDrawnState = STATE_CHECK_NEXT;
        Serial.printf("[STATE] CHECK_NEXT (%d/%d)\n", filledGallons, totalGallons);
      }

      if (filledGallons < totalGallons) {
        if (digitalRead(PIN_IR_SENSOR) == HIGH) {
          Serial.println("[IR] Galon penuh diangkat -> Menyiapkan galon berikutnya");
          delay(1000);
          currentState   = STATE_PREPARE_FILL;
          lastDrawnState = (AppState)(-1);
        }
      } else {
        Serial.println("[API] Semua galon selesai, kirim DONE...");
        if (updateDeviceStatus("DONE")) {
          Serial.println("[API] DONE terkirim");
        } else {
          Serial.println("[WARN] Gagal kirim DONE");
        }
        currentState   = STATE_DONE;
        lastDrawnState = (AppState)(-1);
      }
      break;
    }

    case STATE_DONE: {
      if (lastDrawnState != STATE_DONE) {
        drawDoneScreen();
        lastDrawnState = STATE_DONE;
        Serial.println("[STATE] DONE");
      }

      delay(5000);

      totalGallons    = 0;
      filledGallons   = 0;
      remainingTimeMs = 0;
      isPaused        = false;

      currentState   = STATE_IDLE;
      lastDrawnState = (AppState)(-1);
      Serial.println("[STATE] Kembali ke IDLE");
      break;
    }

    case STATE_ERROR: {
      if (lastDrawnState != STATE_ERROR) {
        digitalWrite(PIN_RELAY, LOW);
        drawErrorScreen("Hubungi operator");
        lastDrawnState = STATE_ERROR;
        Serial.println("[STATE] ERROR - Pompa dimatikan paksa");
      }

      if (now - lastPollTime >= 5000) {
        lastPollTime = now;
        String status = getDeviceStatus();
        Serial.printf("[POLL-ERROR] status = %s\n", status.c_str());
        if (status == "SUCCESS") {
          currentState   = STATE_IDLE;
          lastDrawnState = (AppState)(-1);
          Serial.println("[RECOVERY] Kembali ke IDLE");
        }
      }
      break;
    }

    case STATE_WIFI_CONNECTING:
    default:
      break;
  }
}

// =============================================================================
// FLOW SENSOR ISR
// =============================================================================

void IRAM_ATTR flowPulseISR() {
  flowPulseCount++;
}

void resetFlowCounter() {
  noInterrupts();
  flowPulseCount = 0;
  interrupts();
}

void startFilling() {
  resetFlowCounter();
  digitalWrite(PIN_RELAY, HIGH);
  fillingStart = millis();
}

void stopFilling() {
  digitalWrite(PIN_RELAY, LOW);
}

// =============================================================================
// WIFI
// =============================================================================

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    wifiWasConnected = true;
    currentState   = STATE_IDLE;
    lastDrawnState = (AppState)(-1);
    Serial.printf("[WiFi] IP: %s\n", WiFi.localIP().toString().c_str());
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
    tft.fillRect(80, 195, 160, 16, CLR_DARK_GRAY);
    for (int d = 0; d <= (attempts % 8); d++) {
      tft.fillCircle(92 + d * 18, 203, 5, CLR_YELLOW);
    }
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    wifiWasConnected = true;
    currentState   = STATE_IDLE;
    lastDrawnState = (AppState)(-1);
    tft.fillScreen(CLR_DARK_GRAY);
    drawCenteredText("WiFi Terhubung!", 100, 2, CLR_GREEN, CLR_DARK_GRAY);
    char ipStr[32];
    snprintf(ipStr, sizeof(ipStr), "IP: %s", WiFi.localIP().toString().c_str());
    drawCenteredText(ipStr, 130, 1, CLR_WHITE, CLR_DARK_GRAY);
    delay(1500);
  } else {
    Serial.println("[WiFi] Gagal.");
    drawCenteredText("Gagal! Coba lagi...", 130, 1, CLR_RED, CLR_DARK_GRAY);
    delay(3000);
  }
}

// =============================================================================
// HTTP GET
// =============================================================================

String getDeviceStatus() {
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  char url[256];
  snprintf(url, sizeof(url), "%s/devices/code/%s/status", BASE_URL, DEVICE_CODE);

  http.begin(client, url);
  http.setTimeout(HTTP_TIMEOUT_MS);
  http.addHeader("x-device-code", DEVICE_CODE);
  http.addHeader("x-device-token", DEVICE_TOKEN);

  int code = http.GET();
  String result = "";

  if (code == HTTP_CODE_OK) {
    String payload = http.getString();
    Serial.print("[HTTP GET] "); Serial.println(payload);

    StaticJsonDocument<512> doc;
    if (!deserializeJson(doc, payload)) {
      const char* s = doc["data"]["qr_status"];
      if (s) result = String(s);

      if (result == "SCANNED") {
        int tg = doc["data"]["total_galon"] | 0;
        totalGallons = (tg > 0) ? tg : 1;
        Serial.printf("[JSON] total_galon = %d\n", totalGallons);
      }
    }
  } else {
    Serial.print("[HTTP GET] Error: ");
    Serial.println(code < 0 ? http.errorToString(code) : String(code));
  }

  http.end();
  return result;
}

// =============================================================================
// HTTP PATCH
// =============================================================================

bool updateDeviceStatus(const char* status) {
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  char url[128];
  snprintf(url, sizeof(url), "%s/devices/code", BASE_URL);

  http.begin(client, url);
  http.setTimeout(HTTP_TIMEOUT_MS);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("x-device-code", DEVICE_CODE);
  http.addHeader("x-device-token", DEVICE_TOKEN);

  StaticJsonDocument<128> doc;
  doc["code"]   = DEVICE_CODE;
  doc["status"] = status;
  String body;
  serializeJson(doc, body);

  Serial.print("[HTTP PATCH] "); Serial.println(body);
  int code = http.PATCH(body);
  bool ok = (code == HTTP_CODE_OK || code == HTTP_CODE_CREATED);
  if (!ok) {
    Serial.print("[HTTP PATCH] Error: ");
    Serial.println(code < 0 ? http.errorToString(code) : String(code));
  }

  http.end();
  return ok;
}

// =============================================================================
// UI HELPERS
// =============================================================================

void drawCenteredText(const char* text, int y, uint8_t size, uint16_t color, uint16_t bg) {
  tft.setTextSize(size);
  tft.setTextColor(color, bg);
  int x = (TFT_W - tft.textWidth(text, size)) / 2;
  if (x < 2) x = 2;
  tft.setCursor(x, y);
  tft.print(text);
}

void drawProgressBar(int x, int y, int w, int h, int percent, uint16_t fillColor, uint16_t bgColor) {
  if (percent < 0) percent = 0;
  if (percent > 100) percent = 100;
  int fillW = (w * percent) / 100;
  tft.drawRect(x, y, w, h, CLR_WHITE);
  tft.fillRect(x + 1, y + 1, w - 2, h - 2, bgColor);
  if (fillW > 2) tft.fillRect(x + 1, y + 1, fillW - 2, h - 2, fillColor);
}

// =============================================================================
// SCREEN FUNCTIONS
// =============================================================================

void drawWiFiConnectingScreen() {
  tft.fillScreen(CLR_DARK_GRAY);
  tft.fillRect(0, 0, TFT_W, 40, CLR_BLUE);
  drawCenteredText("GalonKu", 10, 2, CLR_WHITE, CLR_BLUE);

  int cx = TFT_W / 2, cy = 115;
  tft.drawCircle(cx, cy, 38, CLR_YELLOW);
  tft.drawCircle(cx, cy, 26, CLR_YELLOW);
  tft.drawCircle(cx, cy, 14, CLR_YELLOW);
  tft.fillCircle(cx, cy, 5, CLR_YELLOW);

  drawCenteredText("Menghubungkan WiFi...", 162, 1, CLR_WHITE, CLR_DARK_GRAY);
  char ssidStr[64];
  snprintf(ssidStr, sizeof(ssidStr), "SSID: %.30s", WIFI_SSID);
  drawCenteredText(ssidStr, 178, 1, CLR_GRAY, CLR_DARK_GRAY);
  tft.fillRect(80, 195, 160, 16, CLR_DARK_GRAY);
}

void drawQRCodeScreen(const char* content) {
  tft.fillScreen(CLR_WHITE);

  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(3)];
  qrcode_initText(&qrcode, qrcodeData, 3, ECC_LOW, content);

  int modules    = qrcode.size;
  int moduleSize = 200 / modules;
  int actualSize = moduleSize * modules;
  int startX     = 8;
  int startY     = (TFT_H - actualSize) / 2;

  tft.fillRect(startX - 4, startY - 4, actualSize + 8, actualSize + 8, CLR_WHITE);
  for (int y = 0; y < modules; y++) {
    for (int x = 0; x < modules; x++) {
      uint16_t color = qrcode_getModule(&qrcode, x, y) ? CLR_BLACK : CLR_WHITE;
      tft.fillRect(startX + x * moduleSize, startY + y * moduleSize, moduleSize, moduleSize, color);
    }
  }

  int rx = startX + actualSize + 14;
  int rw = TFT_W - rx - 4;

  tft.setTextSize(2);
  tft.setTextColor(CLR_BLUE, CLR_WHITE);
  tft.setCursor(rx, 22);
  tft.print("GalonKu");
  tft.drawFastHLine(rx, 50, rw, CLR_DARK_GRAY);

  tft.setTextSize(1);
  tft.setTextColor(CLR_BLACK, CLR_WHITE);
  tft.setCursor(rx, 58);
  tft.print("Scan QR Code");
  tft.setCursor(rx, 74);
  tft.print("untuk isi galon");
  tft.drawFastHLine(rx, 96, rw, CLR_DARK_GRAY);

  tft.setTextColor(CLR_DARK_GRAY, CLR_WHITE);
  tft.setCursor(rx, 106);
  tft.print("Device ID:");
  tft.setTextSize(2);
  tft.setTextColor(CLR_BLUE, CLR_WHITE);
  tft.setCursor(rx, 122);
  tft.print(DEVICE_CODE);
  tft.drawFastHLine(rx, 150, rw, CLR_DARK_GRAY);

  tft.setTextSize(1);
  tft.setTextColor(CLR_DARK_GRAY, CLR_WHITE);
  tft.setCursor(rx, 160);
  tft.print("Status: Menunggu");
  tft.fillCircle(rx + 4, 176, 4, CLR_GREEN);
  tft.setCursor(rx + 14, 172);
  tft.print("Siap");

  tft.drawFastHLine(0, 228, TFT_W, CLR_DARK_GRAY);
  char footer[48];
  snprintf(footer, sizeof(footer), "GalonKu - %s", DEVICE_CODE);
  drawCenteredText(footer, 232, 1, CLR_DARK_GRAY, CLR_WHITE);
}

void drawScannedScreen() {
  tft.fillScreen(CLR_LIGHT_GREEN);
  tft.fillRect(0, 0, TFT_W, 40, CLR_DARK_GREEN);
  drawCenteredText("GalonKu", 10, 2, CLR_WHITE, CLR_DARK_GREEN);

  int cx = 80, cy = 130;
  tft.fillCircle(cx, cy, 52, tft.color565(150, 230, 160));
  tft.fillCircle(cx, cy, 46, CLR_DARK_GREEN);
  tft.fillCircle(cx, cy, 40, CLR_GREEN);
  for (int t = 0; t < 6; t++) {
    tft.drawLine(54 + t, 130, 74 + t, 152, CLR_WHITE);
    tft.drawLine(74 + t, 152, 112 + t, 108, CLR_WHITE);
  }

  int rx = 152;
  tft.setTextSize(2);
  tft.setTextColor(CLR_DARK_GREEN, CLR_LIGHT_GREEN);
  tft.setCursor(rx, 58);
  tft.print("Pembayaran");
  tft.setCursor(rx, 82);
  tft.print("Diterima!");
  tft.drawFastHLine(rx, 110, TFT_W - rx - 8, CLR_DARK_GREEN);

  tft.setTextSize(1);
  tft.setTextColor(CLR_BLACK, CLR_LIGHT_GREEN);
  tft.setCursor(rx, 120);
  tft.print("Memulai pengisian...");
  tft.setCursor(rx, 140);
  tft.print("Mohon tunggu sebentar.");

  tft.drawFastHLine(0, 228, TFT_W, CLR_DARK_GREEN);
  char footer[32];
  snprintf(footer, sizeof(footer), "GalonKu - %s", DEVICE_CODE);
  drawCenteredText(footer, 232, 1, CLR_DARK_GREEN, CLR_LIGHT_GREEN);
}

void drawPrepareFillScreen(int current, int total) {
  tft.fillScreen(CLR_LIGHT_BLUE);
  tft.fillRect(0, 0, TFT_W, 40, CLR_BLUE);
  drawCenteredText("GalonKu", 10, 2, CLR_WHITE, CLR_BLUE);

  int cx = 80, cy = 110;
  tft.fillTriangle(cx, cy + 30, cx - 28, cy - 10, cx + 28, cy - 10, CLR_YELLOW);
  tft.fillRect(cx - 12, cy - 40, 24, 34, CLR_YELLOW);

  tft.fillRoundRect(cx - 22, cy + 36, 44, 58, 6, CLR_BLUE);
  tft.fillRoundRect(cx - 20, cy + 38, 40, 54, 5, CLR_CYAN);
  tft.fillRect(cx - 10, cy + 28, 20, 12, CLR_BLUE);
  tft.fillRect(cx - 8, cy + 30, 16, 8, CLR_CYAN);

  int rx = 140;
  char galStr[24];
  snprintf(galStr, sizeof(galStr), "Galon %d/%d", current, total);
  tft.setTextSize(2);
  tft.setTextColor(CLR_BLUE, CLR_LIGHT_BLUE);
  tft.setCursor(rx, 50);
  tft.print(galStr);
  tft.drawFastHLine(rx, 78, TFT_W - rx - 8, CLR_BLUE);

  tft.setTextSize(1);
  tft.setTextColor(CLR_BLACK, CLR_LIGHT_BLUE);
  tft.setCursor(rx, 88);
  tft.print("Silakan tempatkan");
  tft.setCursor(rx, 104);
  tft.print("galon kosong Anda");
  tft.setCursor(rx, 120);
  tft.print("di bawah dispenser.");

  tft.drawFastHLine(rx, 140, TFT_W - rx - 8, CLR_BLUE);
  tft.setTextColor(CLR_DARK_GRAY, CLR_LIGHT_BLUE);
  tft.setCursor(rx, 150);
  tft.print("Menunggu sensor IR...");

  tft.fillCircle(rx + 4, 170, 4, CLR_ORANGE);
  tft.setTextColor(CLR_DARK_GRAY, CLR_LIGHT_BLUE);
  tft.setCursor(rx + 14, 166);
  tft.print("Galon belum terdeteksi");

  tft.drawFastHLine(0, 228, TFT_W, CLR_BLUE);
  char footer[32];
  snprintf(footer, sizeof(footer), "GalonKu - %s", DEVICE_CODE);
  drawCenteredText(footer, 232, 1, CLR_BLUE, CLR_LIGHT_BLUE);
}

void drawConfirmFillScreen(int current, int total) {
  tft.fillScreen(CLR_LIGHT_GREEN);
  tft.fillRect(0, 0, TFT_W, 40, CLR_DARK_GREEN);
  drawCenteredText("GalonKu", 10, 2, CLR_WHITE, CLR_DARK_GREEN);

  int cx = 80, cy = 110;
  tft.fillCircle(cx, cy, 38, tft.color565(100, 200, 100));
  tft.fillCircle(cx, cy, 32, CLR_DARK_GREEN);
  for (int t = 0; t < 5; t++) {
    tft.drawLine(58 + t, 110, 74 + t, 128, CLR_WHITE);
    tft.drawLine(74 + t, 128, 106 + t, 90, CLR_WHITE);
  }

  tft.fillRoundRect(52, 152, 56, 36, 8, CLR_BLUE);
  tft.fillRoundRect(55, 155, 50, 30, 6, CLR_LIGHT_BLUE);
  drawCenteredText("OK", 163, 1, CLR_WHITE, CLR_LIGHT_BLUE);

  int rx = 145;
  char galStr[24];
  snprintf(galStr, sizeof(galStr), "Galon %d/%d", current, total);
  tft.setTextSize(2);
  tft.setTextColor(CLR_DARK_GREEN, CLR_LIGHT_GREEN);
  tft.setCursor(rx, 50);
  tft.print(galStr);
  tft.drawFastHLine(rx, 78, TFT_W - rx - 8, CLR_DARK_GREEN);

  tft.setTextSize(1);
  tft.setTextColor(CLR_BLACK, CLR_LIGHT_GREEN);
  tft.setCursor(rx, 88);
  tft.print("Galon Terdeteksi!");

  tft.setTextColor(CLR_DARK_GREEN, CLR_LIGHT_GREEN);
  tft.setCursor(rx, 108);
  tft.print("Tekan TOMBOL di");
  tft.setCursor(rx, 124);
  tft.print("dispenser untuk");
  tft.setCursor(rx, 140);
  tft.print("memulai pengisian.");

  tft.fillCircle(rx + 4, 162, 4, CLR_GREEN);
  tft.setTextColor(CLR_DARK_GREEN, CLR_LIGHT_GREEN);
  tft.setCursor(rx + 14, 158);
  tft.print("IR: Galon OK");

  tft.drawFastHLine(0, 228, TFT_W, CLR_DARK_GREEN);
  char footer[32];
  snprintf(footer, sizeof(footer), "GalonKu - %s", DEVICE_CODE);
  drawCenteredText(footer, 232, 1, CLR_DARK_GREEN, CLR_LIGHT_GREEN);
}

void drawGalonLiftedScreen() {
  tft.fillScreen(CLR_ORANGE);
  tft.fillRect(0, 0, TFT_W, 40, tft.color565(180, 80, 0));
  drawCenteredText("GalonKu", 10, 2, CLR_WHITE, tft.color565(180, 80, 0));

  int cx = 80, cy = 125;
  tft.fillTriangle(cx, cy - 50, cx - 44, cy + 34, cx + 44, cy + 34, CLR_YELLOW);
  tft.fillTriangle(cx, cy - 40, cx - 36, cy + 26, cx + 36, cy + 26, CLR_ORANGE);
  tft.fillRect(cx - 5, cy - 24, 10, 28, CLR_YELLOW);
  tft.fillRect(cx - 5, cy + 10, 10, 9, CLR_YELLOW);

  int rx = 145;
  tft.setTextSize(2);
  tft.setTextColor(CLR_WHITE, CLR_ORANGE);
  tft.setCursor(rx, 50);
  tft.print("Galon");
  tft.setCursor(rx, 74);
  tft.print("Terangkat!");
  tft.drawFastHLine(rx, 102, TFT_W - rx - 8, CLR_YELLOW);

  tft.setTextSize(1);
  tft.setTextColor(CLR_WHITE, CLR_ORANGE);
  tft.setCursor(rx, 112);
  tft.print("POMPA DIMATIKAN!");
  tft.setCursor(rx, 132);
  tft.print("Kembalikan galon,");
  tft.setCursor(rx, 148);
  tft.print("lalu tekan tombol");
  tft.setCursor(rx, 164);
  tft.print("untuk melanjutkan.");

  tft.drawFastHLine(0, 228, TFT_W, CLR_YELLOW);
  char footer[32];
  snprintf(footer, sizeof(footer), "GalonKu - %s", DEVICE_CODE);
  drawCenteredText(footer, 232, 1, CLR_YELLOW, CLR_ORANGE);
}

void drawProcessingScreen(int current, int total) {
  tft.fillScreen(CLR_LIGHT_BLUE);
  tft.fillRect(0, 0, TFT_W, 40, CLR_BLUE);
  drawCenteredText("GalonKu", 10, 2, CLR_WHITE, CLR_BLUE);

  tft.fillRoundRect(28, 56, 66, 88, 8, CLR_BLUE);
  tft.fillRoundRect(30, 58, 62, 84, 6, CLR_LIGHT_BLUE);
  tft.fillRect(42, 46, 42, 14, CLR_BLUE);
  tft.fillRect(44, 48, 38, 10, CLR_LIGHT_BLUE);
  tft.fillRoundRect(30, 100, 62, 40, 6, CLR_CYAN);
  tft.fillCircle(45, 115, 4, CLR_WHITE);
  tft.fillCircle(62, 108, 3, CLR_WHITE);
  tft.fillCircle(75, 118, 2, CLR_WHITE);

  int rx = 110;
  char galStr[24];
  snprintf(galStr, sizeof(galStr), "Galon %d/%d", current, total);
  tft.setTextSize(1);
  tft.setTextColor(CLR_WHITE, CLR_BLUE);
  tft.fillRoundRect(TFT_W - 80, 12, 72, 18, 4, tft.color565(0, 0, 180));
  tft.setCursor(TFT_W - 76, 16);
  tft.print(galStr);

  tft.setTextSize(2);
  tft.setTextColor(CLR_BLUE, CLR_LIGHT_BLUE);
  tft.setCursor(rx, 50);
  tft.print("Mengisi...");

  tft.setTextSize(1);
  tft.setTextColor(CLR_BLUE, CLR_LIGHT_BLUE);
  tft.setCursor(rx, 78);
  tft.print("Mohon tunggu sebentar");
  tft.drawFastHLine(rx, 96, TFT_W - rx - 8, CLR_BLUE);
  tft.setCursor(rx, 106);
  tft.print("Jangan cabut galon!");

  tft.drawFastHLine(0, 140, TFT_W, CLR_BLUE);
  drawProgressBar(10, 148, 300, 22, 0, CLR_BLUE, CLR_DARK_GRAY);
  drawCenteredText("Sisa: -- detik", 178, 1, CLR_WHITE, CLR_LIGHT_BLUE);

  tft.drawFastHLine(0, 228, TFT_W, CLR_BLUE);
  char footer[32];
  snprintf(footer, sizeof(footer), "GalonKu - %s", DEVICE_CODE);
  drawCenteredText(footer, 232, 1, CLR_BLUE, CLR_LIGHT_BLUE);
}

void drawCheckNextScreen(int filled, int total) {
  tft.fillScreen(CLR_LIGHT_GREEN);
  tft.fillRect(0, 0, TFT_W, 40, CLR_DARK_GREEN);
  drawCenteredText("GalonKu", 10, 2, CLR_WHITE, CLR_DARK_GREEN);

  int cx = 80, cy = 120;
  tft.fillCircle(cx, cy, 46, tft.color565(100, 210, 120));
  tft.fillCircle(cx, cy, 40, CLR_DARK_GREEN);
  for (int t = 0; t < 6; t++) {
    tft.drawLine(52 + t, 120, 72 + t, 142, CLR_WHITE);
    tft.drawLine(72 + t, 142, 112 + t, 96, CLR_WHITE);
  }

  int rx = 145;
  char doneStr[24];
  snprintf(doneStr, sizeof(doneStr), "%d/%d Selesai!", filled, total);

  tft.setTextSize(2);
  tft.setTextColor(CLR_DARK_GREEN, CLR_LIGHT_GREEN);
  tft.setCursor(rx, 50);
  tft.print("Galon");
  tft.setCursor(rx, 74);
  tft.print("Penuh!");
  tft.drawFastHLine(rx, 102, TFT_W - rx - 8, CLR_DARK_GREEN);

  tft.setTextSize(1);
  tft.setTextColor(CLR_BLACK, CLR_LIGHT_GREEN);
  tft.setCursor(rx, 112);
  tft.print(doneStr);

  if (filled < total) {
    tft.setCursor(rx, 132);
    tft.print("Silakan ambil galon");
    tft.setCursor(rx, 148);
    tft.print("ini, lalu taruh");
    tft.setCursor(rx, 164);
    char nextStr[24];
    snprintf(nextStr, sizeof(nextStr), "galon ke-%d.", filled + 1);
    tft.print(nextStr);

    tft.fillCircle(rx + 4, 184, 4, CLR_ORANGE);
    tft.setTextColor(CLR_DARK_GRAY, CLR_LIGHT_GREEN);
    tft.setCursor(rx + 14, 180);
    tft.print("Menunggu diambil...");
  }

  tft.drawFastHLine(0, 228, TFT_W, CLR_DARK_GREEN);
  char footer[32];
  snprintf(footer, sizeof(footer), "GalonKu - %s", DEVICE_CODE);
  drawCenteredText(footer, 232, 1, CLR_DARK_GREEN, CLR_LIGHT_GREEN);
}

void drawDoneScreen() {
  tft.fillScreen(CLR_GREEN);
  tft.fillRect(0, 0, TFT_W, 40, CLR_DARK_GREEN);
  drawCenteredText("GalonKu", 10, 2, CLR_WHITE, CLR_DARK_GREEN);

  int cx = 80, cy = 130;
  tft.fillCircle(cx, cy, 54, tft.color565(0, 180, 80));
  tft.fillCircle(cx, cy, 48, CLR_WHITE);
  tft.fillCircle(cx, cy, 42, CLR_DARK_GREEN);
  for (int t = 0; t < 7; t++) {
    tft.drawLine(52 + t, 130, 74 + t, 154, CLR_WHITE);
    tft.drawLine(74 + t, 154, 114 + t, 106, CLR_WHITE);
  }

  int rx = 152;
  tft.setTextSize(2);
  tft.setTextColor(CLR_WHITE, CLR_GREEN);
  tft.setCursor(rx, 55);
  tft.print("Pengisian");
  tft.setCursor(rx, 79);
  tft.print("Selesai!");
  tft.drawFastHLine(rx, 108, TFT_W - rx - 8, CLR_WHITE);

  tft.setTextSize(1);
  tft.setTextColor(CLR_WHITE, CLR_GREEN);
  tft.setCursor(rx, 118);
  tft.print("Silakan ambil");
  tft.setCursor(rx, 134);
  tft.print("galon Anda :)");
  tft.setCursor(rx, 158);
  tft.setTextColor(tft.color565(200, 255, 210), CLR_GREEN);
  tft.print("Terima kasih!");

  tft.drawFastHLine(0, 228, TFT_W, CLR_DARK_GREEN);
  char footer[32];
  snprintf(footer, sizeof(footer), "GalonKu - %s", DEVICE_CODE);
  drawCenteredText(footer, 232, 1, CLR_DARK_GREEN, CLR_GREEN);
}

void drawErrorScreen(const char* message) {
  tft.fillScreen(CLR_RED);
  tft.fillRect(0, 0, TFT_W, 40, 0x8000);
  drawCenteredText("GalonKu", 10, 2, CLR_WHITE, 0x8000);

  int cx = 80, cy = 135;
  tft.fillTriangle(cx, cy - 52, cx - 46, cy + 36, cx + 46, cy + 36, CLR_YELLOW);
  tft.fillTriangle(cx, cy - 42, cx - 38, cy + 28, cx + 38, cy + 28, CLR_RED);
  tft.fillRect(cx - 5, cy - 26, 10, 30, CLR_YELLOW);
  tft.fillRect(cx - 5, cy + 10, 10, 9, CLR_YELLOW);

  int rx = 145;
  tft.setTextSize(2);
  tft.setTextColor(CLR_WHITE, CLR_RED);
  tft.setCursor(rx, 55);
  tft.print("Terjadi");
  tft.setCursor(rx, 79);
  tft.print("Kesalahan!");
  tft.drawFastHLine(rx, 108, TFT_W - rx - 8, CLR_YELLOW);

  tft.setTextSize(1);
  tft.setTextColor(CLR_YELLOW, CLR_RED);
  tft.setCursor(rx, 118);
  tft.print(message);
  tft.setTextColor(CLR_WHITE, CLR_RED);
  tft.setCursor(rx, 138);
  tft.print("Mencoba recovery...");
  tft.setCursor(rx, 154);
  tft.print("Menghubungi server...");

  tft.drawFastHLine(0, 228, TFT_W, CLR_YELLOW);
  char footer[32];
  snprintf(footer, sizeof(footer), "GalonKu - %s", DEVICE_CODE);
  drawCenteredText(footer, 232, 1, CLR_YELLOW, CLR_RED);
}
