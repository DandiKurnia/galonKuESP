# Analisis Kode Program GalonKu

## Sistem Dispenser Air Galon Otomatis (ESP32-S3)

Berikut adalah analisis seluruh kode program dari file `program.ino` tanpa singkatan, dibagi per blok fungsi dan komponen beserta penjelasannya:

<table>
<thead>
<tr>
<th style="width: 50%;">Blok Kode Program</th>
<th style="width: 50%;">Penjelasan Fungsi / Komponen</th>
</tr>
</thead>
<tbody>
<!-- ROW 1: LIBRARIES -->
<tr>
<td>
<pre><code>#include &lt;WiFi.h&gt;
#include &lt;HTTPClient.h&gt;
#include &lt;ArduinoJson.h&gt;
#include &lt;TFT_eSPI.h&gt;
#include &lt;SPI.h&gt;
#include &lt;ricmoo_qrcode.h&gt;
#include &lt;WiFiClientSecure.h&gt;</code></pre>
</td>
<td>
<strong>Library / Pustaka Pendukung:</strong>
<ul>
  <li><code>WiFi.h</code>: Mengelola koneksi jaringan WiFi lokal pada ESP32.</li>
  <li><code>HTTPClient.h</code>: Mengirim request HTTP/HTTPS ke server REST API.</li>
  <li><code>ArduinoJson.h</code>: Melakukan serialisasi dan deserialisasi data format JSON.</li>
  <li><code>TFT_eSPI.h</code>: Driver untuk layar TFT LCD (ILI9341 / ST7789).</li>
  <li><code>SPI.h</code>: Protokol komunikasi bus SPI untuk layar LCD.</li>
  <li><code>ricmoo_qrcode.h</code>: Membuat data visual QR Code langsung pada ESP32.</li>
  <li><code>WiFiClientSecure.h</code>: Mengamankan koneksi HTTPS via TLS/SSL.</li>
</ul>
</td>
</tr>
<!-- ROW 2: KONFIGURASI WIFI & API -->
<tr>
<td>
<pre><code>const char* WIFI_SSID     = "ASUS";
const char* WIFI_PASSWORD = "12345678";
const char* BASE_URL      = "https://api.galonku.my.id";
const char* DEVICE_CODE   = "DEV-44";
const char* DEVICE_TOKEN  = "dtkn_6f8ec5ea186260fdcb8c894d4ec73191b4398ee2b5f83623a7ecd68b1d9312ea";
const unsigned long POLL_INTERVAL    = 3000;
const int           HTTP_TIMEOUT_MS  = 5000;</code></pre>
</td>
<td>
<strong>Konfigurasi Koneksi &amp; API Server:</strong>
<ul>
  <li><code>WIFI_SSID</code> &amp; <code>WIFI_PASSWORD</code>: Kredensial jaringan WiFi yang akan dihubungkan.</li>
  <li><code>BASE_URL</code>: Alamat domain server backend API GalonKu.</li>
  <li><code>DEVICE_CODE</code> &amp; <code>DEVICE_TOKEN</code>: Identitas unik dan token keamanan perangkat dispenser nomor 44 untuk autentikasi API.</li>
  <li><code>POLL_INTERVAL</code>: Interval waktu polling status ke server (3000 ms atau 3 detik).</li>
  <li><code>HTTP_TIMEOUT_MS</code>: Batas maksimal menunggu respon server (5 detik).</li>
</ul>
</td>
</tr>
<!-- ROW 3: KONFIGURASI FLOW SENSOR -->
<tr>
<td>
<pre><code>// Flow sensor
#define PIN_FLOW_SENSOR 17
const float TARGET_LITERS = 1.0;
const unsigned long PULSES_PER_LITER = 450;
const unsigned long TARGET_PULSES = (unsigned long)(TARGET_LITERS * PULSES_PER_LITER);
const unsigned long MAX_FILL_TIMEOUT_MS = 30000; // safety timeout 30 detik</code></pre>
</td>
<td>
<strong>Konfigurasi Takaran Air (Flow Sensor YF-S201):</strong>
<ul>
  <li><code>PIN_FLOW_SENSOR</code>: Sensor dipasang pada GPIO 17.</li>
  <li><code>TARGET_LITERS</code>: Jumlah target air per pengisian yaitu 1.0 Liter.</li>
  <li><code>PULSES_PER_LITER</code>: Kalibrasi sensor flow, di mana 450 pulsa setara dengan 1 liter air.</li>
  <li><code>TARGET_PULSES</code>: Hasil perkalian target volume dengan konstanta pulsa (450 pulsa).</li>
  <li><code>MAX_FILL_TIMEOUT_MS</code>: Fitur keselamatan (safety timeout). Jika dalam 30 detik pengisian belum mencapai target, pompa otomatis mati guna mencegah luapan/kebocoran.</li>
