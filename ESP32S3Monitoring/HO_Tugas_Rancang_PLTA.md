## PRAKTIKUM ENERGI BARU DAN TERBARUKAN

TUGAS RANCANG: Pembangkit Listrik Tenaga Air (PLTA) dengan Sistem

Monitoring Daya Berbasis ESP32 (LCD I2C & Web)

Pengampu: Ir. Maria Enggar Santika S.T., M.T.

## A. Tujuan

- 1. Merancang dan membangun prototipe Pembangkit Listrik Tenaga Air (PLTA) skala laboratorium menggunakan penggerak turbin Flow Generator SEA 12V DC (DB-268) untuk mengonversi energi kinetik dan tekanan aliran air menjadi energi listrik arus searah (DC).

- 2. Mengintegrasikan rangkaian pengondisi tegangan menggunakan modul DC-DC Step- Up Converter serta sistem akuisisi dan monitoring data tegangan, arus, dan daya keluaran generator secara real-time menggunakan mikrokontroler ESP32, sensor INA219, tampilan lokal LCD I2C 16x2, dan antarmuka Web Monitoring berbasis Wi- Fi.

- 3. Menguji dan memverifikasi ketercapaian spesifikasi teknis prototipe PLTA yang meliputi aspek mekanik saluran aliran dan debit air, kinerja pembangkitan daya, pengondisian tegangan Step-Up, akurasi pembacaan sensor INA219, serta sinkronisasi tampilan LCD I2C 16x2 dan halaman Web Monitoring.

## B. Dasar Teori

Energi air merupakan salah satu sumber energi baru dan terbarukan yang melimpah, ramah lingkungan, dan memiliki efisiensi konversi yang tinggi. Teknologi pemanfaatan energi kinetik dan tekanan aliran air untuk menghasilkan energi listrik dikenal sebagai Pembangkit Listrik Tenaga Air (PLTA).

Prinsip Kerja Pembangkit Listrik Tenaga Air (PLTA): Sistem PLTA bekerja dengan memanfaatkan debit dan tekanan aliran air yang disalurkan melalui selang/pipa pesat (penstock) menuju flow generator. Tekanan air dari head statis dan energi kinetik aliran memutar poros turbin di dalam generator, sehingga membangkitkan energi listrik arus searah (DC). Pada rancangan ini sumber air (kran PAM) disalurkan langsung ke inlet flow generator tanpa bak penampung atas dan tanpa bak penampung bawah, sehingga head efektif berasal dari ketinggian posisi sumber air terhadap generator.

Flow Generator & Modul Step-Up: Flow Generator SEA 12V DC Micro Hydro Water Turbine (DB-268) bekerja mengubah energi kinetik dan tekanan aliran air di dalam saluran pipa menjadi energi listrik arus searah (DC). Karena debit dan tekanan aliran air pada skala prototipe dapat bervariasi, tegangan keluaran generator cenderung fluktuatif atau berada di bawah tegangan kerja beban. Oleh karena itu, digunakan modul DC-DC Step-Up (Boost Converter) untuk menaikkan dan menstabilkan tegangan masukan yang rendah agar mencapai level tegangan kerja DC (V_out ≥ 5 V DC) yang dapat dimanfaatkan oleh resistor/LED sebagai beban uji.


Sistem Monitoring Daya (ESP32, INA219, LCD I2C 16x2, dan Web Server): Untuk memantau kinerja pembangkitan turbin secara akurat baik secara lokal maupun jarak jauh, digunakan sensor INA219 I2C GY-219 Bi-directional DC Current Sensor yang dipasang langsung pada jalur keluaran Flow Generator untuk mengukur tegangan bus (V), arus DC (mA) melalui resistor shunt internal 0,1 Ω, serta menghitung daya bangkitan (P = V × I). Data pengukuran dikirim melalui jalur komunikasi I2C menuju mikrokontroler ESP32 untuk ditampilkan pada layar LCD I2C 16x2 dan sekaligus disajikan melalui antarmuka Web Monitoring berbasis Wi-Fi secara real-time.

## C. Alat dan Bahan

- 1. 1 Buah Flow Generator SEA 12V DC Micro Hydro Water Turbine Charging DB-268.

- 2. 1 Buah Modul Step-Up (DC-DC Boost Converter).

- 3. 1 Buah LCD I2C 16x2.

- 4. 1 Buah INA219 I2C GY-219 Bi-directional DC Current Sensor.

- 5. 1 Buah Mikrokontroler ESP32 (dengan modul Wi-Fi terintegrasi untuk Web Server) beserta kabel data Micro-USB / Type-C.

