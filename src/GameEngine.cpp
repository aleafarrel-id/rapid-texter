/**
 * @file GameEngine.cpp
 * @brief Implementasi logic utama aplikasi Rapid Texter
 * @author Alea Farrel
 * @date 2025
 * 
 * File ini mengimplementasikan State Machine untuk mengelola alur aplikasi,
 * dari menu awal hingga gameplay dan hasil akhir.
 * 
 * OPTIMIZATION: Ditambahkan terminal.flush() calls untuk mengurangi I/O delay
 */

#include "GameEngine.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <iomanip>
#include <cmath>
#include <cstdlib>

// ============================================================================
// CONSTRUCTOR & INITIALIZATION
// ============================================================================

/**
 * @brief Constructor - Menginisialisasi state awal aplikasi
 * 
 * Menyiapkan terminal, mengatur status unlock awal untuk mode Campaign,
 * dan memuat database kata dari file eksternal.
 */
GameEngine::GameEngine() : currentState(GameState::MENU_LANGUAGE), previousState(GameState::MENU_LANGUAGE) {
    terminal.initialize();
    
    // Setup unlock status untuk Campaign Mode
    unlockedDifficulties[Difficulty::EASY] = true;         // ✓ Terbuka dari awal
    unlockedDifficulties[Difficulty::MEDIUM] = false;      // ✗ Butuh: 40 WPM, 80% Akurasi (Easy)
    unlockedDifficulties[Difficulty::HARD] = false;        // ✗ Butuh: 60 WPM, 90% Akurasi (Medium)
    unlockedDifficulties[Difficulty::PROGRAMMER] = true;   // ✓ Bonus (selalu terbuka)
    hardCompleted = false;                                 // Hard belum pernah diselesaikan
    rickRollAlreadyShown = false;                          // Rick Roll belum ditampilkan
    
    // Load word databases
    textProvider.loadWords("id", "assets/id.txt");
    textProvider.loadWords("en", "assets/en.txt");
    textProvider.loadWords("prog", "assets/prog.txt");
    
    selectedDuration = 30; // Default: 30 detik
    originalLanguage = "";  // Akan diset saat user memilih bahasa
}

// ============================================================================
// MAIN LOOP (STATE MACHINE)
// ============================================================================

/**
 * @brief Loop utama aplikasi menggunakan State Machine pattern
 * 
 * Mengelola perpindahan antar state (menu bahasa, durasi, mode, difficulty,
 * gameplay, hasil, dan credits).
 */
void GameEngine::run() {
    while (currentState != GameState::EXIT) {
        switch (currentState) {
            case GameState::MENU_LANGUAGE:
                handleMenuLanguage();
                break;
            case GameState::MENU_DURATION:
                handleMenuDuration();
                break;
            case GameState::MENU_MODE:
                handleMenuMode();
                break;
            case GameState::MENU_DIFFICULTY:
                handleMenuDifficulty();
                break;
            case GameState::PLAYING:
                gameLoop();
                break;
            case GameState::RESULTS:
                showResults();
                break;
            case GameState::CREDITS:
                showCredits();
                break;
            case GameState::EXIT:
                break;
        }
    }
    terminal.cleanup();
}

// ============================================================================
// RICK ROLL EASTER EGG
// ============================================================================

/**
 * @brief Menjalankan Rick Roll easter egg sesuai platform
 * 
 * Tetap dalam alternate screen buffer untuk mencegah "flash" ke CLI
 * Problem: terminal.cleanup() exit alternate screen, user lihat PowerShell sejenak
 * Solution: Hanya disable raw mode, TETAP dalam alternate screen aplikasi
 */
void GameEngine::playRickRoll() {
    // JANGAN keluar dari alternate screen buffer!
    // Hanya disable raw mode agar script bisa terima input normal
    terminal.disableRawMode();
    terminal.showCursor();
    
    // Clear screen tapi TETAP dalam alternate buffer
    std::cout << "\033[2J\033[H";
    std::cout.flush();
    
#ifdef _WIN32
    // Windows: Jalankan PowerShell script dari folder 'roll/'
    system("powershell -ExecutionPolicy Bypass -File roll/roll.ps1");
#else
    // Linux/Mac: Jalankan bash script dari folder 'roll/'
    system("bash roll/roll.sh");
#endif
    
    // COMPREHENSIVE terminal state reset setelah Rick Roll selesai
    // Phase 1: Reset SEMUA SGR attributes
    std::cout << "\033[0m";        // Reset all attributes (colors, bold, etc)
    std::cout << "\033[r";         // Reset scroll region to full screen
    std::cout << "\033(B";         // Reset character set to default ASCII
    
    // Phase 2: Clear and reposition
    std::cout << "\033[2J";        // Clear entire screen
    std::cout << "\033[H";         // Move cursor to home position (1,1)
    
    // Phase 3: Cursor control
    std::cout << "\033[?25l";      // Hide cursor
    std::cout << "\033[0 q";       // Reset cursor style to default
    
    // Phase 4: Force flush untuk memastikan semua command diterapkan
    std::cout.flush();
    
    // Phase 5: Delay untuk stabilisasi terminal state
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    
    // Phase 6: Re-enable raw mode (TANPA re-initialize yang keluar alternate screen)
    terminal.enableRawMode();
    terminal.hideCursor();
}

// ============================================================================
// LANGUAGE RESTORATION HELPER
// ============================================================================

/**
 * @brief Restore bahasa ke pilihan asli setelah Programmer Mode
 * 
 * Function ini dipanggil setiap kali keluar dari Programmer Mode untuk
 * memastikan bahasa kembali ke ID/EN yang dipilih user di awal.
 * 
 * Dipanggil di:
 * - processInput() saat user tekan ESC
 * - showResults() setelah game selesai
 * - handleMenuDifficulty() saat user kembali ke menu
 */
void GameEngine::restoreLanguageFromProgrammerMode() {
    if (currentDifficulty == Difficulty::PROGRAMMER && !originalLanguage.empty()) {
        currentLanguage = originalLanguage;
    }
}

// ============================================================================
// UI HELPER FUNCTIONS
// ============================================================================