</ul>
</td>
</tr>
<!-- ROW 4: HARDWARE PIN -->
<tr>
<td>
<pre><code>// Hardware
#define PIN_IR_SENSOR  18
#define PIN_BUTTON     19
#define PIN_RELAY      20
#define TFT_W 320
#define TFT_H 240</code></pre>
</td>
<td>
<strong>Definisi Pin Perangkat Keras &amp; Dimensi LCD:</strong>
<ul>
  <li><code>PIN_IR_SENSOR</code>: GPIO 18 untuk sensor IR obstacle (mendeteksi keberadaan galon).</li>
  <li><code>PIN_BUTTON</code>: GPIO 19 untuk tombol fisik konfirmasi pengisian.</li>
  <li><code>PIN_RELAY</code>: GPIO 20 untuk mengontrol saklar elektronik pompa air.</li>
  <li><code>TFT_W</code> &amp; <code>TFT_H</code>: Dimensi lebar (320 piksel) dan tinggi (240 piksel) dari layar LCD.</li>
</ul>
</td>
</tr>
<!-- ROW 5: WARNA TFT -->
<tr>
<td>
<pre><code>// =============================================================================
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
#define CLR_CYAN         0x07FF</code></pre>
</td>
<td>
<strong>Definisi Palet Warna Layar (RGB565):</strong>
<p>Daftar konstanta warna dalam format RGB 16-bit (5-bit merah, 6-bit hijau, 5-bit biru). Format warna ini wajib digunakan untuk menggambar visual pada layar TFT LCD ILI9341/ST7789.</p>
</td>
</tr>
<!-- ROW 6: STATE MACHINE -->
<tr>
<td>
<pre><code>// =============================================================================
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
};</code></pre>
</td>
<td>
<strong>Mesin Status Aplikasi (Finite State Machine):</strong>
<p>Mendefinisikan tipe data status/keadaan sistem dispenser secara terstruktur:</p>
<ul>
  <li><code>STATE_WIFI_CONNECTING</code>: Sedang menyambungkan ke WiFi.</li>
  <li><code>STATE_IDLE</code>: Standby, menampilkan QR Code untuk dipindai.</li>
  <li><code>STATE_PREPARE_FILL</code>: Meminta pengguna menaruh galon kosong.</li>
  <li><code>STATE_CONFIRM_FILL</code>: Galon terdeteksi, menunggu tombol ditekan.</li>
  <li><code>STATE_PROCESSING</code>: Pompa aktif, pengisian air sedang berjalan.</li>
  <li><code>STATE_CHECK_NEXT</code>: Cek antrean pengisian galon berikutnya.</li>
  <li><code>STATE_DONE</code>: Semua galon selesai diisi, proses tuntas.</li>
  <li><code>STATE_ERROR</code>: Terjadi kendala (timeout/koneksi), sistem dikunci demi keamanan.</li>
</ul>
</td>
</tr>
<!-- ROW 7: VARIABEL GLOBAL -->
<tr>
<td>
<pre><code>// =============================================================================
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
unsigned long lastPulseCount = 0;</code></pre>
</td>
<td>
<strong>Deklarasi Variabel Global:</strong>
<ul>
  <li><code>tft</code>: Instansiasi objek driver LCD TFT_eSPI.</li>
  <li><code>currentState</code> &amp; <code>lastDrawnState</code>: Status FSM aktif dan status layar terakhir yang digambar (mencegah kedipan layar akibat redraw berlebih).</li>
  <li><code>lastPollTime</code>: Waktu terakhir modul melakukan request HTTP ke API.</li>
  <li><code>fillingStart</code>: Waktu milidetik (millis) saat pompa mulai berjalan.</li>
  <li><code>wifiWasConnected</code>: Status flag penanda kondisi koneksi WiFi sebelumnya.</li>
  <li><code>totalGallons</code> &amp; <code>filledGallons</code>: Jumlah antrean galon yang dipesan vs jumlah galon yang telah terisi.</li>
  <li><code>isPaused</code> &amp; <code>remainingTimeMs</code>: Menyimpan status jeda serta sisa waktu pengisian jika galon diangkat tiba-tiba.</li>
  <li><code>qrContent</code>: Array karakter penampung string isi QR Code.</li>
  <li><code>flowPulseCount</code>: Variabel volatile pencacah pulsa sensor air (volatile karena diakses langsung di dalam fungsi interupsi ISR).</li>
  <li><code>lastPulseCheckTime</code> &amp; <code>lastPulseCount</code>: Penyimpan status perhitungan debit air sebelumnya.</li>
