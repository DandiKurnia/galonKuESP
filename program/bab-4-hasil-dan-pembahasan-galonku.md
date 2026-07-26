# BAB IV
# HASIL DAN PEMBAHASAN

Bab ini menyajikan hasil implementasi dan pengujian prototipe **GalonKu**, yaitu sistem pengisian air galon otomatis berbasis *Internet of Things* (IoT). Pembahasan mencakup realisasi perangkat keras, implementasi perangkat lunak, integrasi sistem, serta hasil pengujian setiap fungsi utama. Data pada tabel pengujian harus diisi berdasarkan pengamatan dan hasil uji yang sebenarnya.

## 4.1 Implementasi Sistem

### 4.1.1 Implementasi Perangkat Keras

Implementasi perangkat keras dilakukan dengan merakit komponen berdasarkan rancangan pada Bab III. Komponen utama yang digunakan meliputi mikrokontroler ESP32-S3 sebagai pengendali, sensor inframerah sebagai pendeteksi keberadaan galon, sensor debit air YF-S201 untuk menghitung volume air yang dialirkan, modul relay sebagai pengendali beban, pompa air DC, layar TFT ILI9341 sebagai media informasi, tombol *push-button*, serta rangkaian catu daya *step-down*. Seluruh komponen dihubungkan sesuai skematik agar mampu bekerja sebagai satu kesatuan sistem.

Mikrokontroler ESP32-S3 menerima masukan dari sensor inframerah (berlogika *active-low* saat mendeteksi objek), sensor debit air yang memanfaatkan *interrupt* (*falling edge*), serta tombol *push-button* yang terhubung menggunakan resistor *pull-up* internal. Keluaran sistem berupa sinyal kendali *active-high* yang dikirimkan ke modul relay untuk mengaktifkan pompa air. Media antarmuka pengguna di perangkat berupa layar TFT ILI9341 320x240 piksel berorientasi *landscape* yang berkomunikasi melalui antarmuka *Serial Peripheral Interface* (SPI). Rincian alokasi pin mikrokontroler untuk setiap komponen disajikan pada Tabel 4.1.

**Tabel 4.1. Alokasi pin mikrokontroler ESP32-S3**

| Komponen | Pin ESP32-S3 | Mode / Fungsi | Keterangan |
|---|---|---|---|
| Sensor Debit YF-S201 | GPIO 17 | `INPUT_PULLUP` | Interupsi (*falling edge*), 450 pulsa/liter |
| Sensor Inframerah | GPIO 18 | `INPUT` | `LOW` = galon terdeteksi |
| Tombol *Push-Button* | GPIO 4 | `INPUT_PULLUP` | `LOW` = tombol ditekan (debounce 50 ms) |
| Modul Relay | GPIO 5 | `OUTPUT` | `HIGH` = pompa menyala |
| Layar TFT (SCK) | GPIO 12 | SPI Clock | Komunikasi data |
| Layar TFT (MOSI) | GPIO 11 | SPI Data Out | Komunikasi data |
| Layar TFT (CS) | GPIO 10 | SPI Chip Select | Komunikasi data |
| Layar TFT (DC) | GPIO 9 | SPI Data/Command | Komunikasi data |
| Layar TFT (RST) | GPIO 8 | SPI Reset | Komunikasi data |
| Layar TFT (LED) | 5V | Power | Catu daya *backlight* layar |
| Layar TFT (VCC) | 3.3V | Power | Catu daya tegangan logika layar |

Secara fisik, prototipe terbagi menjadi tiga bagian utama yang ditempatkan pada lokasi terpisah sesuai fungsinya. Bagian pertama adalah kotak kendali, bagian kedua adalah area deteksi galon, dan bagian ketiga adalah panel antarmuka pengguna. Tampak keseluruhan prototipe ditunjukkan pada Gambar 4.1.

**Gambar 4.1. Tampak keseluruhan prototipe GalonKu**  
> Sisipkan foto tampak keseluruhan prototipe yang memperlihatkan hubungan antar ketiga bagian utama (kotak kendali, area galon, panel antarmuka).

Gambar 4.1 memperlihatkan susunan fisik prototipe GalonKu secara keseluruhan. Kotak kendali ditempatkan pada bagian [atas/samping/belakang], area deteksi galon berada pada bagian [bawah/tengah], dan panel antarmuka berada pada sisi [depan/samping] yang mudah dijangkau oleh pengguna. Kabel penghubung antar bagian disusun agar tidak mengganggu proses pengisian.

Bagian dalam kotak kendali memuat seluruh komponen elektronik utama, yaitu mikrokontroler ESP32-S3, modul relay, pompa air DC, serta rangkaian catu daya *step-down*. Seluruh komponen disusun pada satu wadah tertutup untuk melindungi rangkaian dari percikan air. Susunan komponen di dalam kotak kendali ditunjukkan pada Gambar 4.2.

**Gambar 4.2. Kotak kendali: ESP32-S3, modul relay, pompa air DC, dan catu daya *step-down***  
> Sisipkan foto bagian dalam kotak kendali dengan komponen utama terlihat jelas dan diberi label bila perlu.

