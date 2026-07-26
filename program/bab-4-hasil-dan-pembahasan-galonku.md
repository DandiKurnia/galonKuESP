# BAB IV
# HASIL DAN PEMBAHASAN

Bab ini menyajikan hasil implementasi dan pengujian prototipe **GalonKu**, yaitu sistem pengisian air galon otomatis berbasis *Internet of Things* (IoT). Pembahasan mencakup realisasi perangkat keras, implementasi perangkat lunak, integrasi sistem, serta hasil pengujian setiap fungsi utama. Data pada tabel pengujian harus diisi berdasarkan pengamatan dan hasil uji yang sebenarnya.

## 4.1 Implementasi Sistem

### 4.1.1 Implementasi Perangkat Keras

Implementasi perangkat keras dilakukan dengan merakit komponen berdasarkan rancangan pada Bab III. Komponen utama yang digunakan meliputi mikrokontroler ESP32-S3 sebagai pengendali, sensor inframerah sebagai pendeteksi keberadaan galon, sensor debit air YF-S201 untuk menghitung volume air yang dialirkan, modul relay sebagai pengendali beban, pompa air DC, layar TFT ILI9341 sebagai media informasi, tombol *push-button*, serta rangkaian catu daya *step-down*. Seluruh komponen dihubungkan sesuai skematik agar mampu bekerja sebagai satu kesatuan sistem.

ESP32-S3 menerima masukan dari sensor inframerah, sensor debit, dan tombol *push-button*. Berdasarkan masukan tersebut, mikrokontroler mengolah kondisi sistem dan memberikan keluaran berupa sinyal kendali kepada modul relay untuk mengaktifkan atau menonaktifkan pompa. Layar TFT ILI9341 menampilkan informasi seperti status koneksi, status galon, proses pengisian, volume air, serta status transaksi.

**Gambar 4.1. Implementasi perangkat keras GalonKu**  
> Sisipkan foto asli prototipe secara keseluruhan dengan latar yang rapi dan komponen utama terlihat jelas.

### 4.1.2 Implementasi Perangkat Lunak

Perangkat lunak pada sistem terdiri atas firmware pada ESP32-S3, aplikasi Android sebagai antarmuka pengguna, basis data PostgreSQL, serta layanan *Payment Gateway*. Firmware dikembangkan untuk membaca data sensor, menjalankan logika pengisian, mengendalikan relay, memperbarui tampilan TFT, dan berkomunikasi dengan layanan server melalui jaringan internet.

Aplikasi Android digunakan pengguna untuk memilih volume pengisian, memulai proses pembayaran, serta memantau status transaksi dan pengisian. Informasi pengguna, transaksi, dan riwayat pengisian disimpan pada basis data PostgreSQL. Setelah pembayaran berhasil diverifikasi oleh sistem, server mengirimkan status transaksi yang digunakan sebagai dasar untuk mengizinkan proses pengisian air.

**Gambar 4.2. Tampilan halaman utama aplikasi GalonKu**  
> Sisipkan tangkapan layar halaman utama aplikasi.

**Gambar 4.3. Tampilan pemilihan volume dan pembayaran**  
> Sisipkan tangkapan layar proses pemilihan volume dan pembayaran.

### 4.1.3 Integrasi Sistem

Integrasi sistem dilakukan dengan menghubungkan perangkat keras, aplikasi Android, basis data, dan layanan *Payment Gateway*. Pengguna melakukan pemilihan volume melalui aplikasi, kemudian sistem membuat transaksi pembayaran. Apabila status pembayaran telah berhasil, informasi transaksi diteruskan ke perangkat ESP32-S3 melalui layanan server.

Sebelum pompa diaktifkan, sensor inframerah memeriksa keberadaan galon pada area pengisian. Jika galon terdeteksi dan pembayaran telah berhasil, relay akan mengaktifkan pompa. Selama proses pengisian berlangsung, sensor debit YF-S201 mengirimkan pulsa ke ESP32-S3 untuk dihitung menjadi volume air. Pompa akan dihentikan ketika volume target tercapai atau ketika galon tidak lagi terdeteksi.