/**
 * @brief Menggambar kotak/border ASCII di koordinat tertentu
 * @param x Posisi horizontal (kolom)
 * @param y Posisi vertikal (baris)
 * @param w Lebar kotak
 * @param h Tinggi kotak
 * @param color Warna border
 */
void GameEngine::drawBox(int x, int y, int w, int h, Color color) {
    terminal.setColor(color);
    
    // Baris atas
    terminal.setCursor(x, y);
    terminal.print("+");
    for(int i=0; i<w-2; ++i) terminal.print("-");
    terminal.print("+");
    
    // Sisi kiri dan kanan
    for(int i=1; i<h-1; ++i) {
        terminal.setCursor(x, y+i);
        terminal.print("|");
        terminal.setCursor(x+w-1, y+i);
        terminal.print("|");
    }
    
    // Baris bawah
    terminal.setCursor(x, y+h-1);
    terminal.print("+");
    for(int i=0; i<w-2; ++i) terminal.print("-");
    terminal.print("+");
    
    terminal.resetColor();
}

/**
 * @brief Mencetak teks di tengah layar secara horizontal
 * @param y Posisi baris (vertikal)
 * @param text Teks yang akan dicetak
 * @param color Warna teks
 */
void GameEngine::printCentered(int y, std::string text, Color color) {
    int cx = terminal.getWidth() / 2;
    int x = cx - (text.length() / 2);
    terminal.setCursor(x, y);
    if (color != Color::DEFAULT) terminal.setColor(color);
    terminal.print(text);
    terminal.resetColor();
}

/**
 * @brief Menggambar status bar di bagian bawah layar
 * 
 * Menampilkan informasi: Bahasa, Durasi, dan Mode yang dipilih
 */
void GameEngine::drawStatusBar() {
    int w = terminal.getWidth();
    int h = terminal.getHeight();
    int y = h - 2;

    // Siapkan teks status
    std::string lang = currentLanguage.empty() ? "N/A" : 
                      (currentLanguage == "id" ? "ID" : 
                      (currentLanguage == "en" ? "EN" : "PROG"));
    
    std::string time;
    if (selectedDuration == -1) time = "Inf";
    else if (selectedDuration == 0) time = "30s";
    else time = std::to_string(selectedDuration) + "s";

    std::string mode = currentMode.empty() ? "N/A" : 
                      (currentMode == "manual" ? "Manual" : "Campaign");
    if (currentLanguage == "prog") mode = "Programmer";

    std::string status = " Lang: " + lang + " | Time: " + time + " | Mode: " + mode + " ";

    // Tentukan lebar bar (dengan padding)
    int barWidth = status.length() + 75;
    if (barWidth < 40) barWidth = 40;
    if (barWidth > w) barWidth = w;

    // Posisikan di tengah
    int startX = (w - barWidth) / 2;

    // Gambar background biru
    terminal.setBackgroundColor(Color::BLUE);
    std::string bg(barWidth, ' ');
    terminal.setCursor(startX + 1, y);
    terminal.print(bg);

    // Tulis teks di atas background
    printCentered(y, status, Color::WHITE);
    terminal.resetColor();
}

/**
 * @brief Mendapatkan input string dari user (untuk input custom)
 * @param digitsOnly True jika hanya menerima angka
 * @return String hasil input user
 */
std::string GameEngine::getStringInput(bool digitsOnly) {
    std::string inputBuf = "";
    int startX = terminal.getWidth() / 2 - 10;
    int startY = terminal.getHeight() / 2 + 1;
    
    terminal.setCursor(startX + inputBuf.length(), startY);
    terminal.showCursor();

    while(true) {
        if (terminal.hasInput()) {
            char c = terminal.getInput();
            
            // ESC - Batalkan input
            if (c == 27) { 
                terminal.hideCursor(); 
                return ""; 
            }
            
            // ENTER - Konfirmasi input
            if (c == 10 || c == 13) { 
                terminal.hideCursor(); 
                return inputBuf; 
            }
            
            // BACKSPACE - Hapus karakter
            if (c == 127 || c == '\b' || c == 8) { 
                if (!inputBuf.empty()) {
                    inputBuf.pop_back();
                    terminal.setCursor(startX + inputBuf.length(), startY);
                    terminal.print(" ");
                    terminal.setCursor(startX + inputBuf.length(), startY);
                    terminal.flush(); // Flush untuk visual feedback
                }
            } 
            // Input karakter
            else if (!digitsOnly || (c >= '0' && c <= '9')) {
                if (c >= 32 && c <= 126 && inputBuf.length() < 20) {
                     inputBuf += c;
                     terminal.print(std::string(1, c));
                     terminal.flush(); // Flush untuk visual feedback
                }
            }
        }
    }
}

// ============================================================================
// MENU HANDLERS
// ============================================================================

/**
 * @brief Handler untuk menu pemilihan bahasa
 * 
 * User memilih antara: Indonesia (ID), English (EN), atau Quit
 */
