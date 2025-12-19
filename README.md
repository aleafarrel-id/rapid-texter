Rapid Texter

Rapid Texter adalah aplikasi tes kecepatan mengetik berbasis terminal (CLI) yang sederhana namun *powerful*, ditulis menggunakan C++. Aplikasi ini dirancang untuk membantu Anda mengukur dan meningkatkan kecepatan mengetik (WPM) serta akurasi Anda dalam berbagai mode bahasa.

## 🚀 Fitur Utama

- **Cross-Platform:** Sekarang mendukung penuh **Windows** dan **Linux**.
- **Mode Beragam:**
  - Bahasa Indonesia
  - Bahasa Inggris
  - Mode Programmer (Sintaks kode)
- **Statistik Real-time:** Menampilkan WPM (*Words Per Minute*), Waktu dan Akurasi.
- **Ringan:** Berjalan langsung di terminal tanpa dependensi GUI yang berat.

## 🛠️ Prasyarat

Sebelum melakukan kompilasi, pastikan sistem Anda memiliki:

1.  **C++ Compiler** versi cukup baru yang mendukung standar modern (misal: GCC, Clang, MinGW atau MSVC).
2.  **CMake** (Versi 3.10 atau lebih baru).

## 🏗️ Cara Build (Kompilasi)

Proyek ini sekarang menggunakan **CMake** untuk memastikan kompatibilitas di berbagai sistem operasi.

### Linux / macOS

1.  Buka terminal dan navigasikan ke direktori proyek.
2.  Buat folder build dan masuk ke dalamnya:
    ```bash
    mkdir build
    cd build
    ```
3.  Jalankan CMake dan compile:
    ```bash
    cmake ..
    make
    ```
4.  Jalankan aplikasi:
    ```bash
    ./rapid-texter
    ```

### Windows

1.  Buka Command Prompt (CMD) atau PowerShell di direktori proyek.
2.  Buat direktori build:
    ```cmd
    mkdir build
    cd build
    ```
3.  Generate file project menggunakan CMake:
    ```cmd
    cmake ..
    ```
4.  Build aplikasi (Mode Release disarankan):
    ```cmd
    cmake --build . --config Release
    ```
5.  Aplikasi yang sudah jadi biasanya berada di folder `Release` atau `Debug` di dalam folder `build`.
    ```cmd
    .\\Release\\rapid-texter.exe
    ```

> **Catatan:** Jika Anda menggunakan Visual Studio atau VS Code, Anda juga bisa langsung membuka folder proyek ini dan membiarkan IDE mendeteksi `CMakeLists.txt` secara otomatis.

## 📂 Struktur File Data

```
rapid-texter/
├── include/        # Header files (.h)
├── src/            # Source files (.cpp)
├── assets/         # Text files (en.txt, id.txt, prog.txt)
└── README.md       # Documentation
```

Aplikasi ini membutuhkan file teks sumber (`id.txt`, `en.txt`, `prog.txt`) untuk bekerja.
- Pastikan Anda menjalankan program dari direktori root proyek, **ATAU**
- Salin file `.txt` tersebut ke folder yang sama dengan file executable (`.exe` atau binary) jika program tidak dapat menemukan file teks.

## 📜 Lisensi

Proyek ini dilisensikan di bawah [MIT License](LICENSE). Silakan lihat file LICENSE untuk informasi lebih lanjut.