/**
 * @file GameEngine.cpp
 * @brief Implementasi logic utama aplikasi Rapid Texter
 * @author Alea Farrel
 * @date 2025
 * 
 * File ini mengimplementasikan State Machine untuk mengelola alur aplikasi,
 * dari menu awal hingga gameplay dan hasil akhir.
 */

#include "GameEngine.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <iomanip>
#include <cmath>

// ============================================================================
// CONSTRUCTOR & INITIALIZATION
// ============================================================================

/**
 * @brief Constructor - Menginisialisasi state awal aplikasi
 * 
 * Menyiapkan terminal, mengatur status unlock awal untuk mode Campaign,
 * dan memuat database kata dari file eksternal.
 */
GameEngine::GameEngine() : currentState(GameState::MENU_LANGUAGE) {
    terminal.initialize();
    
    // Setup unlock status untuk Campaign Mode
    unlockedDifficulties[Difficulty::EASY] = true;         // ✓ Terbuka dari awal
    unlockedDifficulties[Difficulty::MEDIUM] = false;      // ✗ Butuh: 40 WPM, 80% Akurasi (Easy)
    unlockedDifficulties[Difficulty::HARD] = false;        // ✗ Butuh: 60 WPM, 90% Akurasi (Medium)
    unlockedDifficulties[Difficulty::PROGRAMMER] = true;   // ✓ Bonus (selalu terbuka)
    
    // Load word databases
    textProvider.loadWords("id", "assets/id.txt");
    textProvider.loadWords("en", "assets/en.txt");
    textProvider.loadWords("prog", "assets/prog.txt");
    
    selectedDuration = 30; // Default: 30 detik
}

// ============================================================================
// MAIN LOOP (STATE MACHINE)
// ============================================================================

/**
 * @brief Loop utama aplikasi menggunakan State Machine pattern
 * 
 * Mengelola perpindahan antar state (menu bahasa, durasi, mode, difficulty,
 * gameplay, dan hasil).
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
            case GameState::EXIT:
                break;
        }
    }
    terminal.cleanup();
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
                }
            } 
            // Input karakter
            else if (!digitsOnly || (c >= '0' && c <= '9')) {
                if (c >= 32 && c <= 126 && inputBuf.length() < 20) {
                     inputBuf += c;
                     terminal.print(std::string(1, c));
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
            std::string title1 = " ____  _  _  ____  ____  ____ ";
            std::string title2 = "(  _ \\/ )( \\(  _ \\(_  _)(    \\";
            std::string title3 = " )   /) __ ( ) __/ _)(_  ) D (";
            std::string title4 = "(__\\_)\\_)(_/(__)  (____)(____/";
            std::string subtitle = "RAPID TEXTER";

            // Kotak menu
            int boxW = 50;
            int boxH = 13;
            drawBox(cx - boxW / 2, cy - boxH / 2, boxW, boxH, Color::CYAN);

            // Tampilkan judul dan menu
            printCentered(cy - 5, title1, Color::CYAN);
            printCentered(cy - 4, title2, Color::CYAN);
            printCentered(cy - 3, title3, Color::CYAN);
            printCentered(cy - 2, title4, Color::CYAN);
            printCentered(cy, subtitle, Color::BLUE);

            printCentered(cy + 2, "[1] Indonesia (ID)");
            printCentered(cy + 3, "[2] English (EN)");
            printCentered(cy + 5, "(Q) Quit", Color::RED);

            drawStatusBar();
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
                currentState = GameState::MENU_DURATION; 
                return; 
            }
            if (c == '2') { 
                currentLanguage = "en"; 
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
            int boxH = 18;
            drawBox(cx - boxW / 2, cy - boxH / 2, boxW, boxH, Color::MAGENTA);

            printCentered(cy - 7, "CAMPAIGN DIFFICULTY", Color::MAGENTA);

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

            printCentered(cy - 4, easyText, cE);
            printCentered(cy - 3, mediumText, cM);
            printCentered(cy - 2, hardText, cH);
            printCentered(cy, "[4] Programmer Mode", Color::CYAN);
            
            printCentered(cy + 3, "Requirements:", Color::YELLOW);
            printCentered(cy + 4, "Easy -> Medium: 40 WPM, 80% Accuracy");
            printCentered(cy + 5, "Medium -> Hard: 60 WPM, 90% Accuracy");
            
            printCentered(cy + 7, "(B) Back", Color::YELLOW);
            drawStatusBar();
        }

        if (terminal.hasInput()) {
            char d = terminal.getInput();
            if (d == 'b' || d == 'B') { 
                currentState = GameState::MENU_MODE; 
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
                if (currentDifficulty == Difficulty::PROGRAMMER) 
                    currentLanguage = "prog";
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
 * Mengambil kata acak, reset statistik, dan siapkan timer
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
 * @brief Render tampilan saat game berlangsung
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
    
    // Render setiap karakter dengan warna
    for (size_t i = 0; i < flatTargetString.length(); ++i) {
        // Word wrap
        if (curX >= startX + boxWidth) {
            curX = startX;
            curY += 2;
        }

        char targetChar = flatTargetString[i];
        char charToDraw = targetChar;
        Color color = Color::WHITE; 

        // Tentukan warna berdasarkan status ketikan
        if (i < typedString.length()) {
            char typedChar = typedString[i];
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
        if ((int)i == cursorPosition) {
            terminal.setColor(Color::YELLOW);
            terminal.print("^");
        } else {
            terminal.print(" ");
        }

        curX++;
    }
    terminal.resetColor();
    
    // Instruksi awal
    if (!isGameStarted) {
        printCentered(cy + 6, "Type to start...", Color::WHITE);
    } else {
         printCentered(cy + 6, "                 ", Color::WHITE);
    }
    
    terminal.hideCursor(); 
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
 */