</ul>
</td>
</tr>
<!-- ROW 8: DEKLARASI FUNGSI -->
<tr>
<td>
<pre><code>// =============================================================================
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
void resetFlowCounter();</code></pre>
</td>
<td>
<strong>Prototipe / Deklarasi Fungsi (Function Prototype):</strong>
<p>Mendeklarasikan semua fungsi yang digunakan dalam program sebelum diimplementasikan di bagian bawah. Berguna agar compiler C++ mengenali fungsi-fungsi tersebut saat dipanggil di dalam fungsi <code>setup()</code> dan <code>loop()</code>.</p>
</td>
</tr>
<!-- ROW 9: SETUP - SERIAL & PINS -->
<tr>
<td>
<pre><code>void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== GalonKu Dispenser v3.0 ===");
  Serial.printf("Device Code : %s\n", DEVICE_CODE);
  pinMode(PIN_IR_SENSOR, INPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, LOW);
  pinMode(PIN_FLOW_SENSOR, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_FLOW_SENSOR), flowPulseISR, FALLING);</code></pre>
</td>
<td>
<strong>Inisialisasi Hardware &amp; Debugging (setup bagian 1):</strong>
<ul>
  <li><code>Serial.begin(115200)</code>: Membuka komunikasi serial debugging.</li>
  <li><code>pinMode</code>: Mengatur GPIO 18 (Sensor IR) sebagai input digital biasa, GPIO 19 (Tombol) sebagai input pull-up internal (active LOW), dan GPIO 20 (Relay Pompa) sebagai output digital.</li>
  <li><code>digitalWrite(PIN_RELAY, LOW)</code>: Mematikan relay pompa air saat awal menyala untuk alasan keamanan.</li>
  <li><code>attachInterrupt</code>: Mengaktifkan interrupt hardware pada GPIO 17 (Flow Sensor) yang memicu fungsi <code>flowPulseISR</code> ketika sinyal transisi jatuh (FALLING/HIGH ke LOW).</li>
</ul>
</td>
</tr>
<!-- ROW 10: SETUP - TFT LCD & WIFI CONNECT -->
<tr>
<td>
<pre><code>  Serial.println("[HW] IR Sensor  : GPIO 18");
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
}</code></pre>
</td>
<td>
<strong>Inisialisasi LCD &amp; Jaringan (setup bagian 2):</strong>
<ul>
  <li>Mencetak log konfigurasi pin hardware ke serial monitor.</li>
  <li><code>tft.init()</code>: Mengaktifkan LCD TFT ILI9341/ST7789.</li>
  <li><code>tft.setRotation(1)</code>: Mengatur orientasi layar horizontal (320x240 piksel).</li>
  <li><code>tft.fillScreen(CLR_BLACK)</code>: Membersihkan layar TFT dengan warna hitam.</li>
  <li><code>snprintf</code>: Memasukkan string <code>DEVICE_CODE</code> ke array buffer <code>qrContent</code>.</li>
  <li>Mengarahkan status awal ke <code>STATE_WIFI_CONNECTING</code>, menggambar layar loading koneksi WiFi, lalu mengeksekusi fungsi <code>connectWiFi()</code>.</li>