**Gambar 4.4. Sistem GalonKu saat proses pengisian**  
> Sisipkan foto proses pengisian yang memperlihatkan galon, pompa, dan tampilan status alat.

## 4.2 Pengujian Sistem

Pengujian sistem dilakukan untuk memastikan setiap fungsi bekerja sesuai tujuan perancangan. Setiap pengujian memuat tujuan, skenario, hasil pengamatan, dan analisis. Nilai pada tabel berikut merupakan template dan **wajib diganti dengan data hasil pengujian aktual**.

### 4.2.1 Pengujian Sensor Inframerah

**Tujuan pengujian.** Menguji kemampuan sensor inframerah dalam mendeteksi keberadaan galon pada area pengisian.

**Skenario pengujian.** Galon diletakkan pada area sensor dan kemudian dijauhkan dari area sensor. Status pembacaan sensor diamati melalui layar TFT, *Serial Monitor*, atau aplikasi.

**Tabel 4.1. Hasil pengujian sensor inframerah**

| No. | Kondisi objek/galon | Status sensor yang diharapkan | Status sensor hasil uji | Keterangan |
|---:|---|---|---|---|
| 1 | Galon berada di area deteksi | Terdeteksi | [Isi hasil] | [Isi keterangan] |
| 2 | Galon tidak berada di area deteksi | Tidak terdeteksi | [Isi hasil] | [Isi keterangan] |
| 3 | Galon diletakkan berulang kali | Status berubah sesuai kondisi | [Isi hasil] | [Isi keterangan] |

Berdasarkan Tabel 4.1, sensor inframerah [isi kesimpulan berdasarkan data]. Apabila seluruh kondisi menghasilkan status yang sesuai, sensor dapat dinyatakan mampu mendukung fungsi pengaman agar pompa hanya bekerja ketika galon berada pada posisi pengisian.

### 4.2.2 Pengujian Sensor Debit Air YF-S201

**Tujuan pengujian.** Mengukur kemampuan sensor YF-S201 dalam menghitung volume air yang dialirkan dan membandingkannya dengan volume hasil pengukuran menggunakan alat ukur pembanding.

**Skenario pengujian.** Sistem dijalankan pada beberapa target volume. Volume yang terbaca pada sistem dibandingkan dengan volume aktual yang diukur menggunakan gelas ukur atau wadah berskala.

**Tabel 4.2. Hasil pengujian akurasi sensor debit air**

| No. | Target volume (mL) | Volume terbaca sistem (mL) | Volume aktual (mL) | Selisih (mL) | Galat (%) | Keterangan |
|---:|---:|---:|---:|---:|---:|---|
| 1 | [Isi] | [Isi] | [Isi] | [Isi] | [Isi] | [Isi] |
| 2 | [Isi] | [Isi] | [Isi] | [Isi] | [Isi] | [Isi] |
| 3 | [Isi] | [Isi] | [Isi] | [Isi] | [Isi] | [Isi] |
| 4 | [Isi] | [Isi] | [Isi] | [Isi] | [Isi] | [Isi] |
| 5 | [Isi] | [Isi] | [Isi] | [Isi] | [Isi] | [Isi] |

Perhitungan galat dapat dilakukan menggunakan persamaan berikut.

$$\text{Galat} = \left|\frac{\text{Volume terbaca sistem} - \text{Volume aktual}}{\text{Volume aktual}}\right| \times 100\%$$

Berdasarkan Tabel 4.2, hasil pengujian menunjukkan bahwa [isi analisis: rata-rata galat, volume dengan galat tertinggi, dan kemungkinan penyebabnya]. Hasil tersebut dapat digunakan untuk menentukan apakah nilai kalibrasi sensor debit perlu disesuaikan.

### 4.2.3 Pengujian Tombol *Push-Button*

**Tujuan pengujian.** Memastikan tombol *push-button* dapat memberikan masukan kepada ESP32-S3 sesuai fungsi yang dirancang, misalnya untuk memulai, menghentikan, atau memilih proses pengisian.