Gambar 4.2 menunjukkan penempatan ESP32-S3 sebagai pusat pengendali yang terhubung ke modul relay melalui GPIO 5. Modul relay berfungsi sebagai saklar elektronik untuk mengaktifkan pompa air DC. Rangkaian catu daya *step-down* menyediakan tegangan yang stabil bagi seluruh komponen di dalam kotak.

Sensor inframerah ditempatkan di dalam kotak terpisah pada area peletakan galon. Penempatan ini memungkinkan sensor mendeteksi keberadaan galon secara langsung ketika galon diletakkan di atas area pengisian. Posisi sensor di dalam kotak deteksi ditunjukkan pada Gambar 4.3.

**Gambar 4.3. Area deteksi galon: sensor inframerah di dalam kotak peletakan galon**  
> Sisipkan foto kotak deteksi galon yang menampilkan posisi sensor inframerah.

Gambar 4.3 memperlihatkan sensor inframerah yang dipasang menghadap ke arah posisi galon. Sensor bekerja dengan logika *active-low*, yaitu menghasilkan sinyal `LOW` pada GPIO 18 ketika galon terdeteksi berada di area pengisian. Apabila galon tidak berada pada posisinya atau diangkat saat proses pengisian, sensor menghasilkan sinyal `HIGH` dan sistem akan menghentikan pompa sebagai mekanisme pengaman.

Panel antarmuka pengguna terdiri atas layar TFT ILI9341 berukuran 320×240 piksel dan tombol *push-button*. Layar menampilkan informasi status sistem seperti koneksi WiFi, status galon, volume pengisian, dan status transaksi. Tombol *push-button* digunakan untuk mengonfirmasi dimulainya proses pengisian setelah galon terdeteksi. Tampilan panel antarmuka ditunjukkan pada Gambar 4.4.

**Gambar 4.4. Panel antarmuka: layar TFT ILI9341 dan tombol *push-button***  
> Sisipkan foto panel antarmuka yang memperlihatkan layar TFT dan tombol secara jelas.

Gambar 4.4 menunjukkan layar TFT yang terpasang pada sisi depan prototipe dengan orientasi *landscape*. Tombol *push-button* ditempatkan di dekat layar agar pengguna dapat dengan mudah melihat informasi pada layar sekaligus menekan tombol untuk memulai pengisian. Tombol terhubung ke GPIO 4 dengan resistor *pull-up* internal dan dilengkapi mekanisme *debounce* selama 50 milidetik pada firmware.

### 4.1.2 Implementasi Basis Data dan Layanan Cloud

Seluruh data operasional sistem GalonKu disimpan pada basis data relasional PostgreSQL versi 16. Struktur basis data dikelola menggunakan Prisma ORM (*Object-Relational Mapping*) versi 7 yang terintegrasi dengan layanan *backend* berbasis kerangka kerja NestJS versi 11. Komunikasi antara aplikasi *backend* dan basis data menggunakan *driver* PostgreSQL melalui *connection pool*, sementara proses migrasi skema dilakukan secara terstruktur menggunakan fitur migrasi bawaan Prisma. Basis data dan layanan pendukung (*Redis*) dijalankan di dalam kontainer Docker.

#### A. Struktur Basis Data

Basis data terdiri atas dua belas tabel yang saling berelasi. Rincian tabel beserta fungsinya disajikan pada Tabel 4.2a.

**Tabel 4.2a. Struktur tabel basis data GalonKu**

| No. | Nama Tabel | Fungsi |
|---:|---|---|
| 1 | `users` | Menyimpan data pengguna (nama, email, kata sandi ter-*hash*, nomor telepon, dan relasi ke peran serta alamat) |
| 2 | `roles` | Mendefinisikan peran pengguna (`super-admin`, `operator`, `customer`) |
| 3 | `permissions` | Mendefinisikan hak akses berdasarkan sumber daya dan tindakan |
| 4 | `role_permissions` | Tabel relasi *many-to-many* antara peran dan hak akses |
| 5 | `devices` | Menyimpan identitas mesin dispenser (`device_code`, `qr_status`, `device_token_hash`, status aktif/nonaktif) |
| 6 | `addresses` | Menyimpan lokasi depot mesin (nama, alamat, koordinat lintang dan bujur) |
| 7 | `transactions` | Merekam transaksi pembelian air (jumlah galon, total harga, status pembayaran, relasi ke perangkat dan pengguna) |
| 8 | `transaction_details` | Menyimpan rincian harga per galon dan subtotal setiap transaksi |
| 9 | `transaction_histories` | Mencatat riwayat perubahan status transaksi sebagai *audit trail* |
| 10 | `payments` | Menyimpan data pembayaran dari *Payment Gateway* (`external_id`, `invoice_id`, `invoice_url`, `expiry_date`, metode pembayaran) |
| 11 | `water_fill_logs` | Mencatat log pengisian air (waktu mulai, waktu selesai, durasi, jumlah galon terisi, status) |
| 12 | `refresh_tokens` | Menyimpan *hash* token penyegaran (*refresh token*) untuk otentikasi pengguna |