</ul>
</td>
</tr>
<!-- ROW 11: LOOP - WIFI MONITOR -->
<tr>
<td>
<pre><code>void loop() {
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
  switch (currentState) {</code></pre>
</td>
<td>
<strong>Siklus Utama &amp; Pengawas WiFi (loop bagian 1):</strong>
<ul>
  <li>Membaca nilai milidetik aktif ESP32 via <code>millis()</code>.</li>
  <li>Melakukan pengecekan status jaringan nirkabel. Jika WiFi terputus saat sistem sedang berjalan, program langsung mematikan pompa air (LOW) untuk keselamatan, mengubah layar ke loading koneksi, lalu memanggil fungsi re-koneksi <code>connectWiFi()</code>.</li>
  <li>Jika terhubung, flag <code>wifiWasConnected</code> diatur bernilai true dan program masuk ke switch-case evaluasi status (FSM).</li>
</ul>
</td>
</tr>
<!-- ROW 12: LOOP - STATE_IDLE -->
<tr>
<td>
<pre><code>    case STATE_IDLE: {
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
    }</code></pre>
</td>
<td>
<strong>Logika Status Siaga (loop bagian 2 - STATE_IDLE):</strong>
<ul>
  <li>Tampilan QR Code digambar di layar jika LCD belum menampilkan halaman standby.</li>
  <li>Setiap 3 detik (<code>POLL_INTERVAL</code>), ESP32 melakukan request GET status ke server API.</li>
  <li>Jika status "SCANNED", counter di-reset dan masuk ke <code>STATE_PREPARE_FILL</code>.</li>
  <li>Jika status "PROCESSING" didapat secara stale, sistem otomatis mengirim "DONE" untuk membersihkan antrean (recovery gantung).</li>
  <li>Jika status "ERROR" atau "FAILED", lompat ke penanganan kesalahan (<code>STATE_ERROR</code>).</li>
</ul>
</td>
</tr>
<!-- ROW 13: LOOP - STATE_PREPARE_FILL -->
<tr>
<td>
<pre><code>    case STATE_PREPARE_FILL: {
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
    }</code></pre>
</td>
<td>
<strong>Logika Deteksi Penempatan Galon (loop bagian 3 - STATE_PREPARE_FILL):</strong>
<ul>
  <li>Menggambar antarmuka petunjuk peletakan galon kosong lengkap dengan info nomor antrean galon aktif.</li>
  <li><code>digitalRead(PIN_IR_SENSOR) == LOW</code>: Membaca kondisi sensor inframerah secara real-time. Jika sensor mendeteksi objek (LOW), program langsung memindahkan status dispenser ke <code>STATE_CONFIRM_FILL</code>.</li>
</ul>
</td>
</tr>
<!-- ROW 14: LOOP - STATE_CONFIRM_FILL -->
<tr>
<td>
<pre><code>    case STATE_CONFIRM_FILL: {
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
    }</code></pre>
</td>
<td>
<strong>Logika Konfirmasi &amp; Aktivasi Pompa (loop bagian 4 - STATE_CONFIRM_FILL):</strong>
<ul>
  <li>Menampilkan layar konfirmasi tombol tekan "OK".</li>
  <li>Jika galon diangkat kembali sebelum tombol ditekan (sensor IR membaca HIGH), alur dibatalkan dan dikembalikan ke <code>STATE_PREPARE_FILL</code>.</li>
  <li>Jika tombol ditekan (<code>btnNow == LOW</code>) dan sensor IR valid (ada galon):
    <ul>
      <li>Relay pompa air dinyalakan (HIGH).</li>
      <li>Jika status dalam mode jeda (isPaused), sisa waktu safety timeout dikalkulasi ulang untuk dilanjutkan (resume). Jika pengisian baru, waktu pengisian (<code>fillingStart</code>) di-reset ke millis saat ini.</li>
      <li>Cacah sensor flow dibersihkan ke nol (<code>resetFlowCounter()</code>).</li>
      <li>Jika ini adalah pengisian galon pertama, program mengirim request update status "PROCESSING" ke cloud API server.</li>
      <li>Mengalihkan status FSM ke <code>STATE_PROCESSING</code>.</li>
    </ul>
  </li>
</ul>
</td>
</tr>
<!-- ROW 15: LOOP - STATE_PROCESSING -->
<tr>
<td>
<pre><code>    case STATE_PROCESSING: {
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
    }</code></pre>
</td>
<td>
<strong>Logika Proses Pengisian Air Bahaya (loop bagian 5 - STATE_PROCESSING):</strong>
<ul>
  <li><strong>Pengaman Galon Terangkat:</strong> Jika di tengah pengisian galon diangkat (IR membaca HIGH), pompa langsung dimatikan (LOW), sisa waktu timeout disimpan, flag <code>isPaused</code> diset True, memicu tampilan peringatan "Galon Terangkat!" selama 2 detik, lalu mengembalikan status ke <code>STATE_CONFIRM_FILL</code>.</li>
  <li><strong>Pembaruan UI Real-time:</strong> Menggambar persentase kemajuan volume air lewat bar progress secara visual, disertai keterangan liter air terkirim hasil konversi jumlah pulsa flow dibagi 450.</li>
  <li><strong>Selesai Target:</strong> Bila pulsa flow melampaui target (450 pulsa / 1 liter), pompa dimatikan (LOW), variabel galon sukses terisi (<code>filledGallons</code>) bertambah satu, lalu bergeser ke status <code>STATE_CHECK_NEXT</code>.</li>
  <li><strong>Pengaman Durasi Batas (Safety Timeout):</strong> Jika waktu berlalu melebihi batas 30 detik (<code>MAX_FILL_TIMEOUT_MS</code>) namun volume belum terpenuhi (karena air habis / sensor macet), pompa dimatikan paksa (LOW) dan sistem masuk ke <code>STATE_ERROR</code>.</li>
</ul>
</td>
</tr>
<!-- ROW 16: LOOP - STATE_CHECK_NEXT -->
<tr>
<td>
<pre><code>    case STATE_CHECK_NEXT: {
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
    }</code></pre>
</td>
<td>
<strong>Logika Pengurutan Multi-Galon (loop bagian 6 - STATE_CHECK_NEXT):</strong>
<ul>
  <li>Menampilkan layar visual galon penuh "X/Y Selesai".</li>
  <li>Jika jumlah galon terisi masih kurang dari total kuantitas transaksi: Program memantau sensor IR. Jika galon yang sudah terisi penuh diangkat (IR membaca HIGH), program memberi delay 1 detik lalu beralih kembali ke status persiapan <code>STATE_PREPARE_FILL</code> untuk pengisian galon kosong berikutnya.</li>
  <li>Jika semua pesanan galon telah selesai diisi: ESP32 mengirim request update status transaksi menjadi "DONE" ke cloud API server, kemudian beralih ke status transaksi final <code>STATE_DONE</code>.</li>
</ul>
</td>
</tr>
<!-- ROW 17: LOOP - STATE_DONE -->
<tr>
<td>
<pre><code>    case STATE_DONE: {
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
    }</code></pre>
</td>
<td>
<strong>Logika Penutupan Transaksi (loop bagian 7 - STATE_DONE):</strong>
<ul>
  <li>Menggambar layar penutupan berwarna hijau dengan ucapan terima kasih selama 5 detik.</li>
  <li>Mereset seluruh variabel transaksi lokal meliputi <code>totalGallons</code>, <code>filledGallons</code>, <code>remainingTimeMs</code>, dan status flag <code>isPaused</code> kembali ke posisi awal.</li>
  <li>Mengalihkan status FSM kembali ke status siaga <code>STATE_IDLE</code> untuk menampilkan QR Code baru.</li>
</ul>
</td>
</tr>
<!-- ROW 18: LOOP - STATE_ERROR -->
<tr>
<td>
<pre><code>    case STATE_ERROR: {
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
}</code></pre>
</td>
<td>
<strong>Logika Penanganan Error / Locking (loop bagian 8 - STATE_ERROR):</strong>
<ul>
  <li>Mematikan paksa pin relay pompa air (LOW) dan menampilkan halaman error merah "Hubungi operator" di layar LCD.</li>
  <li>Sistem terkunci dari pemakaian biasa. ESP32 melakukan request status GET ke server setiap 5 detik (Recovery Polling).</li>
  <li>Jika server merespon dengan status transaksi baru bernilai "SUCCESS" (artinya admin/operator backend telah melakukan reset atau penyelesaian transaksi manual secara cloud), status dikembalikan secara otomatis ke mode siaga <code>STATE_IDLE</code>.</li>
</ul>
</td>
</tr>
<!-- ROW 19: FLOW SENSOR ISR -->
<tr>
<td>
<pre><code>// =============================================================================
// FLOW SENSOR ISR
// =============================================================================
void IRAM_ATTR flowPulseISR() {
  flowPulseCount++;
}
void resetFlowCounter() {
  noInterrupts();
  flowPulseCount = 0;
  interrupts();
}</code></pre>
</td>
<td>
<strong>Interrupt Service Routine (ISR) &amp; Reset Sensor Flow:</strong>
<ul>
  <li><code>flowPulseISR()</code>: Fungsi penanganan interupsi cepat yang disimpan di RAM (IRAM_ATTR). Berfungsi mencacah setiap sinyal pulsa baling-baling dari sensor debit air secara langsung di latar belakang, memastikan tidak ada pulsa yang terlewat meskipun program utama sedang memproses logika lain.</li>
  <li><code>resetFlowCounter()</code>: Menghentikan seluruh interupsi mikrokontroler untuk sementara (noInterrupts) saat mereset nilai counter pulsa sensor ke nol, guna menghindari race-condition antar thread program. Interupsi kembali diaktifkan setelah reset berhasil.</li>
</ul>
</td>
</tr>
<!-- ROW 20: START & STOP FILLING -->
<tr>
<td>
<pre><code>void startFilling() {
  resetFlowCounter();
  digitalWrite(PIN_RELAY, HIGH);
  fillingStart = millis();
}
void stopFilling() {
  digitalWrite(PIN_RELAY, LOW);
}</code></pre>
</td>
<td>
<strong>Fungsi Kontrol Jalur Aliran Air:</strong>
<ul>
  <li><code>startFilling()</code>: Fungsi instan untuk menyalakan pompa air dengan mereset counter flow sensor terlebih dahulu, memberikan sinyal HIGH ke relay, serta mencatat milidetik awal pengisian.</li>
  <li><code>stopFilling()</code>: Fungsi instan untuk menghentikan aliran air dengan memutus aliran listrik relay (LOW).</li>
</ul>
</td>
</tr>
<!-- ROW 21: CONNECT WIFI -->
<tr>
<td>
<pre><code>// =============================================================================
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
}</code></pre>
</td>
<td>
<strong>Fungsi Konektivitas Jaringan (connectWiFi):</strong>
<ul>
  <li>Melakukan inisialisasi ESP32 dalam mode Station (STA) agar dapat terhubung dengan Access Point (router) luar.</li>
  <li>Memulai proses koneksi menggunakan <code>WiFi.begin()</code>.</li>
  <li>Melakukan looping tunggu (maksimal 40 kali percobaan atau 20 detik) sambil menggambar indikator loading titik-titik kuning pada layar TFT.</li>
  <li>Jika terhubung, menampilkan pesan sukses "WiFi Terhubung!" beserta alamat IP lokal yang didapat, lalu mengalihkan status FSM ke <code>STATE_IDLE</code>.</li>
  <li>Jika gagal, memunculkan notifikasi merah di layar selama 3 detik sebelum nanti dipanggil kembali untuk melakukan percobaan ulang.</li>
</ul>
</td>
</tr>
<!-- ROW 22: HTTP GET STATUS -->
<tr>
<td>
<pre><code>// =============================================================================
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
}</code></pre>
</td>
<td>
<strong>Request Mengambil Status Perangkat (getDeviceStatus):</strong>
<ul>
  <li>Menggunakan <code>WiFiClientSecure</code> dengan <code>setInsecure()</code> untuk membypass pengecekan rantai sertifikat SSL (HTTPS tanpa SSL certificate verification).</li>
  <li>Menyusun URL tujuan ke endpoint detail status device di cloud backend.</li>
  <li>Menambahkan header otentikasi wajib <code>x-device-code</code> dan <code>x-device-token</code>.</li>
  <li>Mengirimkan request HTTP GET ke REST API Server.</li>
  <li>Jika respon sukses (HTTP 200 OK), payload JSON yang diterima didekode menggunakan <code>ArduinoJson</code>.</li>
  <li>Membaca status transaksi terbaru (misal: "SCANNED"). Jika terdeteksi scan, fungsi juga akan membaca data payload <code>total_galon</code> untuk dikonversi menjadi antrean pengisian lokal dispenser.</li>
