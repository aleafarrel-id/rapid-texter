<div align="center">

# 🚀 Rapid Texter

![C++](https://img.shields.io/badge/C++-17-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-lightgrey?style=for-the-badge)
![License](https://img.shields.io/badge/License-MIT-green?style=for-the-badge)

**Uji kecepatan jari dan ketepatan pikiran Anda langsung dari Terminal.** Rapid Texter adalah aplikasi *Touch Typing* berbasis CLI yang ringan, cepat, dan estetis.

[Fitur](#-fitur-utama) • [Instalasi](#-cara-compile--jalankan) • [Kontribusi](#-kontribusi)

</div>

---

## ⚡ Fitur Utama

* **🌐 Multi-Bahasa:** Tersedia mode Bahasa Indonesia & Bahasa Inggris.
* **💻 Mode Programmer:** Latih pengetikan simbol dan sintaks koding (`#include`, `std::vector`, dll).
* **📊 Statistik Real-time:** Pantau WPM (*Words Per Minute*), Akurasi, dan Waktu secara langsung.
* **🎨 Cross-Platform:** Tampilan cantik yang konsisten di **Windows** (CMD/PowerShell) dan **Linux**.
* **🚀 Ringan:** Tanpa GUI berat, berjalan instan di terminal apa saja.

## 🛠️ Cara Compile & Jalankan

Kami telah menyederhanakan proses instalasi. Anda tidak perlu repot membuat folder build secara manual. Hasil compile akan langsung muncul di folder ini.

### Prasyarat
Pastikan Anda sudah menginstall:
1.  **C++ Compiler** (GCC, Clang, atau MSVC).
2.  **CMake** (Install saat menginstall Visual Studio atau via `sudo apt install cmake` di Linux).

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
rapid_texter.exe
```

**🐧 Linux / macOS:**
```bash
./rapid_texter
```

> **Catatan:** Pastikan folder `assets/` berada di lokasi yang sama dengan aplikasi (seharusnya sudah otomatis tersedia).

## 📂 Struktur Project

```text
rapid-texter/
├── assets/         # Database kata (ID, EN, Code)
├── include/        # Header files (.h)
├── src/            # Source code (.cpp)
├── CMakeLists.txt  # Konfigurasi Build
└── rapid_texter    # (Akan muncul setelah compile)
```

## 📜 Lisensi

Project ini dilisensikan di bawah **MIT License**. Bebas untuk digunakan, dimodifikasi, dan didistribusikan.

---
<div align="center">
  Developed 2025 by Alea Farrel.
</div>