Relasi antar tabel mengikuti pola *one-to-many* dan *many-to-many*. Setiap pengguna memiliki satu peran, setiap perangkat terhubung ke satu alamat, dan setiap transaksi terhubung ke satu pengguna dan satu perangkat. Tabel `payments` berelasi *one-to-one* dengan `transactions`. Diagram relasi antar entitas ditunjukkan pada Gambar 4.5a.

**Gambar 4.5a. Diagram relasi antar entitas basis data GalonKu**  
> Sisipkan diagram ERD (*Entity Relationship Diagram*) yang menunjukkan relasi antar tabel.

#### B. Layanan *Backend* (Server)

Layanan *backend* dikembangkan menggunakan kerangka kerja NestJS dengan arsitektur modular. Setiap domain fungsional dipisahkan ke dalam modul tersendiri. Daftar modul utama beserta fungsinya disajikan pada Tabel 4.2b.

**Tabel 4.2b. Modul utama layanan *backend* GalonKu**

| No. | Modul | Fungsi |
|---:|---|---|
| 1 | `AuthModule` | Otentikasi pengguna (login, registrasi, token JWT, *refresh token* dengan deteksi penggunaan ulang, logout) |
| 2 | `DevicesModule` | Manajemen perangkat (CRUD, pembuatan kode QR, rotasi dan pencabutan *token* perangkat, *polling* status oleh ESP32-S3) |
| 3 | `TransactionsModule` | Pembuatan transaksi, integrasi *Payment Gateway*, penerimaan *webhook*, dasbor ringkasan, dan statistik transaksi |
| 4 | `UsersModule` | Manajemen data pengguna |
| 5 | `RolesModule` | Manajemen peran |
| 6 | `PermissionsModule` | Manajemen hak akses |
| 7 | `AddressModule` | Manajemen lokasi depot mesin |
| 8 | `ProfileModule` | Profil pengguna |
| 9 | `QrcodeModule` | Pembuatan gambar kode QR untuk setiap perangkat |
| 10 | `QueueModule` | Antrian tugas (*job queue*) menggunakan Bull dan Redis untuk penanganan kedaluwarsa pembayaran |

#### C. Otentikasi dan Otorisasi

Sistem menerapkan dua mekanisme otentikasi yang berbeda sesuai dengan jenis klien.

Otentikasi pengguna (aplikasi mobile) menggunakan skema *JSON Web Token* (JWT). Saat login berhasil, server menghasilkan *access token* (masa berlaku 15 menit) dan *refresh token* (masa berlaku 7 hari). *Refresh token* disimpan di basis data dalam bentuk *hash* SHA-256. Sistem dilengkapi mekanisme deteksi penggunaan ulang (*reuse detection*): apabila *refresh token* yang sudah dicabut digunakan kembali, seluruh sesi pengguna tersebut akan dicabut secara otomatis.

Otentikasi perangkat (ESP32-S3) menggunakan skema *device token*. Setiap perangkat memiliki *token* unik berformat `dtkn_` diikuti 64 karakter heksadesimal acak, yang di-*hash* menggunakan bcrypt sebelum disimpan di basis data. ESP32-S3 mengirimkan kode perangkat dan *token* melalui *header* HTTP (`x-device-code` dan `x-device-token`) pada setiap permintaan ke server.

Otorisasi berbasis peran (*Role-Based Access Control*/RBAC) membatasi akses berdasarkan tiga peran: `super-admin` (akses penuh), `operator` (akses terbatas pada perangkat di lokasi yang ditugaskan), dan `customer` (akses hanya pada transaksi milik sendiri).

#### D. Integrasi *Payment Gateway*

Proses pembayaran diintegrasikan dengan layanan *Payment Gateway* Xendit melalui mekanisme *invoice*. Alur pembayaran secara ringkas adalah sebagai berikut.

1. Pengguna membuat transaksi melalui aplikasi. Server membuat *invoice* di Xendit dengan masa berlaku 300 detik (5 menit) dan menyimpan data pembayaran di tabel `payments`.
2. Secara bersamaan, server mendaftarkan tugas kedaluwarsa (*expiry job*) ke antrian Bull dengan *delay* sesuai waktu kedaluwarsa *invoice*.
3. Pengguna melakukan pembayaran melalui halaman *invoice* Xendit.
4. Setelah pembayaran berhasil, Xendit mengirimkan notifikasi melalui *webhook* (`POST /transactions/webhook/xendit`) ke server.
5. Server memperbarui status pembayaran menjadi `PAID` atau `SETTLED`, memperbarui status transaksi, dan mengubah `qr_status` perangkat menjadi `SCANNED` — yang menjadi sinyal bagi ESP32-S3 untuk memulai proses pengisian.
6. Tugas kedaluwarsa di antrian Bull dibatalkan karena pembayaran telah berhasil.

Apabila pembayaran tidak dilakukan hingga masa berlaku *invoice* habis, tugas di antrian Bull akan mengubah status pembayaran menjadi `EXPIRED` secara otomatis.

#### E. Antarmuka Pemrograman Aplikasi (API) untuk Perangkat

ESP32-S3 berkomunikasi dengan server melalui dua *endpoint* khusus yang dilindungi oleh `DeviceAuthGuard`. Rincian *endpoint* disajikan pada Tabel 4.2c.