void GameEngine::handleMenuLanguage() {
    int lastW = 0, lastH = 0;

    while (true) {
        int currW = terminal.getWidth();
        int currH = terminal.getHeight();

        // Redraw hanya jika ukuran berubah (optimasi performa)
        if (currW != lastW || currH != lastH) {
            lastW = currW;
            lastH = currH;
            terminal.clear();

            int h = terminal.getHeight();
            int w = terminal.getWidth();
            int cy = h / 2;
            int cx = w / 2;

            // ASCII Art Banner
            std::string title1 = "  ______    ________   ______   ________  ______       ";
            std::string title2 = " /_____/\\  /_______/\\ /_____/\\ /_______/\\/_____/\\      ";
            std::string title3 = " \\:::_ \\ \\ \\::: _  \\ \\\\:::_ \\ \\\\__.::._\\/\\:::_ \\ \\     ";
            std::string title4 = "  \\:(_) ) )_\\::(_)  \\ \\\\:(_) \\ \\  \\::\\ \\  \\:\\ \\ \\ \\    ";
            std::string title5 = "   \\: __ `\\ \\\\:: __  \\ \\\\: ___\\/  _\\::\\ \\__\\:\\ \\ \\ \\   ";
            std::string title6 = "    \\ \\ `\\ \\ \\\\:.\\ \\  \\ \\\\ \\ \\   /__\\::\\__/\\\\:\\/.:| |  ";
            std::string title7 = "     \\_\\/ \\_\\/ \\__\\/\\__\\/ \\_\\/   \\________\\/ \\____/_/  ";
            std::string subtitle = "RAPID TEXTER";

            // Kotak menu
            int boxW = 68; 
            int boxH = 18; 
            drawBox(cx - boxW / 2, cy - boxH / 2, boxW, boxH, Color::CYAN);

            // Tampilkan judul dan menu
            printCentered(cy - 7, title1, Color::CYAN);
            printCentered(cy - 6, title2, Color::CYAN);
            printCentered(cy - 5, title3, Color::CYAN);
            printCentered(cy - 4, title4, Color::CYAN);
            printCentered(cy - 3, title5, Color::CYAN);
            printCentered(cy - 2, title6, Color::CYAN);
            printCentered(cy - 1, title7, Color::CYAN);
            
            printCentered(cy + 1, subtitle, Color::BLUE);

            printCentered(cy + 3, "[1] Indonesia (ID)");
            printCentered(cy + 4, "[2] English (EN)");
            printCentered(cy + 6, "(Q) Quit", Color::RED);

            drawStatusBar();
            
            // Flush setelah rendering selesai
            terminal.flush();
        }

        // Handle input
        if (terminal.hasInput()) {
            char c = terminal.getInput();
            if (c == 'q' || c == 'Q') { 
                currentState = GameState::EXIT; 
                return; 
            }
            if (c == '1') { 
                currentLanguage = "id"; 
                originalLanguage = "id"; // Simpan bahasa asli
                currentState = GameState::MENU_DURATION; 
                return; 
            }
            if (c == '2') { 
                currentLanguage = "en"; 
                originalLanguage = "en"; // Simpan bahasa asli
                currentState = GameState::MENU_DURATION; 
                return; 
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

/**
 * @brief Handler untuk menu pemilihan durasi
 * 
 * User memilih: 15s, 30s, 60s, Custom, atau Tanpa Batas
 */
void GameEngine::handleMenuDuration() {
    int lastW = 0, lastH = 0;

    while (true) {
        int currW = terminal.getWidth();
        int currH = terminal.getHeight();

        if (currW != lastW || currH != lastH) {
            lastW = currW;
            lastH = currH;
            terminal.clear();

            int h = terminal.getHeight();
            int w = terminal.getWidth();
            int cy = h / 2;
            int cx = w / 2;

            int boxW = 75;
            int boxH = 14;
            drawBox(cx - boxW / 2, cy - boxH / 2, boxW, boxH, Color::MAGENTA);

            printCentered(cy - 5, "SELECT DURATION", Color::MAGENTA);
            printCentered(cy - 2, "[1] 15 Seconds");
            printCentered(cy - 1, "[2] 30 Seconds");
            printCentered(cy, "[3] 60 Seconds");
            printCentered(cy + 1, "[4] Custom");
            printCentered(cy + 2, "[5] Tanpa Waktu");
            printCentered(cy + 4, "(B) Back", Color::YELLOW);

            drawStatusBar();
            
            // Flush setelah rendering selesai
            terminal.flush();
        }

        if (terminal.hasInput()) {
            char c = terminal.getInput();
            if (c == 'b' || c == 'B') { 
                currentState = GameState::MENU_LANGUAGE; 
                return; 
            }
            if (c == '1') { 
                selectedDuration = 15; 
                currentState = GameState::MENU_MODE; 
                return; 
            }
            if (c == '2') { 
                selectedDuration = 30; 
                currentState = GameState::MENU_MODE; 
                return; 
            }
            if (c == '3') { 
                selectedDuration = 60; 
                currentState = GameState::MENU_MODE; 
                return; 
            }
            if (c == '5') { 
                selectedDuration = -1; 
                currentState = GameState::MENU_MODE; 
                return; 
            }
            
            // Custom duration input
            if (c == '4') {
                terminal.clear();
                int w = terminal.getWidth(); 
                int h = terminal.getHeight();
                int cy = h / 2; 
                int cx = w / 2;

                drawBox(cx - 30, cy - 5, 60, 12, Color::MAGENTA);
                printCentered(cy - 2, "Enter Duration (seconds):", Color::WHITE);
                printCentered(cy + 3, "(ESC) Cancel", Color::YELLOW);
                terminal.flush(); // Flush before input
                
                std::string inp = getStringInput(true);
                lastW = 0; // Force redraw

                if (inp.empty()) continue;
                
                try {
                    int val = std::stoi(inp);
                    if (val > 0) {
                        selectedDuration = val;
                        currentState = GameState::MENU_MODE;
                        return;
                    }
                } catch (...) {}
                continue;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

/**
 * @brief Handler untuk menu pemilihan mode
 * 
 * User memilih: Manual Mode atau Campaign Mode
 */
void GameEngine::handleMenuMode() {
    int lastW = 0, lastH = 0;

    while (true) {
        int currW = terminal.getWidth();
        int currH = terminal.getHeight();

        if (currW != lastW || currH != lastH) {
            lastW = currW;
            lastH = currH;
            terminal.clear();

            int h = terminal.getHeight();
            int w = terminal.getWidth();
            int cy = h / 2;
            int cx = w / 2;

            int boxW = 50;
            int boxH = 12;
            drawBox(cx - boxW / 2, cy - boxH / 2, boxW, boxH, Color::GREEN);

            printCentered(cy - 4, "SELECT MODE", Color::GREEN);
            printCentered(cy, "[1] Manual Mode");
            printCentered(cy + 1, "[2] Campaign Mode");
            printCentered(cy + 3, "(B) Back", Color::YELLOW);

            drawStatusBar();
            
            // Flush setelah rendering selesai
            terminal.flush();
        }

        if (terminal.hasInput()) {
            char c = terminal.getInput();
            if (c == 'b' || c == 'B') { 
                currentState = GameState::MENU_DURATION; 
                return; 
            }
            if (c == '1') { 
                currentMode = "manual"; 
                currentState = GameState::MENU_DIFFICULTY; 
                return; 
            }
            if (c == '2') { 
                currentMode = "campaign"; 
                currentState = GameState::MENU_DIFFICULTY; 
                return; 
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

/**
 * @brief Handler untuk menu pemilihan kesulitan/setup
 * 
 * Manual Mode: User input target WPM
 * Campaign Mode: User pilih difficulty level (Easy/Medium/Hard/Programmer)
 */
void GameEngine::handleMenuDifficulty() {
    int lastW = 0, lastH = 0;

    // === MANUAL MODE: Custom WPM Target ===
    if (currentMode == "manual") {
        std::string inputBuf = "";

        while (true) {
            int currW = terminal.getWidth();
            int currH = terminal.getHeight();
            bool sizeChanged = (currW != lastW || currH != lastH);

            if (sizeChanged) {
                lastW = currW;
                lastH = currH;
                terminal.clear();

                int h = terminal.getHeight();
                int w = terminal.getWidth();
                int cy = h / 2;
                int cx = w / 2;

                int boxW = 50;
                int boxH = 10;
                drawBox(cx - boxW / 2, cy - boxH / 2, boxW, boxH, Color::BLUE);
                printCentered(cy - 3, "MANUAL SETUP", Color::BLUE);
                printCentered(cy - 1, "Enter Target WPM:", Color::WHITE);
                printCentered(cy + 3, "(ESC) Back | (ENTER) Confirm", Color::YELLOW);

                drawStatusBar();

                // Tampilkan input buffer
                int inputX = cx - 10;
                int inputY = cy + 1;
                terminal.setCursor(inputX, inputY);
                terminal.print(inputBuf);
                terminal.setCursor(inputX + inputBuf.length(), inputY);
                terminal.showCursor();
                
                // Flush setelah rendering selesai
                terminal.flush();
            }

            // Handle input manual
            if (terminal.hasInput()) {
                char c = terminal.getInput();

                // ENTER - Konfirmasi
                if (c == 10 || c == 13) {
                    terminal.hideCursor();
                    if (inputBuf.empty()) { 
                        currentState = GameState::MENU_MODE; 
                        return; 
                    }
                    try { 
                        targetWPM = std::stoi(inputBuf); 
                    } catch (...) { 
                        targetWPM = 40; 
                    }
                    currentDifficulty = Difficulty::MEDIUM;
                    currentState = GameState::PLAYING;
                    resetSession();
                    return;
                }
                // ESC - Kembali
                else if (c == 27) {
                    terminal.hideCursor();
                    currentState = GameState::MENU_MODE;
                    return;
                }
                // BACKSPACE - Hapus
                else if (c == 127 || c == '\b' || c == 8) {
                    if (!inputBuf.empty()) {
                        inputBuf.pop_back();
                        lastW = 0; // Force redraw
                    }
                }
                // ANGKA - Ketik
                else if (c >= '0' && c <= '9') {
                    if (inputBuf.length() < 5) {
                        inputBuf += c;
                        terminal.print(std::string(1, c));
                        terminal.flush(); // Flush untuk visual feedback
                    }
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }
    }

    // === CAMPAIGN MODE: Difficulty Selection ===
    while (true) {
        int currW = terminal.getWidth();
        int currH = terminal.getHeight();

        if (currW != lastW || currH != lastH) {
            lastW = currW;
            lastH = currH;
            terminal.clear();

            int h = terminal.getHeight();
            int w = terminal.getWidth();
            int cy = h / 2;
            int cx = w / 2;

            int boxW = 60;
            int boxH = 20;
            drawBox(cx - boxW / 2, cy - boxH / 2, boxW, boxH, Color::MAGENTA);

            printCentered(cy - 8, "CAMPAIGN DIFFICULTY", Color::MAGENTA);

            // Status lock dan persyaratan
            bool easyUnlocked = unlockedDifficulties[Difficulty::EASY];
            bool mediumUnlocked = unlockedDifficulties[Difficulty::MEDIUM];
            bool hardUnlocked = unlockedDifficulties[Difficulty::HARD];

            std::string easyText = easyUnlocked ? "[1] Easy" : "[1] Easy [LOCKED]";
            std::string mediumText = mediumUnlocked ? "[2] Medium" : "[2] Medium [LOCKED]";
            std::string hardText = hardUnlocked ? "[3] Hard" : "[3] Hard [LOCKED]";

            Color cE = easyUnlocked ? Color::WHITE : Color::RED;
            Color cM = mediumUnlocked ? Color::WHITE : Color::RED;
            Color cH = hardUnlocked ? Color::WHITE : Color::RED;

            printCentered(cy - 5, easyText, cE);
            printCentered(cy - 4, mediumText, cM);
            printCentered(cy - 3, hardText, cH);
            printCentered(cy - 1, "[4] Programmer Mode", Color::CYAN);
            
            printCentered(cy + 2, "Requirements:", Color::YELLOW);
            printCentered(cy + 3, "Easy -> Medium: 40 WPM, 80% Accuracy");
            printCentered(cy + 4, "Medium -> Hard: 60 WPM, 90% Accuracy");
            printCentered(cy + 5, "Hard Complete: 70 WPM, 90% Accuracy");
            
            printCentered(cy + 7, "(B) Back | (C) Credits", Color::YELLOW);
            drawStatusBar();
            
            // Flush setelah rendering selesai
            terminal.flush();
        }

        if (terminal.hasInput()) {
            char d = terminal.getInput();
            
            // Restore bahasa saat user kembali dari menu
            if (d == 'b' || d == 'B') {
                restoreLanguageFromProgrammerMode();
                currentState = GameState::MENU_MODE; 
                return; 
            }
            
            // Shortcut ke Credits
            if (d == 'c' || d == 'C') {
                previousState = GameState::MENU_DIFFICULTY;
                currentState = GameState::CREDITS;
                return;
            }

            bool valid = false;
            if (d == '1' && unlockedDifficulties[Difficulty::EASY]) { 
                currentDifficulty = Difficulty::EASY; 
                valid = true; 
            }
            if (d == '2' && unlockedDifficulties[Difficulty::MEDIUM]) { 
                currentDifficulty = Difficulty::MEDIUM; 
                valid = true; 
            }
            if (d == '3' && unlockedDifficulties[Difficulty::HARD]) { 
                currentDifficulty = Difficulty::HARD; 
                valid = true; 
            }
            if (d == '4') { 
                currentDifficulty = Difficulty::PROGRAMMER; 
                currentLanguage = "prog"; 
                valid = true; 
            }

            if (valid) {
                currentState = GameState::PLAYING;
                resetSession();
                return;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

// ============================================================================
// GAME SESSION FUNCTIONS
// ============================================================================

/**
 * @brief Reset semua data sesi permainan sebelum mulai
 * 
 * Mengambil kata acak, reset statistik, dan siapkan timer.
 * 
 * CATATAN PENTING:
 * - Function ini TIDAK restore bahasa dari Programmer Mode
 * - Karena digunakan untuk restart (TAB) di tengah permainan
 * - Bahasa hanya di-restore saat benar-benar keluar (ESC atau selesai)
 */
void GameEngine::resetSession() {
    // Ambil kata-kata acak dari TextProvider
    targetWords = textProvider.getWords(currentLanguage, currentDifficulty, 30); 
    
    // Gabungkan vector kata menjadi string dengan spasi
    flatTargetString = "";
    for (size_t i = 0; i < targetWords.size(); ++i) {
        flatTargetString += targetWords[i];
        if (i < targetWords.size() - 1) flatTargetString += " ";
    }
    
    typedString = "";
    cursorPosition = 0;
    currentStats.reset();
    timeLimitSeconds = selectedDuration; 
    isGameStarted = false;
    terminal.clear(); 
}

/**
 * @brief Render tampilan saat game berlangsung dengan word-aware wrapping
 * 
 * Menggunakan word-aware wrapping yang tidak memotong kata di tengah.
 * Algoritma: Render per-kata, cek apakah kata muat di baris saat ini sebelum render.
 * 
 * Menampilkan kata-kata dengan highlight warna:
 * - Putih: Belum diketik
 * - Hijau: Benar
 * - Merah: Salah
 */
void GameEngine::renderGame() {
    int w = terminal.getWidth();
    int h = terminal.getHeight();
    int cy = h / 2;
    int cx = w / 2;
    
    // Hitung sisa waktu / waktu berjalan
    std::string timeStr;
    if (selectedDuration == -1) {
        if (isGameStarted) {
             auto now = std::chrono::steady_clock::now();
             int elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();
             timeStr = std::to_string(elapsed) + "s";
        } else {
             timeStr = "Inf";
        }
    } else {
        timeStr = std::to_string(timeLimitSeconds);
    }

    // Header info
    std::string info = " MODE: " + currentMode + " | TIME: " + timeStr + "   ";
    terminal.setCursor(cx - info.length()/2, 2);
    terminal.resetColor();
    terminal.print(info);

    // Footer controls
    std::string footer = "TAB: Restart | ESC: Exit";
    terminal.setCursor(cx - (footer.length()/2), h - 2);
    terminal.setColor(Color::YELLOW);
    terminal.print(footer);
    terminal.resetColor();

    // Area pengetikan dengan word wrap
    int boxWidth = 60;
    if (w < 64) boxWidth = w - 4;
    int startX = (w - boxWidth) / 2;
    int startY = cy - 5; 

    int curX = startX;
    int curY = startY;
    int globalCharIndex = 0; // Track posisi global di flatTargetString
    
    // Render per-kata untuk mencegah kata terpotong
    for (size_t wordIdx = 0; wordIdx < targetWords.size(); ++wordIdx) {
        const std::string& word = targetWords[wordIdx];
        int wordLength = word.length();
        
        // Cek apakah kata muat di baris saat ini
        // Jika tidak muat dan bukan di awal baris, pindah ke baris baru
        if (curX + wordLength > startX + boxWidth && curX > startX) {
            curX = startX;
            curY += 2;
        }
        
        // Render setiap karakter dalam kata
        for (int charIdx = 0; charIdx < wordLength; ++charIdx) {
            char targetChar = word[charIdx];
            char charToDraw = targetChar;
            Color color = Color::WHITE;
            
            // Tentukan warna berdasarkan status ketikan
            if (globalCharIndex < (int)typedString.length()) {
                char typedChar = typedString[globalCharIndex];
                if (typedChar == targetChar) {
                    color = Color::GREEN; // Benar
                } else {
                    color = Color::RED;   // Salah
                    if (targetChar == ' ') {
                        charToDraw = typedChar;
                        if (charToDraw == ' ') charToDraw = '_';
                    }
                }
            }
            
            terminal.setColor(color);
            terminal.printAt(curX, curY, std::string(1, charToDraw));
            
            // Indikator posisi kursor (^)
            terminal.setCursor(curX, curY + 1);
            if (globalCharIndex == cursorPosition) {
                terminal.setColor(Color::YELLOW);
                terminal.print("^");
            } else {
                terminal.print(" ");
            }
            
            curX++;
            globalCharIndex++;
        }
        
        // Tambahkan spasi antar kata (kecuali kata terakhir)
        if (wordIdx < targetWords.size() - 1) {
            char spaceChar = ' ';
            Color spaceColor = Color::WHITE;
            
            // Cek status spasi
            if (globalCharIndex < (int)typedString.length()) {
                char typedChar = typedString[globalCharIndex];
                if (typedChar == ' ') {
                    spaceColor = Color::GREEN;
                } else {
                    spaceColor = Color::RED;
                    spaceChar = typedChar;
                }
            }
            
            terminal.setColor(spaceColor);
            terminal.printAt(curX, curY, std::string(1, spaceChar));
            
            // Indikator kursor untuk spasi
            terminal.setCursor(curX, curY + 1);
            if (globalCharIndex == cursorPosition) {
                terminal.setColor(Color::YELLOW);
                terminal.print("^");
            } else {
                terminal.print(" ");
            }
            
            curX++;
            globalCharIndex++;
        }
    }
    
    terminal.resetColor();
    
    // Instruksi awal
    if (!isGameStarted) {
        printCentered(cy + 6, "Type to start...", Color::WHITE);
    } else {
         printCentered(cy + 6, "                 ", Color::WHITE);
    }
    
    terminal.hideCursor();
    
    // Flush semua output buffer setelah rendering selesai
    // Ini menggantikan ratusan flush() individual yang menyebabkan delay
    terminal.flush();
}

/**
 * @brief Loop utama gameplay - Menangani input dan timer
 */
void GameEngine::gameLoop() {
    terminal.clear();

    int lastW = terminal.getWidth();
    int lastH = terminal.getHeight();

    while (currentState == GameState::PLAYING) {
        int currW = terminal.getWidth();
        int currH = terminal.getHeight();

        // Deteksi resize terminal
        if (currW != lastW || currH != lastH) {
            terminal.clear();
            lastW = currW;
            lastH = currH;
        }

        // Timer logic
        if (isGameStarted) {
            auto now = std::chrono::steady_clock::now();
            int elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();

            if (selectedDuration != -1) {
                int remaining = selectedDuration - elapsed;
                timeLimitSeconds = remaining;

                if (remaining <= 0) {
                    currentState = GameState::RESULTS;
                    return;
                }
            } else {
                timeLimitSeconds = elapsed;
            }
        }

        renderGame();

        // Non-blocking input check
        if (terminal.hasInput()) {
            char c = terminal.getInput();
            processInput(c);
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }
    }
}

/**
 * @brief Memproses setiap input keyboard saat gameplay
 * @param c Karakter yang ditekan
 * 
 * Menangani ESC (exit), TAB (restart), BACKSPACE, dan karakter biasa
 * 
 * - Restore bahasa dari Programmer Mode saat ESC ditekan
 */
void GameEngine::processInput(char c) {
    // ESC - Kembali ke menu
    if (c == 27) {
        // Restore bahasa jika keluar dari Programmer Mode
        restoreLanguageFromProgrammerMode();
        
        currentState = GameState::MENU_DIFFICULTY; 
        return;
    }
    
    // TAB - Restart sesi
    if (c == 9) { 
        resetSession();
        return;
    }
    
    // BACKSPACE - Hapus karakter dengan smart lock
    if (c == 127 || c == '\b' || c == 8) { 
        int lockedLimit = 0;

        // Cari checkpoint terdekat (kata sebelumnya yang sudah benar)
        for (int i = cursorPosition - 1; i >= 0; --i) {
            if (i < (int)flatTargetString.length() && flatTargetString[i] == ' ') {
                bool match = true;
                if (i < (int)typedString.length()) {
                    for (int k = 0; k <= i; ++k) {
                        if (typedString[k] != flatTargetString[k]) {
                            match = false;
                            break;
                        }
                    }
                    if (match) {
                        lockedLimit = i + 1;
                        break;
                    }
                }
            }
        }

        // Hanya hapus jika di atas batas kunci
        if (cursorPosition > lockedLimit) {
            cursorPosition--;
            typedString.pop_back();
        }
    } 
    // Karakter biasa
    else {
        // Start timer saat karakter pertama ditekan
        if (!isGameStarted) {
            isGameStarted = true;
            startTime = std::chrono::steady_clock::now();
        }

        if (cursorPosition < (int)flatTargetString.length()) {
            char targetChar = flatTargetString[cursorPosition];
            typedString += c;
            
            // Update statistik
            if (targetChar == c) {
                currentStats.correctKeystrokes++;
            } else {
                currentStats.errors++;
            }
            currentStats.totalKeystrokes++;
            cursorPosition++;
            
            // Cek apakah sudah selesai
            if (cursorPosition >= (int)flatTargetString.length()) {
                currentState = GameState::RESULTS;
            }
        }
    }
}

// ============================================================================
// RESULTS SCREEN
// ============================================================================

/**
 * @brief Menampilkan layar hasil dengan statistik lengkap
 * 
 * Ditambahkan aggressive input clearing dan safety delay sebelum
 * Rick Roll untuk mencegah premature exit dari script eksternal.
 * 
 * - Cek hardJustCompleted SEBELUM masuk loop rendering
 * - Setelah Rick Roll selesai, langsung redirect ke Credits
 * - Mencegah input buffer yang tertinggal dari Rick Roll
 * 
 * Setelah selesai Programmer mode, kembalikan bahasa ke pilihan awal user.
 * Menampilkan WPM, Accuracy, Time, Errors, dan status unlock (campaign)
 */
void GameEngine::showResults() {
    // Hitung waktu yang dihabiskan
    double seconds = 0;
    if (isGameStarted) {
        auto now = std::chrono::steady_clock::now();
        seconds = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count() / 1000.0;
        if (selectedDuration != -1 && seconds > selectedDuration) 
            seconds = selectedDuration;
    }
    
    currentStats.timeTaken = seconds;
    currentStats.calculate((int)flatTargetString.length());

    bool shouldShowRickRoll = false;
    bool hardJustCompleted = false;
    
    // Cek apakah ini pertama kali menyelesaikan Hard mode
    if (currentMode == "campaign" && currentDifficulty == Difficulty::HARD) {
        if (currentStats.wpm >= 70 && currentStats.accuracy >= 90) {
            if (!hardCompleted) {
                hardCompleted = true;
                shouldShowRickRoll = true;
                hardJustCompleted = true;
            }
        }
    }

    // Jika baru menyelesaikan Hard, langsung tampilkan pesan selamat, lalu Rick Roll, lalu Credits
    if (shouldShowRickRoll) {
        terminal.clear();
        int h = terminal.getHeight();
        int w = terminal.getWidth();
        int cy = h / 2;
        int cx = w / 2;

        // Tampilkan pesan selamat terlebih dahulu
        int boxW = 60;
        int boxH = 15;
        drawBox(cx - boxW / 2, cy - boxH / 2, boxW, boxH, Color::CYAN);
        
        printCentered(cy - 5, "RESULTS", Color::CYAN);
        printCentered(cy - 2, "WPM: " + std::to_string((int)currentStats.wpm), Color::GREEN);
        printCentered(cy - 1, "Accuracy: " + std::to_string((int)currentStats.accuracy) + "%", Color::WHITE);
        printCentered(cy + 1, "HARD MODE COMPLETED!!!", Color::GREEN);
        printCentered(cy + 2, "CONGRATULATIONS!!!", Color::GREEN);
        printCentered(cy + 4, "Preparing special surprise...", Color::YELLOW);
        
        // Flush sebelum delay
        terminal.flush();
        
        // Aggressive input buffer clearing sebelum delay
        // Problem: Input dari gameplay masih tersisa dan akan trigger Rick Roll exit
        // Solution: Clear semua input yang tertinggal
        while (terminal.hasInput()) {
            terminal.getInput();
        }
        
        // Delay agar user sempat membaca pesan
        std::this_thread::sleep_for(std::chrono::seconds(3));
        
        // Clear input buffer LAGI sebelum Rick Roll
        // User mungkin menekan tombol selama delay 3 detik
        while (terminal.hasInput()) {
            terminal.getInput();
        }
        
        // Safety delay tambahan untuk stabilisasi
        // Memastikan tidak ada keystroke yang tertinggal di OS keyboard buffer
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        // Jalankan Rick Roll dengan input buffer yang bersih
        playRickRoll();
        
        // Set flag bahwa Rick Roll sudah ditampilkan
        rickRollAlreadyShown = true;
        
        // Aggressive clearing setelah Rick Roll
        // Script mungkin meninggalkan input buffer yang kotor
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        while (terminal.hasInput()) {
            terminal.getInput();
        }
        
        // Reset session sebelum pindah ke Credits
        // Menggunakan resetSession() yang sudah ada untuk cleanup
        // Mencegah duplicate results dari state yang kotor (isGameStarted, stats, dll)
        resetSession();
        
        // Additional input buffer clearing untuk safety
        while (terminal.hasInput()) {
            terminal.getInput();
        }
        
        // Langsung redirect ke Credits setelah Rick Roll
        previousState = GameState::MENU_DIFFICULTY;
        currentState = GameState::CREDITS;
        return;
    }

    // ========================================================================
    // RENDER LOOP NORMAL (JIKA TIDAK ADA RICK ROLL)
    // ========================================================================
    int lastW = 0, lastH = 0;

    while (true) {
        int currW = terminal.getWidth();
        int currH = terminal.getHeight();

        if (currW != lastW || currH != lastH) {
            lastW = currW;
            lastH = currH;
            terminal.clear();

            int h = terminal.getHeight();
            int w = terminal.getWidth();
            int cy = h / 2;
            int cx = w / 2;

            int boxW = 50;
            int boxH = 20;
            drawBox(cx - boxW / 2, cy - boxH / 2, boxW, boxH, Color::CYAN);

            printCentered(cy - 8, "RESULTS", Color::CYAN);

            // Posisi label dan value (aligned)
            int labelX = cx - 12;
            int valueX = cx + 5;

            // WPM
            terminal.setCursor(labelX, cy - 5); 
            terminal.print("WPM:");
            terminal.setCursor(valueX, cy - 5); 
            terminal.setColor(Color::GREEN);
            terminal.print(std::to_string((int)currentStats.wpm)); 
            terminal.resetColor();

            // Accuracy
            terminal.setCursor(labelX, cy - 4); 
            terminal.print("Accuracy:");
            terminal.setCursor(valueX, cy - 4); 
            terminal.print(std::to_string((int)currentStats.accuracy) + "%");

            // Time
            terminal.setCursor(labelX, cy - 3); 
            terminal.print("Time:");
            terminal.setCursor(valueX, cy - 3);
            std::stringstream stream; 
            stream << std::fixed << std::setprecision(1) << seconds;
            terminal.print(stream.str() + "s");

            // Errors
            terminal.setCursor(labelX, cy - 2); 
            terminal.print("Errors:");
            terminal.setCursor(valueX, cy - 2); 
            terminal.setColor(Color::RED);
            terminal.print(std::to_string(currentStats.errors)); 
            terminal.resetColor();

            // Target (Manual Mode Only)
            if (currentMode == "manual") {
                terminal.setCursor(labelX, cy - 1); 
                terminal.print("Target:");
                terminal.setCursor(valueX, cy - 1); 
                terminal.setColor(Color::YELLOW);
                terminal.print(std::to_string(targetWPM)); 
                terminal.resetColor();
            }

            // Status dan pesan unlock
            std::string msg = "";
            Color msgColor = Color::YELLOW;

            if (currentMode == "campaign") {
                bool pass = false;
                std::string requirement = "";
                
                // Cek status pass/fail berdasarkan difficulty
                if (currentDifficulty == Difficulty::EASY) {
                    requirement = "Need: 40 WPM, 80% Accuracy";
                    if (currentStats.wpm >= 40 && currentStats.accuracy >= 80) {
                        pass = true;
                        msg = "LEVEL PASSED! Medium Unlocked!";
                        msgColor = Color::GREEN;
                        unlockedDifficulties[Difficulty::MEDIUM] = true;
                    } else {
                        msg = "LEVEL FAILED";
                        msgColor = Color::RED;
                    }
                } 
                else if (currentDifficulty == Difficulty::MEDIUM) {
                    requirement = "Need: 60 WPM, 90% Accuracy";
                    if (currentStats.wpm >= 60 && currentStats.accuracy >= 90) {
                        pass = true;
                        msg = "LEVEL PASSED! Hard Unlocked!";
                        msgColor = Color::GREEN;
                        unlockedDifficulties[Difficulty::HARD] = true;
                    } else {
                        msg = "LEVEL FAILED";
                        msgColor = Color::RED;
                    }
                }
                else if (currentDifficulty == Difficulty::HARD) {
                    requirement = "Need: 70 WPM, 90% Accuracy";
                    if (currentStats.wpm >= 70 && currentStats.accuracy >= 90) {
                        // Jika sudah pernah complete sebelumnya
                        if (hardCompleted) {
                            msg = "HARD MODE PASSED!";
                            msgColor = Color::GREEN;
                        }
                    } else {
                        msg = "LEVEL FAILED";
                        msgColor = Color::RED;
                    }
                }
                else if (currentDifficulty == Difficulty::PROGRAMMER) {
                    msg = "Programmer Mode Completed!";
                    msgColor = Color::CYAN;
                }
                
                // Tampilkan pesan dan requirement
                if (!msg.empty()) {
                    printCentered(cy + 1, msg, msgColor);
                }
                if (!requirement.empty() && !pass) {
                    printCentered(cy + 2, requirement, Color::YELLOW);
                }
            }
            // Manual Mode - Cek apakah target tercapai
            else if (currentMode == "manual") {
                if (currentStats.wpm >= targetWPM) { 
                    msg = "TARGET REACHED!"; 
                    msgColor = Color::GREEN; 
                } else { 
                    msg = "TARGET MISSED!"; 
                    msgColor = Color::RED; 
                }
                printCentered(cy + 2, msg, msgColor);
            }

            printCentered(cy + 5, "(C) Credits", Color::YELLOW);
            printCentered(cy + 6, "Press ENTER to continue", Color::WHITE);
            
            // Flush setelah rendering selesai
            terminal.flush();
        }

        // Wait for ENTER or C
        if (terminal.hasInput()) {
            char c = terminal.getInput();
            if (c == 10 || c == 13) break;
            if (c == 'c' || c == 'C') {
                previousState = GameState::RESULTS;
                currentState = GameState::CREDITS;
                return;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    // Restore bahasa asli setelah selesai Programmer mode
    restoreLanguageFromProgrammerMode();

    currentState = GameState::MENU_DIFFICULTY;
}

// ============================================================================
// CREDITS SCREEN
// ============================================================================

/**
 * @brief Menampilkan layar credits dengan Rick Roll easter egg
 * 
 * Tidak keluar dari alternate screen sebelum Rick Roll
 * untuk mencegah "flash" ke CLI sistem.
 * 
 * - Cek flag rickRollAlreadyShown untuk mencegah Rick Roll dipanggil dua kali
 * - Jika dipanggil setelah Hard completion, skip Rick Roll (sudah ditampilkan di showResults)
 * - Jika dipanggil dari menu (tekan C), tampilkan Rick Roll
 * - Clear input buffer setelah Rick Roll untuk mencegah input tertinggal
 * - Reset flag setelah selesai agar Rick Roll bisa tampil lagi di sesi berikutnya
 */
void GameEngine::showCredits() {
    // Hanya jalankan Rick Roll jika belum ditampilkan (dipanggil dari menu)
    if (!rickRollAlreadyShown) {
        // Aggressive input clearing sebelum Rick Roll
        // Problem: Input dari menu masih tersisa dan akan trigger Rick Roll exit
        // Solution: Clear semua input yang tertinggal
        while (terminal.hasInput()) {
            terminal.getInput();
        }
        
        // Safety delay untuk stabilisasi input buffer
        // Memastikan tidak ada keystroke yang tertinggal di OS buffer
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        
        // Clear sekali lagi untuk memastikan buffer benar-benar kosong
        while (terminal.hasInput()) {
            terminal.getInput();
        }
        
        playRickRoll();
        
        // Bersihkan semua input yang mungkin tertinggal dari Rick Roll script
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        while (terminal.hasInput()) {
            terminal.getInput();
        }
        
        // Delay singkat untuk memastikan terminal state stabil
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Reset flag untuk sesi berikutnya (jika user tekan C lagi dari menu)
    rickRollAlreadyShown = false;
    
    int lastW = 0, lastH = 0;

    while (true) {
        int currW = terminal.getWidth();
        int currH = terminal.getHeight();

        if (currW != lastW || currH != lastH) {
            lastW = currW;
            lastH = currH;
            terminal.clear();

            int h = terminal.getHeight();
            int w = terminal.getWidth();
            int cy = h / 2;

            int boxW = 50;
            int boxH = 18;
            drawBox((w - boxW) / 2, cy - boxH / 2, boxW, boxH, Color::MAGENTA);

            printCentered(cy - 7, "CREDITS", Color::MAGENTA);
            printCentered(cy - 5, "Developed by:", Color::CYAN);
            
            // Nama-nama developer
            printCentered(cy - 3, "Alea Farrel", Color::WHITE);
            printCentered(cy - 2, "Hensa Katelu", Color::WHITE);
            printCentered(cy - 1, "Yanuar Adi Candra", Color::WHITE);
            printCentered(cy, "Arif Wibowo P.", Color::WHITE);
            printCentered(cy + 1, "Aria Mahendra U.", Color::WHITE);
            
            printCentered(cy + 4, "Thank you for playing!", Color::GREEN);
            printCentered(cy + 6, "Press ENTER to return", Color::YELLOW);
            
            // Flush setelah rendering selesai
            terminal.flush();
        }

        if (terminal.hasInput()) {
            char c = terminal.getInput();
            if (c == 10 || c == 13) {
                // Simplified reset (tidak perlu cleanup+re-initialize)
                // Karena kita tidak keluar dari alternate screen di Rick Roll
                
                // Clear screen dan reset attributes
                terminal.clear();
                
                // Manual ANSI reset untuk safety
                std::cout << "\033[0m";          // Reset all SGR attributes
                std::cout << "\033[r";           // Reset scroll region
                std::cout << "\033(B";           // Reset character set
                std::cout.flush();
                
                // Delay singkat untuk stabilisasi
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                
                // Additional safety reset untuk mencegah corrupt state
                // Terutama penting setelah Hard completion flow (rickroll -> credits -> menu)
                // Menggunakan resetSession() yang sudah ada untuk cleanup
                resetSession();
                
                // Clear input buffer untuk safety
                while (terminal.hasInput()) {
                    terminal.getInput();
                }
                
                // Kembali ke state sebelumnya
                currentState = previousState;
                return;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}