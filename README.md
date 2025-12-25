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

Atau bisa download aplikasi jadi untuk keduanya dari **Release**

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
  Developed 2025 by Alea Farrel.
</div>