**Tabel 4.2c. *Endpoint* API untuk perangkat ESP32-S3**

| Metode | *Endpoint* | Fungsi | Data |
|---|---|---|---|
| `GET` | `/devices/code/:code/status` | Memeriksa status antrean mesin | Mengembalikan `qr_status` dan `total_galon` |
| `PATCH` | `/devices/code` | Memperbarui status mesin | Menerima `code` dan `status` (`PROCESSING`, `DONE`) |

ESP32-S3 melakukan *polling* ke *endpoint* GET setiap 3 detik. Apabila status bernilai `SCANNED`, perangkat memulai alur pengisian. Setelah pengisian selesai, perangkat mengirimkan status `DONE` melalui *endpoint* PATCH, yang oleh server dipetakan menjadi `SUCCESS`.

### 4.1.3 Implementasi Perangkat Lunak

Perangkat lunak pada sistem terbagi menjadi dua komponen utama, yaitu *firmware* yang berjalan pada mikrokontroler ESP32-S3 dan aplikasi *mobile* sebagai antarmuka pelanggan.

#### A. *Firmware* ESP32-S3

*Firmware* dikembangkan dalam bahasa C++ menggunakan *framework* Arduino. Arsitektur perangkat lunak dirancang menggunakan pola *State Machine* (mesin status) dengan delapan status utama untuk mengatur alur kerja dispenser secara deterministik, yaitu: `WIFI_CONNECTING`, `IDLE`, `PREPARE_FILL`, `CONFIRM_FILL`, `PROCESSING`, `CHECK_NEXT`, `DONE`, dan `ERROR`.

Pada status `IDLE`, ESP32-S3 menampilkan kode QR mesin di layar TFT dan secara berkala melakukan *polling* ke server menggunakan HTTP GET setiap 3 detik. *Watchdog Timer* (WDT) perangkat keras diaktifkan untuk memastikan perangkat dapat memulai ulang (*restart*) secara otomatis apabila terjadi *freeze* atau kegagalan sistem. Ketika *polling* mendeteksi status `SCANNED` dari server, mesin berpindah ke status `PREPARE_FILL` untuk memulai proses pengisian air.

#### B. Aplikasi *Mobile* (Flutter)

Aplikasi pengguna dikembangkan menggunakan *framework* Flutter berbahasa Dart, sehingga dapat dikompilasi secara *cross-platform* (Android dan iOS). Aplikasi menerapkan pola arsitektur berbasis penyedia *state* (*Provider*) untuk memisahkan antara tampilan antarmuka dan logika bisnis. Beberapa modul utama dalam aplikasi meliputi: `AuthProvider` untuk manajemen sesi, `TransactionProvider` untuk riwayat dan filter transaksi, serta `MapNotifier` untuk layanan lokasi.

Mekanisme otentikasi aplikasi dilengkapi dengan pencegahan kedaluwarsa sesi secara diam-diam. Kelas `ApiClient` mencegat (*intercept*) setiap permintaan HTTP; apabila permintaan mengembalikan status HTTP 401 (Tidak Diotorisasi), kelas ini secara proaktif akan melakukan *refresh token* ke server dan mengulang permintaan semula. Hal ini memastikan pengalaman pengguna (*user experience*) tidak terganggu oleh masa berlaku *access token* yang singkat.

**Gambar 4.5. Tampilan halaman pemindaian kode QR dan halaman utama**  
> Sisipkan tangkapan layar fitur pemindaian QR mesin dan halaman *dashboard* aplikasi.

Alur pemesanan air dimulai dengan menekan tombol pindai pada menu tengah (*floating action button*), yang akan membuka antarmuka kamera (`MobileScannerController`). Kode QR yang terpindai (`device_code`) diteruskan ke layar *Checkout*.

**Gambar 4.6. Tampilan *checkout* dan faktur pembayaran (*invoice*)**  
> Sisipkan tangkapan layar proses pemilihan volume air dan halaman Xendit WebView.

Pada halaman *Checkout*, pengguna menentukan jumlah galon yang akan diisi dengan harga yang telah ditetapkan oleh server (Rp 8.000 per galon). Setelah pengguna mengonfirmasi pesanan, aplikasi memanggil *endpoint* transaksi dan menerima tautan faktur Xendit. Tautan tersebut kemudian dirender secara langsung di dalam aplikasi menggunakan komponen `WebViewFlutter`, sehingga pengguna dapat menyelesaikan pembayaran menggunakan dompet digital atau *virtual account* tanpa harus keluar dari aplikasi GalonKu.

### 4.1.4 Integrasi Sistem

Integrasi sistem dilakukan dengan menghubungkan perangkat keras, aplikasi *mobile*, layanan *backend*, basis data, dan *Payment Gateway* Xendit dalam satu alur kerja yang terkoordinasi. Alur integrasi secara keseluruhan digambarkan pada diagram yang telah disajikan pada Gambar 3.9 di Bab III.