</ul>
</td>
</tr>
<!-- ROW 23: HTTP PATCH STATUS -->
<tr>
<td>
<pre><code>// =============================================================================
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
}</code></pre>
</td>
<td>
<strong>Request Update Status Perangkat (updateDeviceStatus):</strong>
<ul>
  <li>Digunakan untuk mengabarkan kemajuan proses dispenser ke cloud server (misalnya saat beralih ke "PROCESSING" atau "DONE").</li>
  <li>Menyiapkan data body request JSON dengan struktur objek JSON berisi payload kode device dan status baru yang akan di-update.</li>
  <li>Mengirim data tersebut via metode request HTTP PATCH dengan tipe data Content-Type JSON.</li>
  <li>Mengembalikan nilai boolean True apabila server merespon dengan kode sukses (HTTP 200 OK atau 201 Created), dan False apabila gagal terkirim.</li>
</ul>
</td>
</tr>
<!-- ROW 24: UI HELPERS -->
<tr>
<td>
<pre><code>// =============================================================================
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
}</code></pre>
</td>
<td>
<strong>Fungsi Pembantu (Helper) Visual UI:</strong>
<ul>
  <li><code>drawCenteredText()</code>: Menghitung lebar string teks di layar secara dinamis berdasarkan ukuran font untuk diposisikan secara presisi tepat di tengah layar secara horizontal.</li>
  <li><code>drawProgressBar()</code>: Menggambar komponen grafik persentase kemajuan (0% s/d 100%). Menampilkan border putih luar, mengisi warna background, lalu mengisi area progress di sisi dalam secara proporsional sesuai dengan laju persentase volume air.</li>
