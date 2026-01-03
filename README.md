<div align="center">

# 🚀 Rapid Texter

![C++](https://img.shields.io/badge/C++-17-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-lightgrey?style=for-the-badge)
![License](https://img.shields.io/badge/License-MIT-green?style=for-the-badge)

**Uji kecepatan jari dan ketepatan pikiran Anda langsung dari Terminal.** Rapid Texter adalah aplikasi *Touch Typing* berbasis CLI yang ringan, cepat, dan estetis.

[Fitur](#-fitur-utama) • [Instalasi](#%EF%B8%8F-cara-compile--jalankan) • [Kontribusi](#-lisensi)

</div>

---

## ⚡ Fitur Utama

* **🌐 Multi-Bahasa:** Tersedia mode Bahasa Indonesia & Bahasa Inggris.
* **💻 Mode Programmer:** Latih pengetikan simbol dan sintaks koding (`#include`, `std::vector`, dll).
* **📊 Statistik Real-time:** Pantau WPM (*Words Per Minute*), Akurasi, dan Waktu secara langsung.
* **🎨 Cross-Platform:** Tampilan cantik yang konsisten di **Windows** (CMD/PowerShell) dan **Linux**.
* **🚀 Ringan:** Tanpa GUI berat, berjalan instan di terminal apa saja.
* **🎵 Easter Egg:** Temukan kejutan fitur rahasia yang tersembunyi di dalam folder `roll/`.

---

## 🎮 Cara Menggunakan Aplikasi

### 1. Jalankan Aplikasi
Buka terminal dan jalankan `RapidTexter.exe` (Windows) atau `./RapidTexter` (Linux).

### 2. Menu Utama
Pada menu utama, pilih opsi yang tersedia:
- **[1] Start Game** - Mulai permainan baru
- **[2] Show History** - Lihat riwayat permainan sebelumnya
- **(Q) Quit** - Keluar dari aplikasi
- **(S) Toggle SFX** - Aktifkan/nonaktifkan efek suara

### 3. Pilih Bahasa
Pilih bahasa untuk teks yang akan diketik:
- **[1] Indonesia (ID)** - Teks dalam Bahasa Indonesia
- **[2] English (EN)** - Teks dalam Bahasa Inggris

### 4. Pilih Durasi Waktu
Pilih durasi waktu untuk sesi mengetik:
- **[1] 15 Detik** - Sesi singkat
- **[2] 30 Detik** - Sesi standar
- **[3] 60 Detik** - Sesi panjang
- **[4] Custom** - Masukkan durasi sendiri (dalam detik)
- **[5] Tanpa Waktu** - Tidak ada batas waktu
- **[Enter]** - Gunakan durasi default yang tersimpan

### 5. Pilih Mode Permainan
Pilih mode permainan yang diinginkan:
- **[1] Manual Mode** - Tentukan target WPM sendiri
- **[2] Campaign Mode** - Ikuti level kesulitan bertahap

### 6. Pilih Tingkat Kesulitan / Target WPM
- **Manual Mode:** Masukkan target WPM yang ingin dicapai
- **Campaign Mode:** Pilih tingkat kesulitan:
  - **Easy** - Target WPM rendah, cocok untuk pemula
  - **Medium** - Target WPM menengah
  - **Hard** - Target WPM tinggi, untuk yang sudah mahir
  - **Programmer** - Latihan mengetik simbol dan sintaks koding

### 7. Mulai Mengetik
Ketik teks yang muncul di layar dengan cepat dan akurat. Statistik WPM, akurasi, dan waktu akan ditampilkan secara real-time.

### 8. Lihat Hasil
Setelah waktu habis atau menyelesaikan teks, statistik akhir akan ditampilkan:
- **WPM (Words Per Minute)** - Kecepatan mengetik
- **Accuracy** - Ketepatan pengetikan
- **Errors** - Jumlah kesalahan

---

### Diagram Alur Aplikasi

![Flow Chart](https://github.com/aleafarrel-id/rapid-texter/blob/main/documentation/flow_simple.png)

---

## 🛠️ Cara Compile & Jalankan

Project ini menggunakan **CMake** untuk mempermudah proses instalasi. Anda tidak perlu repot membuat folder build secara manual. Hasil compile akan langsung muncul di folder ini.

### Prasyarat
Pastikan Anda sudah menginstall:
1.  **C++ Compiler** terbaru (GCC, Clang, MinGW atau MSVC).
2.  **CMake** (Install saat menginstall Visual Studio atau via `sudo apt install cmake` di Linux).

> **Catatan:** Project bisa langsung dibuka di Visual Studio dan akan mendeteksi file CMakeLists.txt.

### Langkah Cepat (Windows & Linux)

Buka terminal di folder project ini, lalu jalankan **dua perintah** berikut:

```bash
# 1. Siapkan konfigurasi (cukup sekali)
cmake CMakeLists.txt

# 2. Compile program
cmake --build .
```

### Cara Memulai Aplikasi

Setelah proses di atas selesai, jalankan file yang muncul di folder ini:

**🪟 Windows:**
```cmd
RapidTexter.exe
```

**🐧 Linux:**
```bash
./RapidTexter
```
---

Atau bisa download aplikasi jadi untuk keduanya dari **[Releases](https://github.com/aleafarrel-id/rapid-texter/releases)**

---

> **Catatan:** Untuk membuat dalam format AppImage dapat menjalankan `create_appimage.sh`

> Pastikan folder `assets/` dan `roll/` berada di lokasi yang sama dengan aplikasi (seharusnya sudah otomatis tersedia).

## 📂 Struktur Project

```text
rapid-texter/
├── assets/             # Database kata (ID, EN, Code)
├── resources/          # Windows resource file dan icon
├── roll/               # File tambahan untuk fitur rahasia
├── include/            # Header files (.h)
├── src/                # Source code (.cpp)
├── CMakeLists.txt      # Konfigurasi Build
├── create_appimage.sh  # Script untuk membuat AppImage
├── make_installer.nsi  # Script untuk membuat installer Windows
└── README.md           # Dokumentasi ini
```

## 📜 Lisensi

Project ini dilisensikan di bawah **MIT License**. Bebas untuk digunakan, dimodifikasi, dan didistribusikan.

---
<div align="center">
  Developed 2025 by Alea Farrel & Team.
</div>