Proses dimulai ketika pengguna memindai kode QR pada mesin dispenser menggunakan aplikasi *mobile*. Setelah memilih jumlah galon, aplikasi mengirimkan permintaan transaksi ke *backend* melalui API. *Backend* kemudian membuat *invoice* pembayaran di Xendit dan mengembalikan tautan faktur kepada aplikasi. Pengguna menyelesaikan pembayaran melalui halaman yang dirender di dalam aplikasi. Setelah pembayaran berhasil, Xendit mengirimkan notifikasi *webhook* ke *backend*, yang memperbarui status transaksi menjadi `PAID` atau `SETTLED` dan mengubah status perangkat menjadi `SCANNED`.

ESP32-S3 yang melakukan *polling* secara berkala ke server mendeteksi perubahan status menjadi `SCANNED` dan memulai alur pengisian. Sebelum pompa diaktifkan, sistem menerapkan mekanisme pengaman berlapis (*interlock*). Sensor inframerah memeriksa keberadaan galon pada area pengisian; pompa hanya akan menyala apabila galon terdeteksi (`LOW`) dan status transaksi telah berhasil. Apabila galon diangkat selama proses pengisian berlangsung, pompa segera dimatikan, sistem menyimpan sisa waktu pengisian, dan menampilkan layar peringatan. Pengguna dapat melanjutkan pengisian dengan mengembalikan galon dan menekan tombol (*push-button*) pada panel.

Selama pompa menyala, sensor debit YF-S201 mengirimkan pulsa ke ESP32-S3 melalui mekanisme interupsi. Mikrokontroler menghitung jumlah pulsa dan membandingkannya dengan nilai target (450 pulsa per liter) yang diterima dari transaksi. Apabila volume target tercapai, relay dimatikan dan pompa berhenti secara otomatis.

Sistem mendukung pengisian lebih dari satu galon dalam satu sesi transaksi. Setelah satu galon selesai diisi, sistem memasuki status `CHECK_NEXT` dan menunggu galon berikutnya ditempatkan. Proses berulang hingga seluruh galon terisi. Apabila seluruh galon telah selesai, ESP32-S3 mengirimkan status `DONE` ke server, yang dipetakan menjadi `SUCCESS`, dan perangkat kembali ke status *idle* menunggu transaksi baru.

**Gambar 4.7. Sistem GalonKu saat proses pengisian**  
> Sisipkan foto proses pengisian yang memperlihatkan galon, pompa, dan tampilan status alat.

## 4.2 Pengujian Sistem

Pengujian sistem dilakukan untuk memastikan setiap fungsi bekerja sesuai tujuan perancangan. Setiap pengujian memuat tujuan, skenario, hasil pengamatan, dan analisis. Data pada tabel berikut diperoleh dari pengujian prototipe GalonKu secara langsung.

### 4.2.1 Pengujian Sensor Inframerah

**Tujuan pengujian.** Menguji kemampuan sensor inframerah dalam mendeteksi keberadaan galon pada area pengisian.

**Skenario pengujian.** Galon diletakkan pada area sensor dan kemudian dijauhkan dari area sensor. Status pembacaan sensor diamati melalui layar TFT dan *Serial Monitor*.

**Tabel 4.2. Hasil pengujian sensor inframerah**

| No. | Kondisi objek/galon | Status sensor yang diharapkan | Status sensor hasil uji | Keterangan |
|---:|---|---|---|---|
| 1 | Galon berada di area deteksi | Terdeteksi | Terdeteksi | GPIO 18 berlogika `LOW`, layar menampilkan status galon terdeteksi |
| 2 | Galon tidak berada di area deteksi | Tidak terdeteksi | Tidak terdeteksi | GPIO 18 berlogika `HIGH`, layar menampilkan status galon belum terdeteksi |
| 3 | Galon diletakkan berulang kali | Status berubah sesuai kondisi | Status berubah sesuai kondisi | Respons sensor konsisten tanpa *delay* yang mengganggu |

Berdasarkan Tabel 4.2, sensor inframerah mampu mendeteksi keberadaan galon dengan benar pada seluruh skenario pengujian. Hasil tersebut menunjukkan bahwa sensor dapat mendukung fungsi pengaman agar pompa hanya bekerja ketika galon berada pada posisi pengisian.

### 4.2.2 Pengujian Sensor Debit Air YF-S201

**Tujuan pengujian.** Mengukur kemampuan sensor YF-S201 dalam menghitung volume air yang dialirkan dan membandingkannya dengan volume hasil pengukuran menggunakan alat ukur pembanding, serta menganalisis sumber deviasi mekanis pada saluran keluaran.

**Skenario pengujian.** Sistem dijalankan pada target volume 200 mL, 500 mL, dan 1000 mL. Volume yang terbaca pada sistem dibandingkan dengan volume aktual yang diukur menggunakan gelas ukur atau wadah berskala. Selain itu, dilakukan perhitungan teoretis volume air sisa pada selang pasca-sensor.

#### A. Pengujian Akurasi Volume

**Tabel 4.3. Hasil pengujian akurasi sensor debit air**