</ul>
</td>
</tr>
<!-- ROW 25: SCREEN WIFI CONNECTING -->
<tr>
<td>
<pre><code>// =============================================================================
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
}</code></pre>
</td>
<td>
<strong>Render Tampilan Koneksi WiFi:</strong>
<p>Menggambar tata letak layar saat perangkat baru menyala dan mencoba menyambungkan internet. Menampilkan latar abu-abu gelap, bar header biru di bagian atas bertuliskan logo "GalonKu", ikon lingkaran sinyal radio bertumpuk warna kuning di bagian tengah layar, teks status penyambungan, nama WiFi target (SSID), serta kotak area loading visual.</p>
</td>
</tr>
<!-- ROW 26: SCREEN QR CODE -->
<tr>
<td>
<pre><code>void drawQRCodeScreen(const char* content) {
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
}</code></pre>
</td>
<td>
<strong>Render Tampilan QR Code (drawQRCodeScreen):</strong>
<ul>
  <li>Membersihkan latar belakang layar TFT menjadi putih bersih.</li>
  <li>Membuat matriks kode QR versi 3 (tingkat error correction rendah / ECC_LOW) yang diisi dengan string unik device code dispenser.</li>
  <li>Menggambar kode QR pixel-demi-pixel secara manual di sisi kiri layar dengan warna hitam untuk modul data, dan putih untuk modul kosong.</li>
  <li>Di sisi kanan layar, program menggambar teks promosi "Scan QR Code untuk isi galon", menampilkan Kode Device ID dispenser berukuran besar, status keaktifan mesin berwarna hijau dengan keterangan "Siap", pembatas garis horizontal abu-abu, serta footer info program di baris terbawah.</li>