**Tabel 4.3. Hasil pengujian tombol *push-button***

| No. | Aksi tombol | Respons yang diharapkan | Respons hasil uji | Keterangan |
|---:|---|---|---|---|
| 1 | Tombol ditekan sekali | Sistem menjalankan fungsi yang dipilih | [Isi hasil] | [Isi keterangan] |
| 2 | Tombol ditekan saat kondisi siaga | Status sistem berubah sesuai program | [Isi hasil] | [Isi keterangan] |
| 3 | Tombol ditekan saat pengisian | Sistem merespons sesuai logika program | [Isi hasil] | [Isi keterangan] |

Berdasarkan Tabel 4.3, tombol *push-button* [isi hasil analisis]. Pengujian ini menunjukkan bahwa pengguna dapat memberikan perintah lokal pada perangkat tanpa harus menggunakan aplikasi.

### 4.2.4 Pengujian Pompa dan Relay

**Tujuan pengujian.** Memastikan modul relay dapat mengendalikan pompa air berdasarkan perintah dari ESP32-S3 dan kondisi sistem.

**Tabel 4.4. Hasil pengujian pompa dan relay**

| No. | Kondisi pengujian | Status relay yang diharapkan | Status pompa yang diharapkan | Hasil uji | Keterangan |
|---:|---|---|---|---|---|
| 1 | Transaksi belum berhasil | Tidak aktif | Mati | [Isi hasil] | [Isi keterangan] |
| 2 | Galon tidak terdeteksi | Tidak aktif | Mati | [Isi hasil] | [Isi keterangan] |
| 3 | Transaksi berhasil dan galon terdeteksi | Aktif | Menyala | [Isi hasil] | [Isi keterangan] |
| 4 | Volume target tercapai | Tidak aktif | Mati | [Isi hasil] | [Isi keterangan] |

Berdasarkan Tabel 4.4, relay dan pompa [isi analisis berdasarkan hasil uji]. Mekanisme penghentian pompa ketika target volume tercapai merupakan bagian penting untuk mencegah pengisian berlebih.

### 4.2.5 Pengujian Layar TFT ILI9341

**Tujuan pengujian.** Memastikan layar TFT mampu menampilkan informasi operasional sistem secara jelas.

**Tabel 4.5. Hasil pengujian tampilan TFT ILI9341**

| No. | Informasi yang ditampilkan | Tampilan yang diharapkan | Hasil uji | Keterangan |
|---:|---|---|---|---|
| 1 | Status koneksi sistem | Status koneksi ditampilkan | [Isi hasil] | [Isi keterangan] |
| 2 | Status galon | Terdeteksi/tidak terdeteksi ditampilkan | [Isi hasil] | [Isi keterangan] |
| 3 | Volume pengisian | Nilai volume ditampilkan | [Isi hasil] | [Isi keterangan] |
| 4 | Status pompa | Menyala/mati ditampilkan | [Isi hasil] | [Isi keterangan] |
| 5 | Status transaksi | Berhasil/menunggu/gagal ditampilkan | [Isi hasil] | [Isi keterangan] |

Berdasarkan Tabel 4.5, layar TFT ILI9341 [isi analisis]. Informasi pada layar membantu pengguna dan operator memantau keadaan alat secara langsung di lokasi pengisian.

### 4.2.6 Pengujian Aplikasi dan *Payment Gateway*

**Tujuan pengujian.** Memastikan aplikasi dapat membuat transaksi, menampilkan status pembayaran, dan meneruskan informasi yang diperlukan untuk proses pengisian.

**Tabel 4.6. Hasil pengujian aplikasi dan *Payment Gateway***

