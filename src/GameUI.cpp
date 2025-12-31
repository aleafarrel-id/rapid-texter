/**
 * @file GameUI.cpp
 * @brief Implementasi UI/UX Helper untuk Rapid Texter
 * @author Alea Farrel
 * @date 2025
 * 
 * File ini mengimplementasikan semua fungsi rendering UI.
 * 
 * Fungsi-fungsi di sini bertanggung jawab untuk:
 * - Menggambar kotak/border ASCII
 * - Mencetak teks dengan alignment
 * - Menampilkan status bar
 * - Menangani input string dari user
 * 
 * OPTIMIZATION: Semua fungsi menggunakan terminal buffer untuk batching
 * output, mengurangi I/O overhead.
 */

#include "GameUI.h"
#include <thread>
#include <chrono>

// ============================================================================
// CONSTRUCTOR
// ============================================================================

/**
 * @brief Constructor - Initialize GameUI dengan Terminal reference
 * @param term Reference ke Terminal object yang akan digunakan untuk rendering
 * 
 * GameUI tidak memiliki Terminal sendiri, melainkan menerima reference
 * dari GameEngine untuk menghindari duplikasi resource dan memastikan
 * semua rendering menggunakan terminal instance yang sama.
 */
GameUI::GameUI(Terminal& term) : terminal(term) {}

// ============================================================================
// BOX DRAWING
// ============================================================================

/**
 * @brief Menggambar kotak/border ASCII di koordinat tertentu
 * @param x Posisi horizontal (kolom) - kiri atas kotak
 * @param y Posisi vertikal (baris) - kiri atas kotak
 * @param w Lebar kotak (dalam karakter)
 * @param h Tinggi kotak (dalam baris)
 * @param color Warna border (default: WHITE)
 * 
 * Algoritma:
 * 1. Gambar baris atas: + followed by (w-2) dashes, then +
 * 2. Gambar sisi kiri dan kanan untuk setiap baris tengah: | ... |
 * 3. Gambar baris bawah: sama dengan baris atas
 * 
 * Contoh output untuk w=10, h=5:
 * +--------+
 * |        |
 * |        |
 * |        |
 * +--------+
 */
void GameUI::drawBox(int x, int y, int w, int h, Color color) {
    // Set warna border sesuai parameter
    terminal.setColor(color);
    
    // ========================================================================
    // BARIS ATAS (Top Border)
    // ========================================================================
    terminal.setCursor(x, y);
    terminal.print("+");                          // Pojok kiri atas
    for(int i=0; i<w-2; ++i) terminal.print("-"); // Garis horizontal
    terminal.print("+");                          // Pojok kanan atas
    
    // ========================================================================
    // SISI KIRI DAN KANAN (Left & Right Borders)
    // ========================================================================
    // Loop dari baris kedua (y+1) sampai baris sebelum terakhir (y+h-2)
    for(int i=1; i<h-1; ++i) {
        terminal.setCursor(x, y+i);         // Posisi di sisi kiri
        terminal.print("|");                // Border kiri
        terminal.setCursor(x+w-1, y+i);     // Posisi di sisi kanan
        terminal.print("|");                // Border kanan
    }
    
    // ========================================================================
    // BARIS BAWAH (Bottom Border)
    // ========================================================================
    terminal.setCursor(x, y+h-1);
    terminal.print("+");                          // Pojok kiri bawah
    for(int i=0; i<w-2; ++i) terminal.print("-"); // Garis horizontal
    terminal.print("+");                          // Pojok kanan bawah
    
    // Reset warna ke default setelah selesai menggambar
    terminal.resetColor();
}

// ============================================================================
// CENTERED TEXT PRINTING
// ============================================================================

/**
 * @brief Mencetak teks di tengah layar secara horizontal
 * @param y Posisi baris (vertikal) tempat teks akan dicetak
 * @param text String yang akan dicetak
 * @param color Warna teks (default: DEFAULT untuk warna terminal normal)
 * 
 * Algoritma Centering:
 * 1. Dapatkan lebar terminal (cx = width / 2)
 * 2. Hitung posisi awal: x = cx - (text.length / 2)
 * 3. Print teks mulai dari posisi x
 * 
 * Contoh:
 * Terminal width = 80, text = "HELLO" (5 karakter)
 * cx = 40, x = 40 - 2.5 = 37 (integer truncation)
 * Teks akan mulai dari kolom 37, sehingga visual terlihat centered
 */