</ul>
</td>
</tr>
<!-- ROW 27: SCREEN SCANNED -->
<tr>
<td>
<pre><code>void drawScannedScreen() {
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
}</code></pre>
</td>
<td>
<strong>Render Layar Verifikasi Pembayaran Sukses:</strong>
<p>Menggambar antarmuka layar dengan latar belakang hijau muda segar saat transaksi sukses terverifikasi. Sisi kiri menampilkan simbol lingkaran hijau berlapis dengan ikon gambar centang (checklist) putih tebal di bagian tengahnya. Sisi kanan menampilkan teks "Pembayaran Diterima!", "Memulai pengisian...", serta petunjuk kepada pelanggan untuk menunggu persiapan dispenser.</p>
</td>
</tr>
<!-- ROW 28: SCREEN PREPARE FILL -->
<tr>
<td>
<pre><code>void drawPrepareFillScreen(int current, int total) {
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
}</code></pre>
</td>
<td>
<strong>Render Layar Persiapan Peletakan Wadah (Prepare Fill):</strong>
<ul>
  <li>Mewarnai latar belakang layar dengan warna biru muda.</li>
  <li>Menggambar bentuk grafis ikon corong nozzle dispenser air (segitiga dan balok berwarna kuning) dan visual wadah botol galon (bentuk rounded rectangle biru dan cyan) di area tengah-kiri layar LCD.</li>
  <li>Di bagian kanan layar, menampilkan status nomor antrean galon yang sedang diisi (misal: "Galon 1 dari 2").</li>
  <li>Menuliskan instruksi langkah manual "Silakan tempatkan galon kosong Anda di bawah dispenser".</li>
  <li>Menampilkan indikator status titik lingkaran berwarna oranye dengan tulisan "Galon belum terdeteksi", menandakan bahwa sensor IR inframerah belum mendeteksi adanya objek wadah di nozzle dispenser.</li>
</ul>
</td>
</tr>
<!-- ROW 29: SCREEN CONFIRM FILL -->
<tr>
<td>
<pre><code>void drawConfirmFillScreen(int current, int total) {
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
}</code></pre>
</td>
<td>
<strong>Render Layar Konfirmasi Pengisian (Confirm Fill):</strong>
<ul>
  <li>Mewarnai layar dengan warna dasar hijau muda.</li>
  <li>Menggambar ikon centang checklist berwarna putih dalam lingkaran hijau di sisi kiri untuk menandakan posisi galon sudah pas dan terverifikasi oleh sensor IR.</li>
  <li>Menggambar ilustrasi fisik tombol tekan berbentuk rounded rectangle berwarna biru-muda lengkap dengan tulisan teks "OK".</li>
  <li>Di sisi kanan, memperbarui teks pemberitahuan menjadi "Galon Terdeteksi!" dan menginstruksikan pengguna untuk "Tekan TOMBOL di dispenser untuk memulai pengisian."</li>
  <li>Menampilkan indikator status dot hijau dengan keterangan "IR: Galon OK", mengindikasikan siap menerima input trigger dari tombol konfirmasi.</li>
</ul>
</td>
</tr>
<!-- ROW 30: SCREEN GALON LIFTED -->
<tr>
<td>
<pre><code>void drawGalonLiftedScreen() {
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
}</code></pre>
</td>
<td>
<strong>Render Layar Peringatan Galon Terangkat (Galon Lifted):</strong>
<ul>
  <li>Mewarnai latar belakang layar menjadi warna oranye terang (indikasi warning).</li>
  <li>Menggambar simbol peringatan bahaya level hardware berupa ikon segitiga kuning dengan tanda seru (!) di bagian dalamnya menggunakan kombinasi bentuk segitiga dan kotak kecil.</li>
  <li>Menuliskan teks peringatan berwarna putih tebal "Galon Terangkat! POMPA DIMATIKAN!".</li>
  <li>Memberikan instruksi pemulihan sistem (recovery): "Kembalikan galon, lalu tekan tombol untuk melanjutkan."</li>