- 6. 1 Buah Baterai 3 × AA (4,5 V) beserta battery holder 3 slot, atau power bank USB 5 V sebagai catu daya mandiri untuk ESP32 (tidak digunakan modul charging, tidak dicatu dari flow generator).

- 7. Komponen Mekanik PLTA: Selang PVC fleksibel inlet (penstock) yang langsung dihubungkan menuju saluran masuk (inlet) flow generator, katup pengatur debit (ball valve / keran pengatur aliran) yang dipasang pada jalur selang masuk untuk mengatur debit air, rangka dudukan flow generator (bahan ditentukan oleh praktikan), serta lem tahan air (sealant / silicone sealant). Rangkaian mekanik dibuat tanpa bak penampung air atas (reservoir) dan tanpa bak penampung bawah karena air dialirkan langsung dari sumber air melalui selang menuju flow generator.

- 8. Komponen Sumber Air: Sumber air (kran air PAM) yang menjadi satu-satunya titik suplai air, serta ember/wadah penampung air buangan di bawah flow generator hanya sebagai penampung air buangan sementara.

- 9. Komponen Pendukung, Kalibrasi, & Akses Web: Protoboard, kabel jumper, resistor beban (100 Ω dan 220 Ω) atau lampu LED DC sebagai beban uji, Multimeter Digital, jaringan Wi-Fi (access point/hotspot), serta laptop atau smartphone untuk membuka halaman Web Monitoring.

## D. Spesifikasi Rancangan Sistem (Wiring dan Desain Sistem)

Prototipe Pembangkit Listrik Tenaga Air (PLTA) dan sistem monitoring daya dirancang untuk memenuhi parameter spesifikasi teknis pada setiap sub-sistem berikut:

Tabel TR.1: Spesifikasi Teknis Prototipe PLTA dan Sistem Monitoring


| No | Sub-Sistem / Komponen | Parameter Spesifikasi Target |
| --- | --- | --- |
| 1 | Konstruksi Jalur Selang Langsung & Dudukan Flow Generator | Selang penstock masuk terhubung langsung dan kedap kebocoran ke inlet flow generator (tanpa bak penampung atas dan bak penampung bawah), jalur outlet mengalirkan air secara bebas ke saluran pembuangan, katup pengatur debit berfungsi normal pada variasi bukaan kecil–penuh, dudukan flow generator terpasang kokoh dan tidak bergetar saat beroperasi, serta bagian elektrikal aman dari cipratan air. |
| 2 | Flow Generator SEA 12V DC (DB- 268) | Menghasilkan tegangan DC tanpa beban (𝑉𝑔𝑒𝑛 ≥ 1,5 V− 5 V) yang meningkat proporsional terhadap kenaikan debit aliran air. |
| 3 | Modul DC-DC Step-Up Converter | Menaikkan dan menstabilkan tegangan keluaran flow generator yang fluktuatif menjadi tegangan kerja DC stabil (V_out ≥ 5 V DC). Keluaran modul ini menjadi beban uji yang diukur INA219, bukan sumber daya mikrokontroler. |
| 4 | Sensor Daya INA219 I2C (GY- 219) | Mengukur tegangan keluaran generator (𝑉𝑔𝑒𝑛), arus (mA), dan daya bangkitan (mW) secara real-time dengan galat (error) tegangan ≤ 5% terhadap Multimeter Digital. |
| 5 | Tampilan Lokal LCD I2C 16x2 | Menampilkan nilai tegangan (𝑉), arus (mA), dan daya (mW) secara jelas dan stabil (tanpa glitch) dengan interval pembaruan data ≤ 1 detik. |
| 6 | Web Monitoring (ESP32 Web Server) | Dashboard Web dapat diakses melalui browser via IP lokal ESP32, memperbarui data V, I, dan P otomatis (≤ 2 detik), serta sinkron dengan tampilan pada LCD I2C. ESP32 dan LCD dicatu baterai independen sehingga tampilan tetap stabil saat debit air diubah. |
| 7 | Keandalan Operasi Berbeban | Seluruh sistem pembangkit dan monitoring (LCD & Web) mampu beroperasi kontinu selama ≥ 10 menit |


| No | Sub-Sistem / Komponen | Parameter Spesifikasi Target |
| --- | --- | --- |
|   |   | dalam kondisi berbeban tanpa hang maupun putus koneksi Wi-Fi. |

## E. Ketercapaian Spesifikasi Sistem

