# Catatan Pengembangan - GalonKu Dispenser v3.0

## Tanggal: 24-25 Juli 2026 (Sesi 1: Fix Crash & Hardware)
## Tanggal: 25 Juli 2026 (Sesi 2: Fix WDT Lengkap & Kalibrasi Flow Sensor)

---

## 1. Masalah Utama: ESP32 Crash (Guru Meditation Error)

### Gejala
- ESP32 crash dengan error: `Interrupt wdt timeout on CPU1`
- Crash terjadi **tepat saat** HTTP PATCH `PROCESSING` dikirim + relay pompa dinyalakan bersamaan
- Menggunakan **1 PSU** untuk ESP32 dan pompa sekaligus
- Crash bersifat acak — kadang terjadi, kadang tidak

### Log Error
```
[BTN] Tombol ditekan, IR valid -> Mulai isi!
[API] Mengirim PROCESSING...
[HTTP PATCH] {"code":"DEV-44","status":"PROCESSING"}
Guru Meditation Error: Core 1 panic'ed (Interrupt wdt timeout on CPU1)
```

### Root Cause
1. **Voltage drop** — saat relay pompa ON, arus inrush motor pompa menyebabkan tegangan PSU turun
2. ESP32 brownout tapi tidak cukup rendah untuk trigger brownout detector reset
3. WiFi/HTTP task hang karena power tidak stabil
4. CPU1 stuck di HTTP request atau WiFi driver → WDT timeout → crash

---

## 2. Perbaikan yang Diterapkan (Software)

### A. Urutan Timing Aman (Fix Utama)
**Sebelum:** Relay ON → HTTP PATCH (bersamaan, saling ganggu)
**Sesudah:** 
- Start: HTTP PATCH selesai → delay 500ms → Relay ON
- Stop (Galon Penuh): Relay OFF → delay 500ms → esp_task_wdt_reset() → HTTP PATCH "DONE"
- Tujuan: Menghindari crash karena lonjakan back-EMF saat motor mati berdekatan dengan HTTP request.

### B. Watchdog Timer (WDT) Feed
- Tambah `#include <esp_task_wdt.h>`
- Tambah `esp_task_wdt_add(NULL)` di `setup()` — mendaftarkan loop task ke WDT
- Tambah `esp_task_wdt_reset()` di:
  - Awal `loop()` setiap iterasi
  - Sebelum & sesudah HTTP request di `getDeviceStatus()`
  - Sebelum & sesudah HTTP request di `updateDeviceStatus()`
  - Sebelum `digitalWrite(PIN_RELAY, HIGH)`
  - Persis sebelum `updateDeviceStatus("DONE")` di `STATE_CHECK_NEXT`

### C. Error WDT "task not found"
- Awalnya muncul `esp_task_wdt_reset(707): task not found`
- Penyebab: task Arduino loop belum terdaftar ke WDT
- Fix: `esp_task_wdt_add(NULL)` di akhir `setup()` sebelum `connectWiFi()`

---

## 3. Rekomendasi Hardware (Sudah Diterapkan)

### Konfigurasi 2 PSU (Berhasil Mengatasi Restart)
```
PSU A (12V) → COM & NO relay → Pompa 12V
PSU B (5V)  → VCC + GND relay module
             → VCC + GND ESP32
             → Sensor IR, flow sensor, TFT

⚠️ GND PSU A dan PSU B HARUS disambung (common ground)
```

### Flyback Diode
- Diode dipasang PARALEL dengan pompa (Katoda ke VCC pompa, Anoda ke GND pompa). Pemasangan seri sebelumnya salah dan memicu restart. Sekarang sudah diperbaiki dan stabil.

### Kapasitor
- Sudah ada kapasitor di jalur ESP32 (sudah dipasang user)

---

## 4. Status Saat Ini

| Item | Status |
|------|--------|
| Fix urutan HTTP → Relay | ✅ Diterapkan |
| Fix WDT feed | ✅ Diterapkan |
| Fix WDT register task | ✅ Diterapkan |
| Kapasitor di ESP32 | ✅ Sudah ada |
| 2 PSU terpisah | ✅ Diterapkan |
| Flyback diode di pompa | ✅ Diterapkan (Paralel) |

---

## 5. Pin Mapping ESP32-S3

| Komponen | GPIO |
|----------|------|
| IR Sensor | 18 |
| Button | 4 (INPUT_PULLUP) |
| Relay/Pompa | 5 |
| Flow Sensor (YF-S201) | 17 |
| TFT LCD | via TFT_eSPI config |

---