void GameUI::printCentered(int y, std::string text, Color color) {
    // Hitung posisi horizontal tengah terminal
    int cx = terminal.getWidth() / 2;
    
    // Hitung offset agar teks berada di tengah
    // Misal: width=80, text.length=10 -> x = 40 - 5 = 35
    int x = cx - (text.length() / 2);
    
    // Set posisi cursor ke koordinat yang sudah dihitung
    terminal.setCursor(x, y);
    
    // Terapkan warna jika bukan default
    if (color != Color::DEFAULT) terminal.setColor(color);
    
    // Print teks
    terminal.print(text);
    
    // Reset warna untuk mencegah bleeding ke teks berikutnya
    terminal.resetColor();
}

// ============================================================================
// STATUS BAR
// ============================================================================

/**
 * @brief Menggambar status bar di bagian bawah layar
 * @param language Kode bahasa yang dipilih ("id", "en", "prog")
 * @param duration Durasi yang dipilih dalam detik (-1 untuk unlimited)
 * @param mode Mode permainan ("manual" atau "campaign")
 * 
 * Status bar menampilkan informasi kontekstual tentang sesi game saat ini:
 * - Bahasa: ID / EN / PROG
 * - Time: Durasi dalam detik atau "Inf" untuk unlimited
 * - Mode: Manual / Campaign / Programmer
 * 
 * Tampilan: [ Lang: ID | Time: 30s | Mode: Campaign ]
 * 
 * Layout:
 * - Posisi: 2 baris dari bawah terminal (h - 2)
 * - Background: Biru solid
 * - Text: Putih, centered di atas background
 * - Responsive: Lebar bar menyesuaikan dengan lebar terminal
 */
void GameUI::drawStatusBar(const std::string& language, int duration, const std::string& mode, bool sfxEnabled) {
    int w = terminal.getWidth();
    int h = terminal.getHeight();
    int y = h - 2;  // Posisi: 2 baris dari bawah

    // ========================================================================
    // FORMATTING - Konversi data ke format display
    // ========================================================================
    
    // Format bahasa menjadi display string
    std::string lang = language.empty() ? "N/A" : 
                      (language == "id" ? "ID" : 
                      (language == "en" ? "EN" : "PROG"));
    
    // Format durasi menjadi display string
    std::string time;
    if (duration == -1) time = "Inf";        // Unlimited
    else if (duration == 0) time = "30s";    // Default fallback
    else time = std::to_string(duration) + "s"; // Normal duration
    
    // Format mode menjadi display string
    std::string modeText = mode.empty() ? "N/A" : 
                          (mode == "manual" ? "Manual" : "Campaign");
    
    // Override mode jika sedang dalam Programmer mode
    if (language == "prog") modeText = "Programmer";
    
    // Format SFX status
    std::string sfxStatus = sfxEnabled ? "On" : "Off";
    
    // Gabungkan semua informasi menjadi satu string (dengan SFX status)
    std::string status = " Lang: " + lang + " | Time: " + time + " | Mode: " + modeText + " | SFX: " + sfxStatus + " (S) ";

    // ========================================================================
    // LAYOUT CALCULATION - Hitung dimensi dan posisi bar
    // ========================================================================
    
    // Tentukan lebar bar dengan padding
    int barWidth = status.length() + 75;  // Extra padding untuk estetika
    if (barWidth < 40) barWidth = 40;     // Minimum width
    if (barWidth > w) barWidth = w;       // Maksimum width (tidak boleh lebih dari terminal)

    // Posisikan bar di tengah secara horizontal
    int startX = (w - barWidth) / 2;

    // ========================================================================
    // RENDERING - Gambar background dan teks
    // ========================================================================
    
    // Gambar background biru solid
    terminal.setBackgroundColor(Color::BLUE);
    std::string bg(barWidth, ' ');  // String berisi spasi sebanyak barWidth
    terminal.setCursor(startX + 1, y);
    terminal.print(bg);

    // Tulis teks status di atas background (centered)
    printCentered(y, status, Color::WHITE);
    
    // Reset semua warna untuk mencegah bleeding
    terminal.resetColor();
}