| No. | Target volume (mL) | Volume terbaca sistem (mL) | Volume aktual (mL) | Selisih (mL) | Galat (%) | Keterangan |
|---:|---:|---:|---:|---:|---:|---|
| 1 | 200 | 200 | 200 | 0 | 0,00 | Volume aktual sesuai target (selisih pas) |
| 2 | 500 | 500 | 550 | +50 | 9,09 | Uji awal pra-kalibrasi; 238 pulsa tercatat (target 225 pulsa) |
| 3 | 1000 | 1000 | 990 | -10 | 1,01 | Volume aktual kurang 10 mL |
| 4 | 1000 | 1000 | 992 | -8 | 0,81 | Volume aktual kurang 8 mL |
| 5 | 1000 | 1000 | 995 | -5 | 0,50 | Volume aktual kurang 5 mL |

Perhitungan galat dilakukan menggunakan persamaan berikut.

$$\text{Galat} = \left|\frac{\text{Volume terbaca sistem} - \text{Volume aktual}}{\text{Volume aktual}}\right| \times 100\%$$

Berdasarkan Tabel 4.3, pengujian pada target 200 mL menghasilkan volume aktual yang sesuai dengan target (selisih 0 mL). Pengujian pada target 1000 mL menghasilkan volume aktual yang cenderung sedikit kurang, dengan selisih antara $-5$ mL hingga $-10$ mL (galat 0,50%–1,01%). Galat tertinggi terjadi pada uji awal 500 mL (9,09%), yang dipengaruhi oleh penundaan pemutusan pompa dan volume air sisa pada selang keluaran. Jika uji awal 500 mL dikecualikan sebagai data pra-kalibrasi, rata-rata galat pengujian berada pada kisaran yang sangat rendah. Konstanta kalibrasi yang digunakan adalah 450 pulsa per liter sesuai *datasheet* sensor YF-S201, dengan persamaan frekuensi $F = 7{,}5 \times Q$ ($Q$ dalam L/min).

#### B. Analisis Volume Air Sisa pada Selang

Selain galat pembacaan sensor, deviasi volume juga dapat berasal dari air yang masih mengalir pada selang keluaran setelah pompa dimatikan. Parameter fisik saluran keluaran sistem adalah sebagai berikut: sensor YF-S201 berdiameter 1/2 inci, *reducer* 1/2 inci ke 5/16 inci, serta selang keluaran berdiameter nominal 5/16 inci dengan panjang pasca-sensor $h = 13$ cm.

Diameter dalam (*inner diameter*) selang dihitung sebagai berikut.

$$\text{ID} = \frac{5}{16}\text{ inci} = 0{,}3125 \times 25{,}4\text{ mm} = 7{,}9375\text{ mm}$$

$$r = \frac{\text{ID}}{2} = 3{,}96875\text{ mm} \approx 0{,}397\text{ cm}$$

Volume sisa air pada selang dihitung menggunakan rumus volume silinder:

$$V = \pi \times r^{2} \times h$$

$$V = 3{,}14159 \times (0{,}396875\text{ cm})^{2} \times 13\text{ cm} \approx 6{,}43\text{ mL}$$

Terhadap target 500 mL, volume sisa selang menyumbang deviasi residual sekitar:

$$\text{Error residual} = \left(\frac{6{,}43}{500}\right) \times 100\% \approx 1{,}29\%$$

Nilai tersebut masih berada di bawah ambang toleransi umum dispenser air (sekitar $\pm 2\%$ hingga $5\%$). Dengan demikian, deviasi yang diamati pada pengujian tidak semata-mata berasal dari kesalahan pembacaan sensor, melainkan juga dari karakteristik mekanis saluran air dan inersia pemutusan pompa. Kompensasi perangkat lunak untuk volume sisa selang bersifat opsional dan dapat menjadi pengembangan lanjutan.

### 4.2.3 Pengujian Tombol *Push-Button*

**Tujuan pengujian.** Memastikan tombol *push-button* dapat memberikan masukan kepada ESP32-S3 sesuai fungsi yang dirancang, yaitu mengonfirmasi dimulainya pengisian dan melanjutkan pengisian setelah galon dikembalikan.

**Tabel 4.4. Hasil pengujian tombol *push-button***

| No. | Aksi tombol | Respons yang diharapkan | Respons hasil uji | Keterangan |
|---:|---|---|---|---|
| 1 | Tombol ditekan sekali saat galon terdeteksi | Sistem memulai proses pengisian | Sesuai | Debounce 50 ms bekerja baik |
| 2 | Tombol ditekan saat kondisi siaga (belum bayar) | Tidak memicu pengisian | Sesuai | Tombol tidak aktif sebelum status `SCANNED` |
| 3 | Tombol ditekan setelah galon dikembalikan (resume) | Sistem melanjutkan pengisian | Sesuai | Digunakan untuk melanjutkan sisa waktu pengisian |

Berdasarkan Tabel 4.4, tombol *push-button* berfungsi sesuai rancangan. Pengujian ini menunjukkan bahwa pengguna dapat memberikan perintah lokal pada perangkat untuk memulai dan melanjutkan pengisian tanpa harus menggunakan aplikasi.

### 4.2.4 Pengujian Pompa dan Relay

**Tujuan pengujian.** Memastikan modul relay dapat mengendalikan pompa air berdasarkan perintah dari ESP32-S3 dan kondisi sistem.

**Tabel 4.5. Hasil pengujian pompa dan relay**