Evaluasi keberhasilan rancang bangun prototipe Pembangkit Listrik Tenaga Air (PLTA) didasarkan pada pemenuhan seluruh parameter spesifikasi teknis mekanik, elektrikal, akuisisi sensor, dan sistem monitoring lokal maupun berbasis Web sebagaimana tercantum pada Tabel TR.2 berikut:

*Tabel TR.2: Verifikasi Ketercapaian Spesifikasi Prototipe PLTA & Sistem Monitoring*

| No | Sub-Sistem / Komponen | Parameter Spesifikasi Target | Metode Pengujian | Hasil Pengujian Aktual | Status (Tercapai / Tidak) |
| --- | --- | --- | --- | --- | --- |
| 1 | Konstruksi Jalur Selang Langsung & Dudukan Flow Generator | Selang penstock masuk terhubung langsung dan kedap kebocoran ke inlet flow generator (tanpa bak penampung atas dan bak penampung bawah), jalur outlet mengalirkan air secara bebas ke saluran pembuangan, katup pengatur debit berfungsi baik, dan dudukan flow generator terpasang kokoh serta aman dari cipratan air. | Uji kebocoran (leak test) pada sambungan selang–inlet flow generator dan pengamatan aliran pada variasi bukaan katup debit kecil, sedang, dan penuh. |   |   |
| 2 | Flow Generator SEA 12V DC (DB-268) | Menghasilkan tegangan DC (V_gen ≥ 1,5 V – 5 V tanpa beban) yang meningkat proporsional terhadap kenaikan debit aliran air. | Pengukuran V_gen menggunakan Multimeter Digital pada 3 variasi bukaan katup debit aliran air. |   |   |
| 3 | Modul DC- DC Step-Up Converter | Mampu menaikkan dan menstabilkan tegangan keluaran turbin yang fluktuatif menjadi tegangan kerja DC stabil (V_out ≥ 5 V DC) dan mengalirkannya ke beban uji (100 Ω / LED). | Pengukuran V_out keluaran modul Step- Up pada kondisi tanpa beban dan berbeban (100 Ω / LED), dengan ESP32 dicatu baterai. |   |   |


| No | Sub-Sistem / Komponen | Parameter Spesifikasi Target | Metode Pengujian | Hasil Pengujian Aktual | Status (Tercapai / Tidak) |
| --- | --- | --- | --- | --- | --- |
| 4 | Sensor Daya INA219 I2C (GY-219) | Mengukur tegangan keluaran generator (V_gen), arus (mA), dan daya bangkitan (mW) secara real-time dengan persentase galat (error) tegangan ≤ 5% terhadap Multimeter Digital. | Uji kalibrasi komparatif pembacaan INA219 dengan Multimeter Digital sebanyak 3 kali pengujian. |   |   |
| 5 | Tampilan Lokal LCD I2C 16x2 | Menampilkan nilai V, I (mA), dan P (mW) pada layar LCD 16x2 secara jelas, stabil (tanpa glitch), dengan pembaruan data ≤ 1 detik. | Pengamatan visual langsung pada layar LCD I2C selama pengujian sistem berlangsung. |   |   |
| 6 | Web Monitoring (ESP32 Web Server) | Dashboard Web dapat diakses via browser melalui IP lokal ESP32, memperbarui data V, I, dan P otomatis (≤ 2 detik) serta sinkron dengan LCD I2C. | Pengujian akses melalui browser laptop/smartphone dan verifikasi kesesuaian nilai dengan LCD, termasuk pengamatan kestabilan tampilan saat bukaan katup diubah dan daya generator drop. |   |   |
| 7 | Keandalan Operasi Berbeban | Seluruh sistem pembangkit dan monitoring (LCD & Web) beroperasi kontinu selama ≥ 10 menit berbeban tanpa hang atau putus koneksi Wi-Fi, dengan ESP32 dicatu baterai secara mandiri. | Pengujian operasi berbeban kontinu selama 10 menit dengan pencatatan setiap interval 2 menit. |   |   |

## Catatan Penyusunan Laporan Tugas Rancang:

- 1. Setiap baris parameter pada Tabel TR.2 wajib dilengkapi dengan bukti data kuantitatif hasil pengukuran lapangan (nilai tegangan V_gen, V_out, arus I, daya P, persentase error sensor INA219, serta dokumentasi tampilan LCD I2C 16x2 dan screenshot halaman Web Monitoring).

- 2. Apabila terdapat parameter spesifikasi yang berstatus Tidak Tercapai, berikan analisis penyebab teknis (misalnya rugi-rugi gesekan fluida pada pipa/katup, debit atau head air kurang optimal, tegangan masukan di bawah ambang kerja minimum modul Step-Up, atau latensi jaringan Wi-Fi) serta langkah perbaikan yang dilakukan.