// ============================================================================
// STRING INPUT HANDLER
// ============================================================================

/**
 * @brief Mendapatkan input string dari user (untuk custom input)
 * @param digitsOnly True jika hanya menerima input angka (default: false)
 * @return String hasil input user, atau string kosong jika dibatalkan dengan ESC
 * 
 * Function ini menampilkan cursor dan membiarkan user mengetik input.
 * Mendukung:
 * - ENTER: Konfirmasi input
 * - ESC: Batalkan input (return "")
 * - BACKSPACE: Hapus karakter terakhir
 * - Character input: Masukkan karakter (dengan filter optional)
 * 
 * Input Position:
 * - Horizontal: Centered (width/2 - 10)
 * - Vertical: height/2 + 1
 * 
 * Digunakan untuk:
 * - Custom duration input (digits only)
 * - Custom WPM target input (digits only)
 * 
 * OPTIMIZATION: Flush hanya dipanggil saat visual feedback diperlukan
 * (saat karakter ditambahkan atau dihapus)
 */
std::string GameUI::getStringInput(bool digitsOnly) {
    std::string inputBuf = "";  // Buffer untuk menyimpan input
    
    // ========================================================================
    // LAYOUT - Tentukan posisi input field
    // ========================================================================
    int startX = terminal.getWidth() / 2 - 10;   // Centered dengan offset
    int startY = terminal.getHeight() / 2 + 1;   // Di bawah tengah layar
    
    // Posisikan cursor di awal input field dan tampilkan
    terminal.setCursor(startX + inputBuf.length(), startY);
    terminal.showCursor();

    // ========================================================================
    // INPUT LOOP - Proses input user secara real-time
    // ========================================================================
    while(true) {
        // Check apakah ada input yang tersedia (non-blocking)
        if (terminal.hasInput()) {
            char c = terminal.getInput();
            
            // ================================================================
            // ESC - Batalkan input
            // ================================================================
            if (c == 27) { 
                terminal.hideCursor(); 
                return "";  // Return empty string sebagai tanda cancel
            }
            
            // ================================================================
            // ENTER - Konfirmasi input
            // ================================================================
            if (c == 10 || c == 13) {  // LF atau CR (cross-platform)
                terminal.hideCursor(); 
                return inputBuf;  // Return buffer yang sudah diketik
            }
            
            // ================================================================
            // BACKSPACE - Hapus karakter terakhir
            // ================================================================
            if (c == 127 || c == '\b' || c == 8) {  // DEL, BS (cross-platform)
                if (!inputBuf.empty()) {
                    inputBuf.pop_back();  // Hapus dari buffer
                    
                    // Visual feedback: hapus karakter dari layar
                    terminal.setCursor(startX + inputBuf.length(), startY);
                    terminal.print(" ");  // Overwrite dengan spasi
                    terminal.setCursor(startX + inputBuf.length(), startY);
                    
                    // Flush untuk immediate visual feedback
                    terminal.flush();
                }
            } 
            // ================================================================
            // CHARACTER INPUT - Tambahkan karakter ke buffer
            // ================================================================
            else if (!digitsOnly || (c >= '0' && c <= '9')) {
                // Validasi:
                // 1. Karakter harus printable (ASCII 32-126)
                // 2. Buffer belum mencapai batas maksimal (20 karakter)
                // 3. Jika digitsOnly=true, hanya terima angka 0-9
                
                if (c >= 32 && c <= 126 && inputBuf.length() < 20) {
                     inputBuf += c;  // Tambahkan ke buffer
                     
                     // Visual feedback: tampilkan karakter di layar
                     terminal.print(std::string(1, c));
                     
                     // Flush untuk immediate visual feedback
                     terminal.flush();
                }
            }
        }
    }
}