</ul>
</td>
</tr>
<!-- ROW 31: SCREEN PROCESSING -->
<tr>
<td>
<pre><code>void drawProcessingScreen(int current, int total) {
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
}</code></pre>
</td>
<td>
<strong>Render Layar Proses Pengisian Aktif (Processing):</strong>
<ul>
  <li>Mewarnai latar belakang layar dengan warna biru muda.</li>
  <li>Menggambar ilustrasi galon yang sedang diisi dengan air (kotak galon biru-cyan dengan lingkaran-lingkaran kecil berwarna putih yang merepresentasikan gelembung udara/air).</li>
  <li>Membuat rounded badge di kanan atas untuk menampilkan status antrean (misal: "Galon 1/2").</li>
  <li>Menampilkan teks peringatan krusial "Jangan cabut galon!".</li>
  <li>Menggambar struktur awal progress bar kosong dan teks status estimasi waktu mundur di bagian bawah layar. Komponen visual progress bar ini nantinya diupdate isinya secara berkala dari looping utama.</li>
</ul>
</td>
</tr>
<!-- ROW 32: SCREEN CHECK NEXT -->
<tr>
<td>
<pre><code>void drawCheckNextScreen(int filled, int total) {
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
}</code></pre>
</td>
<td>
<strong>Render Layar Pengecekan Antrean Galon Selesai (Check Next):</strong>
<ul>
  <li>Mewarnai latar belakang layar LCD menjadi hijau muda.</li>
  <li>Menggambar ikon visual centang checklist sukses di sisi kiri layar.</li>
  <li>Menampilkan teks status jumlah galon yang sudah selesai terisi, contoh "1/2 Selesai!".</li>
  <li>Menghasilkan blok kondisional C++: Jika jumlah galon yang terisi (<code>filled</code>) masih kurang dari target pesanan (<code>total</code>), layar akan memunculkan petunjuk dinamis "Silakan ambil galon ini, lalu taruh galon ke-N", diiringi dot berwarna oranye bertuliskan status "Menunggu diambil..." (menunggu pembacaan sensor IR bernilai HIGH/galon diangkat).</li>
</ul>
</td>
</tr>
<!-- ROW 33: SCREEN DONE -->
<tr>
<td>
<pre><code>void drawDoneScreen() {
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
  tft.setTextColor(CLR_WHITE, CL_GREEN);
  tft.setCursor(rx, 55);
  tft.print("Pengisian");
  tft.setCursor(rx, 79);
  tft.print("Selesai!");
  tft.drawFastHLine(rx, 108, TFT_W - rx - 8, CLR_WHITE);
  tft.setTextSize(1);
  tft.setTextColor(CLR_WHITE, CL_GREEN);
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
}</code></pre>
</td>
<td>
<strong>Render Layar Akhir Transaksi Sukses (Done):</strong>
<ul>
  <li>Mewarnai latar belakang layar LCD menjadi hijau pekat (sukses total).</li>
  <li>Menggambar ikon visual lingkaran centang checklist sukses berdimensi besar dengan kombinasi garis warna putih tebal di sisi kiri layar.</li>
  <li>Menampilkan teks penutup besar "Pengisian Selesai!", dengan teks instruksi penutup "Silakan ambil galon Anda :)" dan ucapan terima kasih berwarna hijau muda cerah. Layar ini menandakan seluruh rangkaian proses transaksi pengisian dispenser otomatis telah selesai sepenuhnya.</li>
</ul>
</td>
</tr>
<!-- ROW 34: SCREEN ERROR -->
<tr>
<td>
<pre><code>void drawErrorScreen(const char* message) {
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
}</code></pre>
</td>
<td>
<strong>Render Layar Kesalahan Sistem (Error Screen):</strong>
<ul>
  <li>Mewarnai seluruh layar latar belakang dengan warna merah menyala (kondisi fatal).</li>
  <li>Menggambar ikon tanda bahaya berupa simbol segitiga kuning tebal dengan tanda seru (!) merah di bagian tengahnya.</li>
  <li>Menampilkan teks utama "Terjadi Kesalahan!" dengan garis pemisah horizontal berwarna kuning.</li>
  <li>Menampilkan teks isi parameter pesan kesalahan (<code>message</code>) secara dinamis dari program pemanggil.</li>
  <li>Menambahkan pesan pemulihan otomatis di bagian bawah: "Mencoba recovery... Menghubungi server...". Layar ini akan terus terkunci hingga server cloud memberikan respon status transaksi "SUCCESS" kembali.</li>
</ul>
</td>
</tr>
</tbody>
</table>