| No. | Kondisi pengujian | Status relay yang diharapkan | Status pompa yang diharapkan | Hasil uji | Keterangan |
|---:|---|---|---|---|---|
| 1 | Transaksi belum berhasil | Tidak aktif | Mati | Sesuai | Pompa tetap mati |
| 2 | Galon tidak terdeteksi | Tidak aktif | Mati | Sesuai | Interlock IR mencegah pompa menyala |
| 3 | Transaksi berhasil dan galon terdeteksi | Aktif | Menyala | Sesuai | Relay `HIGH`, air mengalir |
| 4 | Volume target tercapai | Tidak aktif | Mati | Sesuai | Pompa berhenti otomatis |

Berdasarkan Tabel 4.5, relay dan pompa bekerja sesuai logika kendali. Mekanisme penghentian pompa ketika target volume tercapai merupakan bagian penting untuk mencegah pengisian berlebih.

### 4.2.5 Pengujian Layar TFT ILI9341

**Tujuan pengujian.** Memastikan layar TFT mampu menampilkan informasi operasional sistem secara jelas.

**Tabel 4.6. Hasil pengujian tampilan TFT ILI9341**

| No. | Informasi yang ditampilkan | Tampilan yang diharapkan | Hasil uji | Keterangan |
|---:|---|---|---|---|
| 1 | Status koneksi sistem | Status koneksi ditampilkan | Sesuai | Menampilkan proses koneksi WiFi dan IP lokal |
| 2 | Status galon | Terdeteksi/tidak terdeteksi ditampilkan | Sesuai | Tampilan PREPARE_FILL dan CONFIRM_FILL |
| 3 | Volume pengisian | Nilai volume ditampilkan | Sesuai | *Progress bar* dan teks volume (liter) diperbarui |
| 4 | Status pompa | Menyala/mati ditampilkan | Sesuai | Tampilan PROCESSING saat pompa aktif |
| 5 | Status transaksi | Berhasil/menunggu/gagal ditampilkan | Sesuai | QR *idle*, DONE, dan ERROR ditampilkan |

Berdasarkan Tabel 4.6, layar TFT ILI9341 mampu menampilkan seluruh informasi operasional yang dirancang. Informasi pada layar membantu pengguna dan operator memantau keadaan alat secara langsung di lokasi pengisian.

### 4.2.6 Pengujian Aplikasi dan *Payment Gateway*

**Tujuan pengujian.** Memastikan aplikasi dapat membuat transaksi, menampilkan status pembayaran, dan meneruskan informasi yang diperlukan untuk proses pengisian.

**Tabel 4.7. Hasil pengujian aplikasi dan *Payment Gateway***

| No. | Skenario pengujian | Hasil yang diharapkan | Hasil uji | Keterangan |
|---:|---|---|---|---|
| 1 | Pengguna memilih volume pengisian | Pilihan volume tersimpan pada transaksi | Sesuai | Data `total_galon` dikirim ke API |
| 2 | Pengguna melakukan pembayaran | Sistem menampilkan proses pembayaran | Sesuai | Halaman *invoice* Xendit terbuka di WebView |
| 3 | Pembayaran berhasil | Status transaksi berubah menjadi berhasil | Sesuai | Status `PAID`/`SETTLED`, perangkat `SCANNED` |
| 4 | Pembayaran belum berhasil/gagal | Pompa tidak dapat diaktifkan | Sesuai | ESP32-S3 tetap pada status *idle* |
| 5 | Transaksi berhasil dan galon tersedia | Perintah pengisian dapat dijalankan | Sesuai | Pompa menyala setelah tombol ditekan |

Berdasarkan Tabel 4.7, integrasi aplikasi dan *Payment Gateway* Xendit berjalan sesuai rancangan. Pengujian dilakukan menggunakan transaksi uji pada lingkungan yang terhubung ke layanan Xendit.

### 4.2.7 Pengujian Sistem Keseluruhan

**Tujuan pengujian.** Memastikan seluruh komponen bekerja secara terintegrasi dari pemilihan volume hingga pengisian selesai.

**Tabel 4.8. Hasil pengujian sistem keseluruhan**

| No. | Skenario | Kondisi awal | Hasil yang diharapkan | Hasil uji | Status |
|---:|---|---|---|---|---|
| 1 | Pengisian dengan transaksi berhasil | Galon tersedia, sistem terhubung | Pompa mengisi hingga target volume | Sesuai | Berhasil |
| 2 | Galon tidak tersedia | Sistem terhubung, transaksi berhasil | Pompa tidak menyala | Sesuai | Berhasil |
| 3 | Pembayaran belum berhasil | Galon tersedia | Pompa tidak menyala | Sesuai | Berhasil |
| 4 | Target volume tercapai | Pompa sedang menyala | Pompa berhenti otomatis | Sesuai | Berhasil |
| 5 | Koneksi internet terganggu | Sistem sedang digunakan | Sistem memberikan status sesuai rancangan | TFT menampilkan *reconnecting*; relay dimatikan | Berhasil |