| No. | Skenario pengujian | Hasil yang diharapkan | Hasil uji | Keterangan |
|---:|---|---|---|---|
| 1 | Pengguna memilih volume pengisian | Pilihan volume tersimpan pada transaksi | [Isi hasil] | [Isi keterangan] |
| 2 | Pengguna melakukan pembayaran | Sistem menampilkan proses pembayaran | [Isi hasil] | [Isi keterangan] |
| 3 | Pembayaran berhasil | Status transaksi berubah menjadi berhasil | [Isi hasil] | [Isi keterangan] |
| 4 | Pembayaran belum berhasil/gagal | Pompa tidak dapat diaktifkan | [Isi hasil] | [Isi keterangan] |
| 5 | Transaksi berhasil dan galon tersedia | Perintah pengisian dapat dijalankan | [Isi hasil] | [Isi keterangan] |

Berdasarkan Tabel 4.6, integrasi aplikasi dan *Payment Gateway* [isi analisis]. Pengujian perlu dilakukan menggunakan transaksi uji atau lingkungan *sandbox* apabila sistem belum menggunakan akun produksi.

### 4.2.7 Pengujian Sistem Keseluruhan

**Tujuan pengujian.** Memastikan seluruh komponen bekerja secara terintegrasi dari pemilihan volume hingga pengisian selesai.

**Tabel 4.7. Hasil pengujian sistem keseluruhan**

| No. | Skenario | Kondisi awal | Hasil yang diharapkan | Hasil uji | Status |
|---:|---|---|---|---|---|
| 1 | Pengisian dengan transaksi berhasil | Galon tersedia, sistem terhubung | Pompa mengisi hingga target volume | [Isi hasil] | [Berhasil/Belum] |
| 2 | Galon tidak tersedia | Sistem terhubung, transaksi berhasil | Pompa tidak menyala | [Isi hasil] | [Berhasil/Belum] |
| 3 | Pembayaran belum berhasil | Galon tersedia | Pompa tidak menyala | [Isi hasil] | [Berhasil/Belum] |
| 4 | Target volume tercapai | Pompa sedang menyala | Pompa berhenti otomatis | [Isi hasil] | [Berhasil/Belum] |
| 5 | Koneksi internet terganggu | Sistem sedang digunakan | Sistem memberikan status sesuai rancangan | [Isi hasil] | [Berhasil/Belum] |

Berdasarkan pengujian pada Tabel 4.7, sistem GalonKu [isi simpulan pengujian secara faktual]. Fungsi yang telah berjalan baik dapat dijelaskan berdasarkan bukti hasil pengujian, sedangkan bagian yang masih memerlukan pengembangan dapat dijelaskan secara konstruktif, misalnya melalui peningkatan kalibrasi sensor, penguatan koneksi internet, atau penyempurnaan antarmuka aplikasi.

## 4.3 Pembahasan

Hasil implementasi menunjukkan bahwa GalonKu mengintegrasikan perangkat sensor, aktuator, aplikasi Android, basis data, dan layanan pembayaran dalam satu alur kerja. Sensor inframerah berfungsi sebagai masukan untuk memastikan keberadaan galon, sedangkan sensor debit YF-S201 digunakan untuk memantau volume air selama proses pengisian. ESP32-S3 memproses seluruh masukan tersebut dan mengendalikan relay sebagai penghubung kendali pompa.

Berdasarkan hasil pengujian, keberhasilan sistem perlu dinilai dari kesesuaian respons alat terhadap setiap skenario. Sistem dapat dinyatakan berjalan sesuai rancangan apabila pompa hanya aktif saat pembayaran berhasil dan galon terdeteksi, volume pengisian dihentikan ketika mencapai target, serta informasi status dapat ditampilkan pada aplikasi dan layar TFT. Uraian ini harus disesuaikan kembali dengan hasil pada Tabel 4.1 sampai Tabel 4.7.

Kelebihan sistem adalah adanya otomatisasi proses pengisian dan pencatatan transaksi secara digital. Namun, kinerja sistem dapat dipengaruhi oleh ketelitian kalibrasi sensor debit, kestabilan koneksi internet, kualitas catu daya, serta respons layanan server dan *Payment Gateway*. Oleh karena itu, hasil pengujian yang tidak sesuai tidak perlu ditulis sebagai kegagalan mutlak, melainkan sebagai dasar pengembangan untuk meningkatkan kinerja sistem pada tahap berikutnya.
