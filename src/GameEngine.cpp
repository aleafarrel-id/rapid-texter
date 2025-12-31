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
GameEngine::GameEngine() : 
    currentState(GameState::MENU_MAIN), 
    previousState(GameState::MENU_MAIN),
    gameUI(terminal) {  // Initialize gameUI dengan terminal reference
    terminal.initialize();
    
    // Preload audio system untuk menghilangkan delay pada pemutaran pertama
    SFXManager::preload();
    
    // Rick Roll sudah ditampilkan atau belum
    rickRollAlreadyShown = false;
    
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
            case GameState::MENU_MAIN:
                handleMenuMain();
                break;
            case GameState::MENU_HISTORY:
                handleMenuHistory();
                break;
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
// MENU HANDLERS
// ============================================================================

/**
 * @brief Handler untuk menu utama
 * 
 * Menu pertama yang muncul saat aplikasi dibuka.
 * User memilih: Start, Show History, atau Quit
 */
void GameEngine::handleMenuMain() {
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

            // ASCII Art Banner (sama seperti menu language lama)
            std::string title1 = "  ______    ________   ______   ________  ______       ";
            std::string title2 = " /_____/\\  /_______/\\ /_____/\\ /_______/\\/_____/\\      ";
            std::string title3 = " \\:::_ \\ \\ \\::: _  \\ \\\\:::_ \\ \\\\__.::._\\/\\:::_ \\ \\     ";
            std::string title4 = "  \\:(_) ) )_\\::(_)  \\ \\\\:(_) \\ \\  \\::\\ \\  \\:\\ \\ \\ \\    ";
            std::string title5 = "   \\: __ `\\ \\\\:: __  \\ \\\\: ___\\/  _\\::\\ \\__\\:\\ \\ \\ \\   ";
            std::string title6 = "    \\ \\ `\\ \\ \\\\:.\\ \\  \\ \\\\ \\ \\   /__\\::\\__/\\\\:\\/.:| |  ";
            std::string title7 = "     \\_\\/ \\_\\/ \\__\\/\\__\\/ \\_\\/   \\________\\/ \\____/_/  ";
            std::string subtitle = "RAPID TEXTER";

            int boxW = 68; 
            int boxH = 20; 
            gameUI.drawBox(cx - boxW / 2, cy - boxH / 2, boxW, boxH, Color::CYAN);

            gameUI.printCentered(cy - 7, title1, Color::CYAN);
            gameUI.printCentered(cy - 6, title2, Color::CYAN);
            gameUI.printCentered(cy - 5, title3, Color::CYAN);
            gameUI.printCentered(cy - 4, title4, Color::CYAN);
            gameUI.printCentered(cy - 3, title5, Color::CYAN);
            gameUI.printCentered(cy - 2, title6, Color::CYAN);
            gameUI.printCentered(cy - 1, title7, Color::CYAN);
            
            gameUI.printCentered(cy + 1, subtitle, Color::BLUE);

            // Menu options
            gameUI.printCentered(cy + 4, "[1] Start Game", Color::GREEN);
            gameUI.printCentered(cy + 5, "[2] Show History", Color::YELLOW);
            gameUI.printCentered(cy + 7, "(Q) Quit", Color::RED);

            // Status bar dengan SFX status
            gameUI.drawStatusBar("", 0, "", SFXManager::isEnabled());
            
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
                SFXManager::playTrue();
                // Start -> ke pemilihan bahasa
                currentState = GameState::MENU_LANGUAGE; 
                return; 
            }
            if (c == '2') { 
                SFXManager::playTrue();
                // Show History
                currentState = GameState::MENU_HISTORY; 
                return; 
            }
            // Toggle SFX dengan shortcut S
            if (c == 's' || c == 'S') {
                SFXManager::toggle();
                SFXManager::playFalse();
                lastW = 0; // Force redraw
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

/**
 * @brief Handler untuk menu history dengan pagination
 * 
 * Menampilkan history permainan dengan sistem pagination.
 * Maksimal 5 entry per halaman.
 */
void GameEngine::handleMenuHistory() {
    int currentPage = 1;
    const int pageSize = 5;
    int totalPages = historyManager.getTotalPages(pageSize);
    int totalEntries = historyManager.getTotalEntries();
    
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

            // Box lebih besar untuk menampung data dengan rapi
            int boxW = 85;
            int boxH = 27;
            gameUI.drawBox(cx - boxW / 2, cy - boxH / 2, boxW, boxH, Color::YELLOW);

            gameUI.printCentered(cy - 11, "GAME HISTORY", Color::YELLOW);
            
            if (totalEntries == 0) {
                // Tidak ada history
                gameUI.printCentered(cy - 1, "No history available yet.", Color::WHITE);
                gameUI.printCentered(cy, "Play some games to see your history here!", Color::CYAN);
            } else {
                // Tampilkan info pagination
                std::string pageInfo = "Page " + std::to_string(currentPage) + " of " + std::to_string(totalPages);
                std::string totalInfo = "(" + std::to_string(totalEntries) + " total entries)";
                gameUI.printCentered(cy - 9, pageInfo, Color::CYAN);
                gameUI.printCentered(cy - 8, totalInfo, Color::WHITE);
                
                // Ambil entries untuk halaman ini
                auto entries = historyManager.getPage(currentPage, pageSize);
                
                // ====================================================================
                // HEADER TABEL
                // ====================================================================
                int startY = cy - 5;
                int startX = cx - 40; // Start dari kiri dengan padding
                
                terminal.setCursor(startX, startY);
                terminal.setColor(Color::CYAN);
                
                // Header dengan format yang konsisten dengan data
                char headerLine[100];
                snprintf(headerLine, sizeof(headerLine), "%-7s%-10s%-8s%-12s%-6s%-12s%s",
                         "WPM", "Accuracy", "Errors", "Difficulty", "Lang", "Mode", "Date/Time");
                terminal.print(headerLine);
                
                terminal.resetColor();
                
                // Garis pemisah
                terminal.setCursor(startX, startY + 1);
                terminal.setColor(Color::CYAN);
                gameUI.printCentered(cy - 4, std::string(boxW - 2, '-'));
                terminal.resetColor();
                
                // ====================================================================
                // DATA ENTRIES DENGAN ALIGNMENT YANG KONSISTEN
                // ====================================================================
                for (size_t i = 0; i < entries.size(); ++i) {
                    const auto& entry = entries[i];
                    // Tambahkan spacing antar entry (i * 2 untuk jarak 1 baris antar entry)
                    int rowY = startY + 3 + static_cast<int>(i) * 2;
                    
                    terminal.setCursor(startX, rowY);
                    
                    // Format row dengan alignment yang sama dengan header
                    char rowLine[120];
                    
                    // Prepare strings dengan padding yang benar
                    char wpmStr[10];
                    snprintf(wpmStr, sizeof(wpmStr), "%.1f", entry.wpm);
                    
                    char accStr[12];
                    snprintf(accStr, sizeof(accStr), "%.1f%%", entry.accuracy);
                    
                    // Truncate strings jika terlalu panjang
                    std::string diffStr = entry.difficulty;
                    if (diffStr.length() > 11) diffStr = diffStr.substr(0, 11);
                    
                    std::string langStr = entry.language;
                    if (langStr.length() > 5) langStr = langStr.substr(0, 5);
                    
                    std::string modeStr = entry.mode;
                    if (modeStr.length() > 11) modeStr = modeStr.substr(0, 11);
                    
                    // Print dengan format yang konsisten
                    snprintf(rowLine, sizeof(rowLine), "%-7s%-10s%-8d%-12s%-6s%-12s%s",
                             wpmStr, accStr, entry.errors, diffStr.c_str(), 
                             langStr.c_str(), modeStr.c_str(), entry.timestamp.c_str());
                    terminal.print(rowLine);
                }
                
                // Garis pemisah bawah tabel (posisi setelah entry terakhir dengan spacing)
                int bottomY = startY + 3 + static_cast<int>(entries.size()) * 2;
                terminal.setCursor(startX, bottomY);
                terminal.setColor(Color::CYAN);
                gameUI.printCentered(bottomY, std::string(boxW - 2, '-'));
                terminal.resetColor();
                
                // ====================================================================
                // NAVIGATION HINTS
                // ====================================================================
                if (totalPages > 1) {
                    gameUI.printCentered(cy + 9, "[1] Previous | [2] Next", Color::CYAN);
                }
            }

            // Menu footer
            gameUI.printCentered(cy + 11, "(B) Back", Color::YELLOW);
            if (totalEntries > 0) {
                gameUI.printCentered(cy + 12, "(C) Clear History", Color::RED);
            }
            terminal.resetColor();
            
            // Status bar dengan SFX status
            gameUI.drawStatusBar("", 0, "", SFXManager::isEnabled());
            
            terminal.flush();
        }

        // Handle input
        if (terminal.hasInput()) {
            char c = terminal.getInput();
            
            if (c == 'b' || c == 'B') { 
                SFXManager::playTrue();
                currentState = GameState::MENU_MAIN; 
                return; 
            }
            
            // Toggle SFX dengan shortcut S
            if (c == 's' || c == 'S') {
                SFXManager::toggle();
                SFXManager::playFalse();
                lastW = 0; // Force redraw
            }
            
            // Pagination controls (hanya jika ada lebih dari 1 halaman)
            if (totalPages > 1) {
                if (c == '2') { // Next page
                    if (currentPage < totalPages) {
                        SFXManager::playTrue();
                        currentPage++;
                        lastW = 0; // Force redraw
                    } else {
                        // Sudah di halaman terakhir
                        SFXManager::playFalse();
                    }
                }
                if (c == '1') { // Previous page
                    if (currentPage > 1) {
                        SFXManager::playTrue();
                        currentPage--;
                        lastW = 0; // Force redraw
                    } else {
                        // Sudah di halaman pertama
                        SFXManager::playFalse();
                    }
                }
            }
            
            // Clear History (hanya jika ada history)
            if (totalEntries > 0 && (c == 'c' || c == 'C')) {
                if (showClearHistoryConfirmation()) {
                    SFXManager::playTrue();
                    // Refresh data setelah clear
                    totalPages = historyManager.getTotalPages(pageSize);
                    totalEntries = historyManager.getTotalEntries();
                    currentPage = 1;
                }
                lastW = 0; // Force redraw
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

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

            // Kotak menu
            int boxW = 60; 
            int boxH = 14; 
            gameUI.drawBox(cx - boxW / 2, cy - boxH / 2, boxW, boxH, Color::CYAN);

            gameUI.printCentered(cy - 5, "SELECT LANGUAGE", Color::CYAN);
            gameUI.printCentered(cy - 1, "[1] Indonesia (ID)");
            gameUI.printCentered(cy + 0, "[2] English (EN)");
            gameUI.printCentered(cy + 4, "(B) Back", Color::YELLOW);

            gameUI.drawStatusBar(currentLanguage, selectedDuration, currentMode, SFXManager::isEnabled());
            
            // Flush setelah rendering selesai
            terminal.flush();
        }

        // Handle input
        if (terminal.hasInput()) {
            char c = terminal.getInput();
            if (c == 'b' || c == 'B') { 
                SFXManager::playTrue();
                currentState = GameState::MENU_MAIN; // Kembali ke menu utama
                return; 
            }
            if (c == '1') { 
                SFXManager::playTrue();
                currentLanguage = "id"; 
                originalLanguage = "id"; // Simpan bahasa asli
                currentState = GameState::MENU_DURATION; 
                return; 
            }
            if (c == '2') { 
                SFXManager::playTrue();
                currentLanguage = "en"; 
                originalLanguage = "en"; // Simpan bahasa asli
                currentState = GameState::MENU_DURATION; 
                return; 
            }
            // Toggle SFX dengan shortcut S
            if (c == 's' || c == 'S') {
                SFXManager::toggle();
                SFXManager::playFalse();
                lastW = 0; // Force redraw
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
            gameUI.drawBox(cx - boxW / 2, cy - boxH / 2, boxW, boxH, Color::MAGENTA);

            gameUI.printCentered(cy - 5, "SELECT DURATION", Color::MAGENTA);
            gameUI.printCentered(cy - 2, "[1] 15 Seconds");
            gameUI.printCentered(cy - 1, "[2] 30 Seconds");
            gameUI.printCentered(cy, "[3] 60 Seconds");
            gameUI.printCentered(cy + 1, "[4] Custom");
            gameUI.printCentered(cy + 2, "[5] Tanpa Waktu");
            gameUI.printCentered(cy + 4, "(B) Back", Color::YELLOW);

            gameUI.drawStatusBar(currentLanguage, selectedDuration, currentMode, SFXManager::isEnabled());
            
            // Flush setelah rendering selesai
            terminal.flush();
        }

        if (terminal.hasInput()) {
            char c = terminal.getInput();
            if (c == 'b' || c == 'B') { 
                SFXManager::playTrue();
                currentState = GameState::MENU_LANGUAGE; 
                return; 
            }
            if (c == '1') { 
                SFXManager::playTrue();
                selectedDuration = 15; 
                currentState = GameState::MENU_MODE; 
                return; 
            }
            if (c == '2') { 
                SFXManager::playTrue();
                selectedDuration = 30; 
                currentState = GameState::MENU_MODE; 
                return; 
            }
            if (c == '3') { 
                SFXManager::playTrue();
                selectedDuration = 60; 
                currentState = GameState::MENU_MODE; 
                return; 
            }
            if (c == '5') { 
                SFXManager::playTrue();
                selectedDuration = -1; 
                currentState = GameState::MENU_MODE; 
                return; 
            }
            
            // Toggle SFX dengan shortcut S
            if (c == 's' || c == 'S') {
                SFXManager::toggle();
                SFXManager::playFalse();
                lastW = 0; // Force redraw
            }
            
            // Custom duration input
            if (c == '4') {
                SFXManager::playTrue();
                terminal.clear();
                int w = terminal.getWidth(); 
                int h = terminal.getHeight();
                int cy = h / 2; 
                int cx = w / 2;

                gameUI.drawBox(cx - 30, cy - 5, 60, 12, Color::MAGENTA);
                gameUI.printCentered(cy - 2, "Enter Duration (seconds):", Color::WHITE);
                gameUI.printCentered(cy + 3, "(ESC) Cancel", Color::YELLOW);
                terminal.flush(); // Flush before input
                
                std::string inp = gameUI.getStringInput(true);
                lastW = 0; // Force redraw

                if (inp.empty()) continue;
                
                try {
                    int val = std::stoi(inp);
                    if (val > 0) {
                        SFXManager::playTrue();
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
            gameUI.drawBox(cx - boxW / 2, cy - boxH / 2, boxW, boxH, Color::GREEN);

            gameUI.printCentered(cy - 4, "SELECT MODE", Color::GREEN);
            gameUI.printCentered(cy, "[1] Manual Mode");
            gameUI.printCentered(cy + 1, "[2] Campaign Mode");
            gameUI.printCentered(cy + 3, "(B) Back", Color::YELLOW);

            gameUI.drawStatusBar(currentLanguage, selectedDuration, currentMode, SFXManager::isEnabled());
            
            // Flush setelah rendering selesai
            terminal.flush();
        }

        if (terminal.hasInput()) {
            char c = terminal.getInput();
            if (c == 'b' || c == 'B') { 
                SFXManager::playTrue();
                currentState = GameState::MENU_DURATION; 
                return; 
            }
            if (c == '1') { 
                SFXManager::playTrue();
                currentMode = "manual"; 
                currentState = GameState::MENU_DIFFICULTY; 
                return; 
            }
            if (c == '2') { 
                SFXManager::playTrue();
                currentMode = "campaign"; 
                currentState = GameState::MENU_DIFFICULTY; 
                return; 
            }
            // Toggle SFX dengan shortcut S
            if (c == 's' || c == 'S') {
                SFXManager::toggle();
                SFXManager::playFalse();
                lastW = 0; // Force redraw
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
                gameUI.drawBox(cx - boxW / 2, cy - boxH / 2, boxW, boxH, Color::BLUE);
                gameUI.printCentered(cy - 3, "MANUAL SETUP", Color::BLUE);
                gameUI.printCentered(cy - 1, "Enter Target WPM:", Color::WHITE);
                gameUI.printCentered(cy + 3, "(ESC) Back | (ENTER) Confirm", Color::YELLOW);

                gameUI.drawStatusBar(currentLanguage, selectedDuration, currentMode, SFXManager::isEnabled());

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
                        SFXManager::playTrue();
                        currentState = GameState::MENU_MODE; 
                        return; 
                    }
                    try { 
                        targetWPM = std::stoi(inputBuf); 
                    } catch (...) { 
                        targetWPM = 40; 
                    }
                    SFXManager::playTrue();
                    currentDifficulty = Difficulty::MEDIUM;
                    currentState = GameState::PLAYING;
                    resetSession();
                    return;
                }
                // ESC - Kembali
                else if (c == 27) {
                    terminal.hideCursor();
                    SFXManager::playTrue();
                    currentState = GameState::MENU_MODE;
                    return;
                }
                // Toggle SFX dengan shortcut S (hanya huruf S besar/kecil, bukan angka)
                else if (c == 's' || c == 'S') {
                    SFXManager::toggle();
                    SFXManager::playFalse();
                    lastW = 0; // Force redraw
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
            int boxH = 24;
            gameUI.drawBox(cx - boxW / 2, cy - boxH / 2, boxW, boxH, Color::MAGENTA);

            gameUI.printCentered(cy - 9, "CAMPAIGN DIFFICULTY", Color::MAGENTA);

            // Status lock dan persyaratan
            bool easyUnlocked = progressManager.isUnlocked(currentLanguage, Difficulty::EASY);
            bool mediumUnlocked = progressManager.isUnlocked(currentLanguage, Difficulty::MEDIUM);
            bool hardUnlocked = progressManager.isUnlocked(currentLanguage, Difficulty::HARD);

            // Build text dengan completion indicator
            std::string easyText = "[1] Easy";
            if (!easyUnlocked) {
                easyText += " [LOCKED]";
            } else if (progressManager.isCompleted(currentLanguage, Difficulty::EASY)) {
                easyText += " [PASSED]";
            }

            std::string mediumText = "[2] Medium";
            if (!mediumUnlocked) {
                mediumText += " [LOCKED]";
            } else if (progressManager.isCompleted(currentLanguage, Difficulty::MEDIUM)) {
                mediumText += " [PASSED]";
            }

            std::string hardText = "[3] Hard";
            if (!hardUnlocked) {
                hardText += " [LOCKED]";
            } else if (progressManager.isCompleted(currentLanguage, Difficulty::HARD)) {
                hardText += " [PASSED]";
            }

            // Tentukan warna berdasarkan status unlock dan completion
            Color cE = !easyUnlocked ? Color::RED : 
                    (progressManager.isCompleted(currentLanguage, Difficulty::EASY) ? Color::GREEN : Color::WHITE);
            Color cM = !mediumUnlocked ? Color::RED : 
                    (progressManager.isCompleted(currentLanguage, Difficulty::MEDIUM) ? Color::GREEN : Color::WHITE);
            Color cH = !hardUnlocked ? Color::RED : 
                    (progressManager.isCompleted(currentLanguage, Difficulty::HARD) ? Color::GREEN : Color::WHITE);

            gameUI.printCentered(cy - 6, easyText, cE);
            gameUI.printCentered(cy - 5, mediumText, cM);
            gameUI.printCentered(cy - 4, hardText, cH);
            
            std::string progText = "[4] Programmer Mode";
            Color progColor = Color::CYAN;

            // Check jika Programmer sudah completed di bahasa saat ini
            // Langsung cek di currentLanguage
            bool progCompleted = progressManager.isCompleted(currentLanguage, Difficulty::PROGRAMMER);

            if (progCompleted) {
                progText += " [CERTIFIED]";
                progColor = Color::GREEN;
            }

            gameUI.printCentered(cy - 2, progText, progColor);

            // Tampilkan banner berdasarkan progress
            bool mainLevelsCompleted = progressManager.isCompleted(currentLanguage, Difficulty::EASY) &&
                           progressManager.isCompleted(currentLanguage, Difficulty::MEDIUM) &&
                           progressManager.isCompleted(currentLanguage, Difficulty::HARD);

            // Cek Programmer Certification
            bool programmerCertified = false;
            if (currentLanguage == "prog") {
                programmerCertified = progressManager.isCompleted("prog", Difficulty::PROGRAMMER);
            } else if (currentLanguage == "id") {
                programmerCertified = progressManager.isCompleted("id", Difficulty::PROGRAMMER);
            } else if (currentLanguage == "en") {
                programmerCertified = progressManager.isCompleted("en", Difficulty::PROGRAMMER);
            }

            // Show banner sesuai status
            // Banner logic: Priority untuk Programmer Certification
            if (programmerCertified) {
                gameUI.printCentered(cy + 1, "================================", Color::CYAN);
                gameUI.printCentered(cy + 2, "PROGRAMMER CERTIFIED!", Color::CYAN);
                gameUI.printCentered(cy + 3, "You are now a certified programmer!", Color::WHITE);
                gameUI.printCentered(cy + 4, "Master of syntax and speed!", Color::GREEN);
                gameUI.printCentered(cy + 5, "================================", Color::CYAN);
            }
            else if (mainLevelsCompleted) {
                gameUI.printCentered(cy + 1, "================================", Color::GREEN);
                gameUI.printCentered(cy + 2, "CONGRATULATIONS!", Color::GREEN);
                gameUI.printCentered(cy + 3, "You have completed all levels!", Color::WHITE);
                gameUI.printCentered(cy + 4, "================================", Color::GREEN);
                gameUI.printCentered(cy + 5, "Try Programmer Mode to get certified!", Color::CYAN);
            } 
            else {
                gameUI.printCentered(cy + 1, "Requirements:", Color::YELLOW);
                gameUI.printCentered(cy + 2, "Easy -> Medium: 40 WPM, 80% Accuracy");
                gameUI.printCentered(cy + 3, "Medium -> Hard: 60 WPM, 90% Accuracy");
                gameUI.printCentered(cy + 4, "Hard Complete: 70 WPM, 90% Accuracy");
                gameUI.printCentered(cy + 5, "Programmer Cert: 50 WPM, 90% Accuracy", Color::CYAN);
            }

            
            gameUI.printCentered(cy + 8, "(B) Back | (C) Credits", Color::YELLOW);
            // Reset progress hanya muncul jika Easy sudah selesai
            if (progressManager.isCompleted(currentLanguage, Difficulty::EASY)){
                gameUI.printCentered(cy + 9, "(R) Reset Progress", Color::RED);
            }

            gameUI.drawStatusBar(currentLanguage, selectedDuration, currentMode, SFXManager::isEnabled());
            
            // Flush setelah rendering selesai
            terminal.flush();
        }

        if (terminal.hasInput()) {
            char d = terminal.getInput();
            
            // Toggle SFX dengan shortcut S
            if (d == 's' || d == 'S') {
                SFXManager::toggle();
                SFXManager::playFalse();
                lastW = 0; // Force redraw
                continue;
            }
            
            // Restore bahasa saat user kembali dari menu
            if (d == 'b' || d == 'B') {
                SFXManager::playTrue();
                restoreLanguageFromProgrammerMode();
                currentState = GameState::MENU_MODE; 
                return; 
            }
            
            // Shortcut ke Credits
            if (d == 'c' || d == 'C') {
                SFXManager::playTrue();
                previousState = GameState::MENU_DIFFICULTY;
                currentState = GameState::CREDITS;
                return;
            }

            // Hanya bisa Reset Progress (jika sudah selesai Easy)
            if (progressManager.isCompleted(currentLanguage, Difficulty::EASY)) {
                if (d == 'r' || d == 'R') {
                    showResetConfirmation();
                    lastW = 0; // Force redraw SELALU, baik Y atau N
                    continue;
                }
            }
            

            bool valid = false;
            if (d == '1' && progressManager.isUnlocked(currentLanguage, Difficulty::EASY)) { 
                currentDifficulty = Difficulty::EASY; 
                valid = true; 
            }
            if (d == '2' && progressManager.isUnlocked(currentLanguage, Difficulty::MEDIUM)) { 
                currentDifficulty = Difficulty::MEDIUM; 
                valid = true; 
            }
            if (d == '3' && progressManager.isUnlocked(currentLanguage, Difficulty::HARD)) { 
                currentDifficulty = Difficulty::HARD; 
                valid = true; 
            }
            if (d == '4') { 
                currentDifficulty = Difficulty::PROGRAMMER; 
                currentLanguage = "prog"; 
                valid = true; 
            }

            if (valid) {
                SFXManager::playTrue();
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

    // Header info dengan SFX status
    std::string sfxStatus = SFXManager::isEnabled() ? "On" : "Off";
    std::string info = " MODE: " + currentMode + " | SFX: " + sfxStatus + " | TIME: " + timeStr;
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
        gameUI.printCentered(cy + 6, "Type to start...", Color::WHITE);
    } else {
         gameUI.printCentered(cy + 6, "                 ", Color::WHITE);
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
                SFXManager::playFalse(); // Play error sound
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

    // Save history entry
    HistoryEntry entry;
    entry.wpm = currentStats.wpm;
    entry.accuracy = currentStats.accuracy;
    entry.errors = currentStats.errors;
    
    // Format difficulty
    switch (currentDifficulty) {
        case Difficulty::EASY: entry.difficulty = "Easy"; break;
        case Difficulty::MEDIUM: entry.difficulty = "Medium"; break;
        case Difficulty::HARD: entry.difficulty = "Hard"; break;
        case Difficulty::PROGRAMMER: entry.difficulty = "Programmer"; break;
    }
    
    // Format language
    if (currentLanguage == "id") entry.language = "ID";
    else if (currentLanguage == "en") entry.language = "EN";
    else if (currentLanguage == "prog") entry.language = "PROG";
    else entry.language = currentLanguage;
    
    // Format mode
    entry.mode = (currentMode == "manual") ? "Manual" : "Campaign";
    
    // Timestamp akan otomatis diset oleh HistoryManager
    entry.timestamp = ""; // akan di-set di saveEntry()
    
    // Save ke history
    historyManager.saveEntry(entry);
    
    // Cek apakah ini pertama kali menyelesaikan Hard mode
    if (currentMode == "campaign" && currentDifficulty == Difficulty::HARD) {
        if (currentStats.wpm >= 70 && currentStats.accuracy >= 90) {
            // Check apakah ini PERTAMA KALI complete hard di bahasa ini
            if (!progressManager.wasHardCompletedBefore(currentLanguage)) {
                shouldShowRickRoll = true;
                hardJustCompleted = true;
                progressManager.markHardCompleted(currentLanguage);
            }
            // Mark sebagai completed (bisa berkali-kali)
            progressManager.setCompleted(currentLanguage, Difficulty::HARD, true);
            progressManager.saveProgress(); // SAVE setelah update
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
        gameUI.drawBox(cx - boxW / 2, cy - boxH / 2, boxW, boxH, Color::CYAN);
        
        gameUI.printCentered(cy - 5, "RESULTS", Color::CYAN);
        gameUI.printCentered(cy - 2, "WPM: " + std::to_string((int)currentStats.wpm), Color::GREEN);
        gameUI.printCentered(cy - 1, "Accuracy: " + std::to_string((int)currentStats.accuracy) + "%", Color::WHITE);
        gameUI.printCentered(cy + 1, "HARD MODE COMPLETED!!!", Color::GREEN);
        gameUI.printCentered(cy + 2, "CONGRATULATIONS!!!", Color::GREEN);
        gameUI.printCentered(cy + 4, "Preparing special surprise...", Color::YELLOW);
        
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
            gameUI.drawBox(cx - boxW / 2, cy - boxH / 2, boxW, boxH, Color::CYAN);

            gameUI.printCentered(cy - 8, "RESULTS", Color::CYAN);

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
                        progressManager.setUnlocked(currentLanguage, Difficulty::MEDIUM, true);
                        progressManager.setCompleted(currentLanguage, Difficulty::EASY, true);
                        progressManager.saveProgress(); // SAVE setelah update
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
                        progressManager.setUnlocked(currentLanguage, Difficulty::HARD, true);
                        progressManager.setCompleted(currentLanguage, Difficulty::MEDIUM, true);
                        progressManager.saveProgress(); // SAVE setelah update
                    } else {
                        msg = "LEVEL FAILED";
                        msgColor = Color::RED;
                    }
                }
                else if (currentDifficulty == Difficulty::HARD) {
                    requirement = "Need: 70 WPM, 90% Accuracy";
                    if (currentStats.wpm >= 70 && currentStats.accuracy >= 90) {
                        // Jika sudah pernah complete sebelumnya
                        if (progressManager.wasHardCompletedBefore(currentLanguage)) {
                            msg = "HARD MODE PASSED!";
                            msgColor = Color::GREEN;
                        }
                        progressManager.setCompleted(currentLanguage, Difficulty::HARD, true);
                        progressManager.saveProgress(); // SAVE
                    } else {
                        msg = "LEVEL FAILED";
                        msgColor = Color::RED;
                    }
                }
                // Programmer Mode
                else if (currentDifficulty == Difficulty::PROGRAMMER) {
                    requirement = "Need: 50 WPM, 90% Accuracy";
                    
                    if (currentStats.wpm >= 50 && currentStats.accuracy >= 90) {
                        // Langsung gunakan originalLanguage untuk menentukan bahasa mana yang harus disave
                        // originalLanguage berisi "id" atau "en" yang dipilih user di awal
                        std::string langToSave = originalLanguage;
                        
                        // Cek apakah sudah pernah certified sebelumnya
                        bool alreadyCertified = progressManager.isCompleted(langToSave, Difficulty::PROGRAMMER);
                        
                        if (alreadyCertified) {
                            msg = "PROGRAMMER CERTIFIED!";
                            msgColor = Color::CYAN;
                        } else {
                            msg = "YOU ARE NOW A CERTIFIED PROGRAMMER!";
                            msgColor = Color::CYAN;
                        }

                        // Simpan status sertifikasi KE BAHASA ASLI (id/en), BUKAN ke "prog"
                        pass = true;
                        progressManager.setCompleted(langToSave, Difficulty::PROGRAMMER, true);
                        progressManager.saveProgress();
                    } else {
                        msg = "CERTIFICATION FAILED";
                        msgColor = Color::RED;
                    }
                }
                
                // Tampilkan pesan dan requirement
                if (!msg.empty()) {
                    gameUI.printCentered(cy + 1, msg, msgColor);
                }
                if (!requirement.empty() && !pass) {
                    gameUI.printCentered(cy + 2, requirement, Color::YELLOW);
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
                gameUI.printCentered(cy + 2, msg, msgColor);
            }

            // SFX status text dengan On/Off indicator
            std::string sfxText = std::string("(C) Credits | (S) SFX: ") + (SFXManager::isEnabled() ? "On" : "Off");
            gameUI.printCentered(cy + 5, sfxText, Color::YELLOW);
            gameUI.printCentered(cy + 6, "Press ENTER to continue", Color::WHITE);
            
            // Flush setelah rendering selesai
            terminal.flush();
        }

        // Wait for ENTER or C or S
        if (terminal.hasInput()) {
            char c = terminal.getInput();
            if (c == 10 || c == 13) {
                SFXManager::playTrue();
                break;
            }
            if (c == 'c' || c == 'C') {
                SFXManager::playTrue();
                // CRITICAL: Set previousState ke MENU_DIFFICULTY, bukan RESULTS
                // Mencegah infinite loop: results -> credits -> results
                // Sekarang flow: results -> credits -> menu difficulty
                previousState = GameState::MENU_DIFFICULTY;
                currentState = GameState::CREDITS;
                return;
            }
            // Toggle SFX
            if (c == 's' || c == 'S') {
                SFXManager::toggle();
                SFXManager::playFalse();
                lastW = 0; // Force redraw
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    // Restore bahasa asli setelah selesai Programmer mode
    restoreLanguageFromProgrammerMode();
    
    // CRITICAL: Reset session sebelum kembali ke menu
    // Membersihkan semua state (isGameStarted, stats, buffers, dll)
    // untuk mencegah corrupt data di game berikutnya
    resetSession();
    
    // Clear input buffer untuk safety
    while (terminal.hasInput()) {
        terminal.getInput();
    }

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
            gameUI.drawBox((w - boxW) / 2, cy - boxH / 2, boxW, boxH, Color::MAGENTA);

            gameUI.printCentered(cy - 7, "CREDITS", Color::MAGENTA);
            gameUI.printCentered(cy - 5, "Developed by:", Color::CYAN);
            
            // Nama-nama developer
            gameUI.printCentered(cy - 3, "Alea Farrel", Color::WHITE);
            gameUI.printCentered(cy - 2, "Hensa Katelu", Color::WHITE);
            gameUI.printCentered(cy - 1, "Yanuar Adi Candra", Color::WHITE);
            gameUI.printCentered(cy, "Arif Wibowo P.", Color::WHITE);
            gameUI.printCentered(cy + 1, "Aria Mahendra U.", Color::WHITE);
            
            gameUI.printCentered(cy + 4, "Thank you for playing!", Color::GREEN);
            gameUI.printCentered(cy + 6, "Press ENTER to return", Color::YELLOW);
            
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


// ============================================================================
// RESET PROGRESS CONFIRMATION
// ============================================================================

/// @brief Menampilkan konfirmasi reset progress
/// @return true jika user mengonfirmasi reset, false jika dibatalkan
bool GameEngine::showResetConfirmation() {
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
            
            int boxW = 60;
            int boxH = 16;
            gameUI.drawBox(cx - boxW / 2, cy - boxH / 2, boxW, boxH, Color::RED);
            
            gameUI.printCentered(cy - 6, "!!! WARNING !!!", Color::RED);
            gameUI.printCentered(cy - 4, "Reset Progress", Color::YELLOW);
            gameUI.printCentered(cy - 2, "This will DELETE all campaign progress");
            gameUI.printCentered(cy - 1, "in ALL languages and difficulties");
            gameUI.printCentered(cy + 1, "This action CANNOT be undone!", Color::RED);
            gameUI.printCentered(cy + 4, "[Y] Yes, Reset Everything", Color::RED);
            gameUI.printCentered(cy + 5, "[N] No, Cancel", Color::GREEN);
            
            gameUI.drawStatusBar(currentLanguage, selectedDuration, currentMode);
            terminal.flush();
        }
        
        if (terminal.hasInput()) {
            char c = terminal.getInput();
            
            if (c == 'y' || c == 'Y') {
                terminal.clear();
                gameUI.printCentered(terminal.getHeight() / 2, "Resetting progress...", Color::YELLOW);
                terminal.flush();
                
                progressManager.resetProgress();
                
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                
                terminal.clear();
                gameUI.printCentered(terminal.getHeight() / 2, "Progress reset successfully!", Color::GREEN);
                terminal.flush();
                
                std::this_thread::sleep_for(std::chrono::seconds(1));
                return true;
            }
            
            if (c == 'n' || c == 'N') {
                terminal.clear();
                terminal.flush();
                return false;
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

// ============================================================================
// CLEAR HISTORY CONFIRMATION
// ============================================================================

/// @brief Menampilkan konfirmasi clear history
/// @return true jika user mengonfirmasi clear, false jika dibatalkan
bool GameEngine::showClearHistoryConfirmation() {
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
            
            int boxW = 60;
            int boxH = 16;
            gameUI.drawBox(cx - boxW / 2, cy - boxH / 2, boxW, boxH, Color::YELLOW);
            
            gameUI.printCentered(cy - 6, "!!! WARNING !!!", Color::YELLOW);
            gameUI.printCentered(cy - 4, "Clear History", Color::RED);
            gameUI.printCentered(cy - 2, "This will DELETE all game history");
            gameUI.printCentered(cy - 1, "including WPM, accuracy, and timestamps");
            gameUI.printCentered(cy + 1, "This action CANNOT be undone!", Color::RED);
            gameUI.printCentered(cy + 4, "[Y] Yes, Clear All History", Color::RED);
            gameUI.printCentered(cy + 5, "[N] No, Cancel", Color::GREEN);
            
            terminal.flush();
        }
        
        if (terminal.hasInput()) {
            char c = terminal.getInput();
            
            if (c == 'y' || c == 'Y') {
                terminal.clear();
                gameUI.printCentered(terminal.getHeight() / 2, "Clearing history...", Color::YELLOW);
                terminal.flush();
                
                historyManager.clearHistory();
                
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                
                terminal.clear();
                gameUI.printCentered(terminal.getHeight() / 2, "History cleared successfully!", Color::GREEN);
                terminal.flush();
                
                std::this_thread::sleep_for(std::chrono::seconds(1));
                return true;
            }
            
            if (c == 'n' || c == 'N') {
                terminal.clear();
                terminal.flush();
                return false;
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}