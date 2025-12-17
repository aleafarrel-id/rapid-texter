# 🚀 Rapid Texter

![Language](https://img.shields.io/badge/language-C++-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Windows-lightgrey.svg)

**Rapid Texter** adalah aplikasi tes kecepatan mengetik berbasis terminal (Command Line Interface) yang ringan dan cepat. Dibangun menggunakan C++, aplikasi ini dirancang untuk membantu pengguna melatih kecepatan (WPM) dan akurasi mengetik dalam berbagai mode bahasa, termasuk Bahasa Indonesia, Inggris, dan sintaks Pemrograman.

## ✨ Fitur Utama

- **Ringan & Cepat:** Berjalan langsung di terminal tanpa GUI yang berat.
- **Multi-Bahasa:** Mendukung latihan dengan kosakata Bahasa Indonesia (`id.txt`) dan Bahasa Inggris (`en.txt`).
- **Mode Programmer:** Latih mengetik simbol dan sintaks koding dengan mode `prog.txt`.
- **Statistik Real-time:** Menampilkan akurasi dan kecepatan mengetik setelah sesi berakhir.
- **Struktur Modular:** Kode terorganisir dengan baik menggunakan konsep OOP (GameEngine, Terminal, TextProvider).

## 📂 Struktur Proyek

```text
rapid-texter/
├── include/          # Header files (.h)
│   ├── GameEngine.h
│   ├── Stats.h
│   ├── Terminal.h
│   └── TextProvider.h
├── src/              # Source code (.cpp)
│   ├── GameEngine.cpp
│   ├── Terminal.cpp
│   ├── TextProvider.cpp
│   └── main.cpp
├── obj/              # Object files hasil kompilasi
├── en.txt            # Dataset kata Bahasa Inggris
├── id.txt            # Dataset kata Bahasa Indonesia
├── prog.txt          # Dataset sintaks Pemrograman
├── Makefile          # Script untuk kompilasi otomatis
└── README.md         # Dokumentasi proyek
```

## 🛠️ Instalasi & Kompilasi

Pastikan Anda telah menginstal `g++` (GNU C++ Compiler) dan `make` di sistem Anda.

1. **Clone repositori ini:**
   ```bash
   git clone [https://github.com/aleafarrel-id/rapid-texter.git](https://github.com/aleafarrel-id/rapid-texter.git)
   cd rapid-texter
   ```

2. **Kompilasi program menggunakan Make:**
   Cukup jalankan perintah berikut di root folder proyek:
   ```bash
   make
   ```
   
   Jika Anda ingin membersihkan file hasil kompilasi lama (clean build):
   ```bash
   make clean
   make
   ```

## 🚀 Cara Penggunaan

Setelah proses kompilasi berhasil, akan muncul file executable (biasanya bernama `main` atau `rapid-texter` tergantung konfigurasi Makefile Anda).

Jalankan program dengan perintah:

```bash
./main
```
*(Catatan: Sesuaikan `./main` dengan nama output binary yang dihasilkan oleh Makefile)*

### Navigasi dalam Game:
1. Pilih mode bahasa yang diinginkan saat diminta.
2. Ketik kata yang muncul di layar secepat mungkin.
3. Tekan `Enter` atau `Spasi` untuk lanjut ke kata berikutnya (tergantung implementasi).
4. Lihat statistik WPM dan Akurasi Anda di akhir sesi.

## 🤝 Kontribusi

Kontribusi sangat diterima! Jika Anda ingin menambahkan fitur baru atau memperbaiki bug:

1. Fork repositori ini.
2. Buat branch fitur baru (`git checkout -b fitur-keren`).
3. Commit perubahan Anda (`git commit -m 'Menambahkan fitur keren'`).
4. Push ke branch (`git push origin fitur-keren`).
5. Buat Pull Request.

## 📝 Lisensi

Proyek ini dilisensikan di bawah [MIT License](LICENSE).

---
Dibuat dengan ❤️ oleh [Alea Farrel]