## F. Kesimpulan

Kesimpulan ditulis sebagai jawaban atas Tujuan (bagian A) dan bukti pencapaian pada Tabel TR.2, bukan sebagai pengulangan isi laporan. Sajikan dalam bentuk narasi 2–3 paragraf mengikuti kerangka berikut (tambahkan baris bila diperlukan):

*Tabel TR.3: Kerangka Isi Kesimpulan*

| Aspek | Isi Kesimpulan yang Wajib Ditulis |
| --- | --- |
| Skenario teruji | Nyatakan apakah rancangan berhasil: sumber air → selang → inlet flow generator → outlet, tanpa bak penampung dan tanpa pompa. |
| Kinerja pembangkitan | Sebutkan nilai V_gen, V_out, I, dan P hasil pengukuran pada bukaan katup kecil/sedang/penuh (angka aktual, bukan target). |
| Keandalan sistem | Nyatakan tercapai/tidaknya operasi kontinu ≥ 10 menit berbeban serta kestabilan pembacaan LCD I2C dan Web Monitoring. |
| Kendala & perbaikan | Sebutkan kendala nyata (mis. kebocoran sambungan selang, head terlalu rendah, tegangan di bawah ambang Step-Up) dan solusinya. |
| Saran | Tuliskan 1–2 saran pengembangan lanjutan (mis. penggunaan katup presisi, penambahan manometer, atau variasi head). |

## G. Ketentuan Penyusunan Laporan Tugas Rancang

Laporan akhir tugas rancang disusun mengikuti struktur baku A–F berikut. Setiap bagian wajib diisi; bagian yang belum lengkap diberi keterangan, bukan dikosongkan.

## A. Tujuan

- Tuliskan tujuan yang terukur, misalnya “menghasilkan V_out ≥ 5 V DC pada bukaan katup 50%”.

- Tujuan harus konsisten dengan alat dan bahan (bagian C) serta dinilai pada bagian E.

## B. Dasar Teori

- Jelaskan persamaan daya hidrolik (P = ρ · g · Q · h · η) dan hubungannya dengan debit hasil bukaan katup.

- Jelaskan prinsip kerja flow generator serta fungsi modul Step-Up, sensor INA219, dan ESP32.

- Sertakan sumber rujukan (buku/datasheet) dan gunakan format sitasi yang konsisten.

## C. Alat dan Bahan

- Tuliskan seluruh alat dan bahan beserta jumlah dan spesifikasi teknisnya.

- Komponen mekanik wajib sesuai model tanpa bak penampung: selang penstock langsung ke inlet, katup pengatur debit, dudukan flow generator, dan sealant.

- Catu daya ESP32 gunakan baterai pada pin VIN.

## D. Spesifikasi Rancangan Sistem (Wiring dan Desain Sistem)

- Wajib memuat diagram wiring (rangkaian elektrikal) dan desain sistem (skema mekanik).

- Wiring menggambarkan dua jalur daya terpisah: (1) jalur ukur flow generator → INA219 → Step-Up → beban uji; dan (2) jalur catu baterai → ESP32 → LCD (I2C).

- Desain sistem menggambarkan jalur air: sumber air → selang → katup → inlet flow generator → outlet → pembuangan, tanpa bak penampung atas maupun bawah.

- Cantumkan Tabel TR.1 (spesifikasi teknis) pada bagian ini.


- E. Ketercapaian Spesifikasi Sistem

- Salin Tabel TR.2 dan isi kolom Hasil Pengujian Aktual serta Status (Tercapai / Tidak).

- Setiap data didukung bukti: angka pengukuran, foto alat ukur, foto LCD, dan screenshot Web Monitoring.

- Berikan analisis penyebab untuk setiap parameter yang Tidak Tercapai.

## F. Kesimpulan

- Kesimpulan menjawab seluruh tujuan pada bagian A secara langsung.

- Gunakan angka hasil pengukuran aktual, bukan angka target.

- Akhiri dengan kendala dan saran pengembangan (lihat Tabel TR.3).

## Ketentuan Umum

- Format: kertas A4, margin 2,5 cm, huruf Times New Roman 12 pt, spasi 1,15–1,5.

- Penomoran: bagian A–F, tabel diberi keterangan “Tabel TR.n” di atas tabel, gambar diberi keterangan di bawah gambar.

- Gambar wajib diberi nomor dan keterangan serta dirujuk di dalam teks.

- Bagian E wajib diisi & tidak boleh kosong.

- Laporan dikumpulkan dalam format DOCX dan Hardfile.