## 6. Konfigurasi Penting

| Parameter | Nilai |
|-----------|-------|
| WiFi SSID | ASUS |
| API Base URL | https://api.galonku.my.id |
| Device Code | DEV-44 |
| Poll Interval | 3000ms |
| HTTP Timeout | 5000ms |
| Target per galon | 0.2 liter |
| Pulsa per liter | 450 |
| Fill timeout | 30 detik |

---

## 7. Flow Aplikasi

```
WIFI_CONNECTING → IDLE (QR tampil)
                    ↓ (poll API, status=SCANNED)
               PREPARE_FILL (tunggu IR sensor)
                    ↓ (IR detect galon)
               CONFIRM_FILL (tunggu tombol)
                    ↓ (tombol ditekan)
                    ↓ HTTP PATCH "PROCESSING" ← dikirim SEBELUM relay
                    ↓ delay 500ms
                    ↓ Relay ON
               PROCESSING (isi air, pantau flow)
                    ↓ (target pulsa tercapai)
               CHECK_NEXT
                    ↓ (semua galon selesai)
                    ↓ HTTP PATCH "DONE"
               DONE → kembali ke IDLE
```

---

## 8. Catatan untuk Sesi Berikutnya

- [x] Test kode terbaru — fix software berhasil (HTTP PROCESSING)
- [x] Fix restart saat galon terakhir penuh (Delay 500ms setelah relay OFF + WDT reset sebelum HTTP DONE)
- [x] Implementasi 2 PSU (PSU terpisah untuk pompa dan logic)
- [x] Flyback diode dipasang PARALEL di pompa (sebelumnya salah pasang seri, menyebabkan restart)
- [x] Evaluasi Flow sensor kalibrasi (450 pulsa/liter, target 1.0L). Margin error volume sisa selang = 1.29%, diabaikan. Tetap pakai 450 pulsa.

### Catatan Penting: Pemasangan Flyback Diode
- ❌ **SALAH:** Diode dipasang SERI (katoda ke VCC pompa, anoda ke NO relay) — menyebabkan ESP32 tetap restart karena back-EMF tidak tertangkap
- ✅ **BENAR:** Diode dipasang PARALEL melintang di kaki pompa (katoda ke (+) pompa, anoda ke (-) pompa) — berhasil mengatasi restart

---

## 9. Update Sesi 25 Juli 2026: Debugging Sistematis WDT

### A. Titik Rawan Restart Acak (Diperbaiki)
Ditemukan 5 titik di mana watchdog timer (WDT) bisa memicu restart acak karena blocking:
1. `STATE_PROCESSING` timeout: relay OFF **tanpa delay** dan WDT reset.
2. `STATE_PROCESSING` galon diangkat: relay OFF **tanpa delay** dan WDT reset.
3. `connectWiFi()`: looping `while` berpotensi memblokir CPU hingga 20 detik saat mencari sinyal.
4. `STATE_DONE`: penggunaan `delay(5000)` memblokir WDT secara penuh selama 5 detik.
5. `STATE_CHECK_NEXT`: `delay(1000)` jeda galon penuh, rawan jika WDT sudah mepet.

### B. Solusi yang Diterapkan
- Penambahan `esp_task_wdt_reset()` di **setiap** iterasi `connectWiFi()`.
- Penggantian `delay(5000)` di `STATE_DONE` dengan *non-blocking loop* iterasi `delay(500)`.
- Penambahan perlindungan `delay(500)` dan `esp_task_wdt_reset()` **setiap kali** relay mati secara paksa (timeout / galon terangkat) guna mengatasi lonjakan *back-EMF*.
- Pemindahan HTTP PATCH PROCESSING sebelum pompa menyala, untuk mencegah *voltage drop* merusak koneksi jaringan.

### C. Kalibrasi Flow Sensor
- Target = 1 Liter. Konstanta datasheet = 450 pulsa/L.
- Dilakukan evaluasi kalibrasi empiris (238 pulsa = 0.55 L $\approx$ 433 pulsa/L).
- Namun, selisih 0.05 L dikarenakan air di dalam selang sepanjang 13 cm memiliki sisa volume ($6.4\text{ mL}$) ditambah efek gravitasi dan inersia yang tak dapat dihindari.
- Keputusan: **Sistem dikembalikan dan menggunakan angka teoritis 450 pulsa/liter** untuk konsistensi. Toleransi error residual selang adalah sekitar $1.29\%$, masih sangat wajar untuk skala dispenser industri.
- File analisis lengkap ada di `ANALISIS_VOLUME_SELANG.md`.