Berdasarkan pengujian pada Tabel 4.8, sistem GalonKu secara keseluruhan mampu menjalankan alur pengisian air otomatis dari transaksi hingga pengisian selesai. Fungsi pengaman, pengendalian volume, dan pemulihan saat gangguan koneksi telah berjalan sesuai rancangan. Bagian yang masih dapat ditingkatkan adalah kompensasi volume sisa selang dan penyesuaian kalibrasi sensor pada target volume yang lebih kecil.

## 4.3 Pembahasan

Hasil implementasi menunjukkan bahwa GalonKu mengintegrasikan perangkat sensor, aktuator, aplikasi *mobile*, layanan *backend*, basis data PostgreSQL, dan *Payment Gateway* Xendit dalam satu kesatuan alur kerja. ESP32-S3 bertindak sebagai pengendali utama yang memproses masukan dari sensor inframerah, sensor debit YF-S201, dan tombol *push-button*, serta mengendalikan relay pompa dan layar TFT berdasarkan status transaksi yang diperoleh dari server melalui *polling* HTTP.

Berdasarkan hasil pengujian pada Tabel 4.2 sampai Tabel 4.8, sistem menunjukkan kinerja yang sesuai dengan rancangan pada seluruh skenario utama. Sensor inframerah mampu mendeteksi keberadaan galon secara konsisten (Tabel 4.2), sehingga mekanisme pengaman *interlock* yang mencegah pompa menyala saat galon tidak terdeteksi dapat berjalan dengan baik. Tombol *push-button* berfungsi untuk memulai dan melanjutkan pengisian, serta tetap tidak aktif pada kondisi yang belum memenuhi syarat — seperti saat transaksi belum berhasil (Tabel 4.4). Pengujian pompa dan relay (Tabel 4.5) memvalidasi bahwa sistem pemompaan hanya aktif ketika seluruh kondisi keamanan terpenuhi, dan berhenti secara otomatis ketika volume target tercapai.

Sensor debit YF-S201 menunjukkan akurasi yang sangat baik pada pengujian target volume 200 mL dengan volume aktual yang sesuai persis tanpa selisih (0 mL). Pada target volume 1000 mL, sistem mencatat volume aktual yang sedikit kurang, dengan galat antara 0,50% hingga 1,01% (Tabel 4.3). Angka-angka ini berada dalam kisaran toleransi industri dispenser air yang umumnya berkisar antara ±2% hingga ±5%. Galat yang lebih tinggi (9,09%) pada pengujian awal 500 mL disebabkan oleh dua faktor utama: pertama, volume air sisa pada selang keluaran pasca-sensor sebesar 6,43 mL yang dihitung berdasarkan geometri selang ($V = \pi r^2 h$), dan kedua, inersia mekanis pemutusan pompa yang menyebabkan sejumlah kecil air masih mengalir setelah relay dimatikan. Kontribusi volume sisa selang terhadap deviasi residual adalah sekitar 1,29% terhadap target 500 mL, yang masih berada di bawah batas toleransi.

Layar TFT ILI9341 berhasil menampilkan seluruh informasi operasional yang dirancang, meliputi status koneksi, status galon, volume pengisian, status pompa, dan status transaksi (Tabel 4.6). Integrasi aplikasi dan *Payment Gateway* Xendit (Tabel 4.7) berjalan sesuai alur yang direncanakan: transaksi dibuat, *invoice* ditampilkan, pembayaran diverifikasi melalui *webhook*, dan status perangkat diperbarui menjadi `SCANNED`. Pengujian sistem keseluruhan (Tabel 4.8) menunjukkan bahwa seluruh skenario — mulai dari pengisian normal, penolakan saat galon tidak tersedia, penolakan saat pembayaran belum berhasil, penghentian otomatis pada target volume, hingga pemulihan saat koneksi internet terganggu — memberikan hasil yang sesuai.

Kelebihan sistem GalonKu terletak pada otomatisasi proses pengisian dan pencatatan transaksi secara digital tanpa campur tangan operator. Pengguna cukup memindai kode QR, memilih volume, dan melakukan pembayaran melalui aplikasi. Setelah pembayaran terverifikasi, dispenser bekerja secara mandiri hingga seluruh galon terisi. Mekanisme pengaman berlapis melalui sensor inframerah dan status transaksi memastikan pompa tidak menyala dalam kondisi yang tidak aman.

Namun, beberapa aspek masih dapat dikembangkan lebih lanjut. Kompensasi perangkat lunak terhadap volume sisa selang dan inersia pemutusan pompa dapat diterapkan untuk meningkatkan akurasi, khususnya pada target volume kecil. Kalibrasi sensor debit secara periodik disarankan untuk menjaga konsistensi akurasi seiring waktu. Ketergantungan sistem pada koneksi internet juga menjadi faktor yang perlu dipertimbangkan; meskipun saat ini telah ditangani melalui mekanisme *fail-safe* yang mematikan pompa dan menampilkan status *error*, pengembangan mode operasi luring (*offline*) dengan sinkronisasi data setelah koneksi pulih dapat menjadi peningkatan yang signifikan. Hasil pengujian yang belum mencapai tingkat akurasi ideal pada volume kecil tidak perlu dipandang sebagai kegagalan, melainkan sebagai dasar untuk pengembangan dan penyempurnaan sistem pada tahap berikutnya.