void GameEngine::processInput(char c) {
    // ESC - Kembali ke menu
    if (c == 27) { 
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
            int boxH = 18;
            drawBox(cx - boxW / 2, cy - boxH / 2, boxW, boxH, Color::CYAN);

            printCentered(cy - 7, "RESULTS", Color::CYAN);

            // Posisi label dan value (aligned)
            int labelX = cx - 12;
            int valueX = cx + 5;

            // WPM
            terminal.setCursor(labelX, cy - 4); 
            terminal.print("WPM:");
            terminal.setCursor(valueX, cy - 4); 
            terminal.setColor(Color::GREEN);
            terminal.print(std::to_string((int)currentStats.wpm)); 
            terminal.resetColor();

            // Accuracy
            terminal.setCursor(labelX, cy - 3); 
            terminal.print("Accuracy:");
            terminal.setCursor(valueX, cy - 3); 
            terminal.print(std::to_string((int)currentStats.accuracy) + "%");

            // Time
            terminal.setCursor(labelX, cy - 2); 
            terminal.print("Time:");
            terminal.setCursor(valueX, cy - 2);
            std::stringstream stream; 
            stream << std::fixed << std::setprecision(1) << seconds;
            terminal.print(stream.str() + "s");

            // Errors
            terminal.setCursor(labelX, cy - 1); 
            terminal.print("Errors:");
            terminal.setCursor(valueX, cy - 1); 
            terminal.setColor(Color::RED);
            terminal.print(std::to_string(currentStats.errors)); 
            terminal.resetColor();

            // Target (Manual Mode Only) - FIXED ALIGNMENT
            if (currentMode == "manual") {
                terminal.setCursor(labelX, cy); 
                terminal.print("Target:");
                terminal.setCursor(valueX, cy); 
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
                    msg = "Congratulations! You completed Hard mode!";
                    msgColor = Color::CYAN;
                }
                else if (currentDifficulty == Difficulty::PROGRAMMER) {
                    msg = "Programmer Mode Completed!";
                    msgColor = Color::CYAN;
                }
                
                // Tampilkan pesan dan requirement
                if (!msg.empty()) {
                    printCentered(cy + 2, msg, msgColor);
                }
                if (!requirement.empty() && !pass) {
                    printCentered(cy + 3, requirement, Color::YELLOW);
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
                printCentered(cy + 3, msg, msgColor);
            }

            printCentered(cy + 6, "Press ENTER to continue", Color::WHITE);
        }

        // Wait for ENTER
        if (terminal.hasInput()) {
            char c = terminal.getInput();
            if (c == 10 || c == 13) break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    currentState = GameState::MENU_DIFFICULTY;
}