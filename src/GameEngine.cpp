#include "GameEngine.h"
#include <iostream>
#include <sstream>
#include <thread>      // Untuk sleep lintas platform
#include <chrono>      // Untuk perhitungan waktu lintas platform
#include <iomanip>
#include <cmath>

// Constructor: Menginisialisasi state awal dan memuat data
GameEngine::GameEngine() : currentState(GameState::MENU_LANGUAGE) {
    // Menyiapkan terminal (raw mode, dll)
    terminal.initialize();
    
    // Mengatur status unlock awal untuk mode Campaign
    unlockedDifficulties[Difficulty::EASY] = true;      // Terbuka dari awal
    unlockedDifficulties[Difficulty::MEDIUM] = false;   // Terkunci
    unlockedDifficulties[Difficulty::HARD] = false;     // Terkunci
    unlockedDifficulties[Difficulty::PROGRAMMER] = true; // Mode bonus (selalu terbuka)
    
    // Memuat kata-kata dari file eksternal ke dalam memori
    textProvider.loadWords("id", "id.txt");
    textProvider.loadWords("en", "en.txt");
    textProvider.loadWords("prog", "prog.txt");
    
    selectedDuration = 30; // Durasi default 30 detik
}

// Loop utama aplikasi (State Machine)
void GameEngine::run() {
    while (currentState != GameState::EXIT) {
        // Switch case ini menangani perpindahan antar "Layar" atau "State"
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
                gameLoop(); // Logika permainan utama ada di sini
                break;
            case GameState::RESULTS:
                showResults(); // Menampilkan skor akhir
                break;
            case GameState::EXIT:
                break;
        }
    }
    // Membersihkan setting terminal sebelum keluar (mengembalikan cursor, echo, dll)
    terminal.cleanup();
}

// === UI HELPERS (Fungsi Bantuan untuk Tampilan) ===

// Menggambar kotak/border ASCII di koordinat tertentu
void GameEngine::drawBox(int x, int y, int w, int h, Color color) {
    terminal.setColor(color);
    
    // Gambar garis atas
    terminal.setCursor(x, y);
    terminal.print("+");
    for(int i=0; i<w-2; ++i) terminal.print("-");
    terminal.print("+");
    
    // Gambar sisi kiri dan kanan
    for(int i=1; i<h-1; ++i) {
        terminal.setCursor(x, y+i);
        terminal.print("|");
        terminal.setCursor(x+w-1, y+i);
        terminal.print("|");
    }
    
    // Gambar garis bawah
    terminal.setCursor(x, y+h-1);
    terminal.print("+");
    for(int i=0; i<w-2; ++i) terminal.print("-");
    terminal.print("+");
    terminal.resetColor();
}

// Mencetak teks tepat di tengah layar secara horizontal pada baris y
void GameEngine::printCentered(int y, std::string text, Color color) {
    int cx = terminal.getWidth() / 2;
    int x = cx - (text.length() / 2); // Hitung posisi x agar teks di tengah
    terminal.setCursor(x, y);
    if (color != Color::DEFAULT) terminal.setColor(color);
    terminal.print(text);
    terminal.resetColor();
}

// Menggambar bar status di bagian bawah layar
void GameEngine::drawStatusBar() {
    int w = terminal.getWidth();
    int h = terminal.getHeight();
    int y = h - 2;
    
    // Menyiapkan string info status
    std::string lang = currentLanguage.empty() ? "N/A" : (currentLanguage == "id" ? "ID" : (currentLanguage == "en" ? "EN" : "PROG"));
    std::string time;
    if (selectedDuration == -1) time = "Inf";
    else if (selectedDuration == 0) time = "30s";
    else time = std::to_string(selectedDuration) + "s";

    std::string mode = currentMode.empty() ? "N/A" : (currentMode == "manual" ? "Manual" : "Campaign");
    if (currentLanguage == "prog") mode = "Programmer";
    
    std::string status = " Lang: " + lang + " | Time: " + time + " | Mode: " + mode + " ";
    
    // Menggambar background biru penuh satu baris
    terminal.setBackgroundColor(Color::BLUE);
    for(int i=0; i<w; ++i) {
        terminal.setCursor(i, y);
        terminal.print(" ");
    }
    // Menulis teks status di atas background biru
    printCentered(y, status, Color::WHITE);
    terminal.resetColor();
}

// Fungsi untuk mendapatkan input string dari user (misal: input durasi custom)
std::string GameEngine::getStringInput(bool digitsOnly) {
    std::string inputBuf = "";
    int startX = terminal.getWidth() / 2 - 10;
    int startY = terminal.getHeight() / 2 + 1;
    
    terminal.setCursor(startX + inputBuf.length(), startY);
    terminal.showCursor(); // Cursor perlu terlihat saat mengetik input

    while(true) {
        if (terminal.hasInput()) {
            char c = terminal.getInput();
            if (c == 27) { terminal.hideCursor(); return ""; } // ESC untuk batal
            if (c == 10 || c == 13) { terminal.hideCursor(); return inputBuf; } // ENTER untuk konfirmasi
            
            // Handle Backspace
            if (c == 127 || c == '\b' || c == 8) { 
                if (!inputBuf.empty()) {
                    inputBuf.pop_back();
                    // Hapus karakter visual di layar
                    terminal.setCursor(startX + inputBuf.length(), startY);
                    terminal.print(" ");
                    terminal.setCursor(startX + inputBuf.length(), startY);
                }
            } else if (!digitsOnly || (c >= '0' && c <= '9')) {
                // Filter karakter yang bisa diketik
                if (c >= 32 && c <= 126 && inputBuf.length() < 20) {
                     inputBuf += c;
                     terminal.print(std::string(1, c));
                }
            }
        }
    }
}

// === HANDLERS (Logika Menu) ===

void GameEngine::handleMenuLanguage() {
    terminal.clear();
    int h = terminal.getHeight();
    int w = terminal.getWidth();
    int cy = h / 2;
    int cx = w / 2;

    // ASCII Art Judul
    std::string title1 = " ____  _  _  ____  ____  ____ ";
    std::string title2 = "(  _ \\/ )( \\(  _ \\(_  _)(    \\";
    std::string title3 = " )   /) __ ( ) __/ _)(_  ) D (";
    std::string title4 = "(__\\_)\\_)(_/(__)  (____)(____/";
    std::string subtitle = "RAPID TEXTER";

    // Gambar Kotak Menu
    int boxW = 50;
    int boxH = 14;
    drawBox(cx - boxW/2, cy - boxH/2, boxW, boxH, Color::CYAN);

    // Tampilkan Judul & Menu
    printCentered(cy - 5, title1, Color::CYAN);
    printCentered(cy - 4, title2, Color::CYAN);
    printCentered(cy - 3, title3, Color::CYAN);
    printCentered(cy - 2, title4, Color::CYAN);
    printCentered(cy, subtitle, Color::BLUE);

    printCentered(cy + 2, "[1] Indonesia (ID)");
    printCentered(cy + 3, "[2] English (EN)");
    printCentered(cy + 5, "(Q) Quit", Color::RED);
    
    drawStatusBar();

    // Loop input menu
    while(true) {
        if (terminal.hasInput()) {
            char c = terminal.getInput();
            if (c == 'q' || c == 'Q') { currentState = GameState::EXIT; return; }
            if (c == '1') { currentLanguage = "id"; currentState = GameState::MENU_DURATION; return; }
            if (c == '2') { currentLanguage = "en"; currentState = GameState::MENU_DURATION; return; }
        }
    }
}

void GameEngine::handleMenuDuration() {
    terminal.clear();
    int h = terminal.getHeight();
    int w = terminal.getWidth();
    int cy = h / 2;
    int cx = w / 2;

    int boxW = 50;
    int boxH = 14;
    drawBox(cx - boxW/2, cy - boxH/2, boxW, boxH, Color::MAGENTA);

    printCentered(cy - 5, "SELECT DURATION", Color::MAGENTA);
    printCentered(cy - 2, "[1] 15 Seconds");
    printCentered(cy - 1, "[2] 30 Seconds");
    printCentered(cy, "[3] 60 Seconds");
    printCentered(cy + 1, "[4] Custom");
    printCentered(cy + 2, "[5] Tanpa Waktu");
    printCentered(cy + 4, "(B) Back", Color::YELLOW);

    drawStatusBar();

    while(true) {
        if (terminal.hasInput()) {
            char c = terminal.getInput();
            if (c == 'b' || c == 'B') { currentState = GameState::MENU_LANGUAGE; return; }
            if (c == '1') { selectedDuration = 15; currentState = GameState::MENU_MODE; return; }
            if (c == '2') { selectedDuration = 30; currentState = GameState::MENU_MODE; return; }
            if (c == '3') { selectedDuration = 60; currentState = GameState::MENU_MODE; return; }
            if (c == '4') {
                // Handle Custom Duration Input
                terminal.clear();
                drawBox(cx - 30, cy - 5, 60, 10, Color::MAGENTA);
                printCentered(cy - 2, "Enter Duration (seconds):", Color::WHITE);
                std::string inp = getStringInput(true);
                if (inp.empty()) { handleMenuDuration(); return; }
                try {
                    int val = std::stoi(inp);
                    if (val > 0) {
                        selectedDuration = val;
                        currentState = GameState::MENU_MODE;
                        return;
                    }
                } catch (...) {}
                handleMenuDuration();
                return;
            }
            if (c == '5') { selectedDuration = -1; currentState = GameState::MENU_MODE; return; }
        }
    }
}

void GameEngine::handleMenuMode() {
    terminal.clear();
    int h = terminal.getHeight();
    int w = terminal.getWidth();
    int cy = h / 2;
    int cx = w / 2;

    int boxW = 50;
    int boxH = 12;
    drawBox(cx - boxW/2, cy - boxH/2, boxW, boxH, Color::GREEN);

    printCentered(cy - 4, "SELECT MODE", Color::GREEN);
    printCentered(cy, "[1] Manual Mode");
    printCentered(cy + 1, "[2] Campaign Mode");
    printCentered(cy + 3, "(B) Back", Color::YELLOW);
    
    drawStatusBar();

    while(true) {
        if (terminal.hasInput()) {
            char c = terminal.getInput();
            if (c == 'b' || c == 'B') { currentState = GameState::MENU_DURATION; return; }
            if (c == '1') { currentMode = "manual"; currentState = GameState::MENU_DIFFICULTY; return; }
            if (c == '2') { currentMode = "campaign"; currentState = GameState::MENU_DIFFICULTY; return; }
        }
    }
}

void GameEngine::handleMenuDifficulty() {
    terminal.clear();
    int h = terminal.getHeight();
    int w = terminal.getWidth();
    int cy = h / 2;
    int cx = w / 2;
    
    // Tampilan berbeda tergantung Mode yang dipilih
    if (currentMode == "manual") {
        // --- MANUAL MODE ---
        int boxW = 50;
        int boxH = 10;
        drawBox(cx - boxW/2, cy - boxH/2, boxW, boxH, Color::BLUE);

        printCentered(cy - 3, "MANUAL SETUP", Color::BLUE);
        printCentered(cy - 1, "Enter Target WPM:", Color::WHITE);
        printCentered(cy + 3, "(ESC) Back | (ENTER) Confirm", Color::YELLOW);
        
        drawStatusBar();

        std::string inputBuf = getStringInput(true);
        
        if (inputBuf.empty()) {
             currentState = GameState::MENU_MODE;
             return;
        }

        try {
            targetWPM = std::stoi(inputBuf);
        } catch (...) {
            targetWPM = 40; // Default fallback
        }
        currentDifficulty = Difficulty::MEDIUM; // Default difficulty untuk manual
        currentState = GameState::PLAYING;
        resetSession(); // Siapkan game baru
        return; 
    } else {
        // --- CAMPAIGN MODE ---
        int boxW = 50;
        int boxH = 16;
        drawBox(cx - boxW/2, cy - boxH/2, boxW, boxH, Color::MAGENTA);
        
        printCentered(cy - 6, "CAMPAIGN DIFFICULTY", Color::MAGENTA);
        
        // Cek status unlock
        std::string e = unlockedDifficulties[Difficulty::EASY] ? " " : " [LOCKED]";
        std::string m = unlockedDifficulties[Difficulty::MEDIUM] ? " " : " [LOCKED]";
        std::string h = unlockedDifficulties[Difficulty::HARD] ? " " : " [LOCKED]";
        
        // Ubah warna jika terkunci
        Color cE = unlockedDifficulties[Difficulty::EASY] ? Color::WHITE : Color::RED;
        Color cM = unlockedDifficulties[Difficulty::MEDIUM] ? Color::WHITE : Color::RED;
        Color cH = unlockedDifficulties[Difficulty::HARD] ? Color::WHITE : Color::RED;
        
        printCentered(cy - 3, "[1] Easy" + e, cE);
        printCentered(cy - 2, "[2] Medium" + m, cM);
        printCentered(cy - 1, "[3] Hard" + h, cH);
        printCentered(cy + 1, "[4] Programmer Mode");
        
        printCentered(cy + 4, "(B) Back", Color::YELLOW);

        drawStatusBar();

        while(true) {
             if (terminal.hasInput()) {
                char d = terminal.getInput();
                if (d == 'b' || d == 'B') { currentState = GameState::MENU_MODE; return; }
                
                // Validasi pilihan berdasarkan status unlock
                bool valid = false;
                if (d == '1' && unlockedDifficulties[Difficulty::EASY]) { currentDifficulty = Difficulty::EASY; valid = true; }
                if (d == '2' && unlockedDifficulties[Difficulty::MEDIUM]) { currentDifficulty = Difficulty::MEDIUM; valid = true; }
                if (d == '3' && unlockedDifficulties[Difficulty::HARD]) { currentDifficulty = Difficulty::HARD; valid = true; }
                if (d == '4') { currentDifficulty = Difficulty::PROGRAMMER; currentLanguage = "prog"; valid = true; } 
                
                if (valid) {
                    // Jika programmer mode, bahasa otomatis ganti ke 'prog'
                    if (currentDifficulty == Difficulty::PROGRAMMER) currentLanguage = "prog";
                    currentState = GameState::PLAYING;
                    resetSession();
                    return;
                }
             }
        }
    }
}

// Reset data sesi permainan sebelum mulai mengetik
void GameEngine::resetSession() {
    // Ambil kata-kata acak dari TextProvider
    targetWords = textProvider.getWords(currentLanguage, currentDifficulty, 30); 
    
    // Gabungkan vector kata menjadi satu string panjang dengan spasi
    flatTargetString = "";
    for (size_t i = 0; i < targetWords.size(); ++i) {
        flatTargetString += targetWords[i];
        if (i < targetWords.size() - 1) flatTargetString += " ";
    }
    
    typedString = "";
    cursorPosition = 0;
    currentStats.reset();
    timeLimitSeconds = selectedDuration; 
    isGameStarted = false; // Waktu belum berjalan sampai user mengetik
    terminal.clear(); 
}

// Render tampilan saat permainan berlangsung (Kata-kata, warna, kursor)
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

    // Header Info
    std::string info = " MODE: " + currentMode + " | TIME: " + timeStr + "   ";
    terminal.setCursor(cx - info.length()/2, 2);
    terminal.resetColor();
    terminal.print(info);

    // Footer Info
    std::string footer = "TAB: Restart | ESC: Exit";
    terminal.setCursor(cx - (footer.length()/2), h - 2);
    terminal.setColor(Color::YELLOW);
    terminal.print(footer);
    terminal.resetColor();

    // Area pengetikan
    int boxWidth = 60;
    if (w < 64) boxWidth = w - 4; // Responsif jika layar kecil
    int startX = (w - boxWidth) / 2;
    int startY = cy - 5; 

    int curX = startX;
    int curY = startY;
    
    // Loop untuk menggambar setiap karakter
    for (size_t i = 0; i < flatTargetString.length(); ++i) {
        // Wrap text jika melebihi lebar box
        if (curX >= startX + boxWidth) {
            curX = startX;
            curY += 2; // Pindah baris (spasi antar baris = 1)
        }

        char targetChar = flatTargetString[i];
        char charToDraw = targetChar;
        Color color = Color::WHITE; 

        // Tentukan warna berdasarkan input user
        if (i < typedString.length()) {
            char typedChar = typedString[i];
            if (typedChar == targetChar) {
                color = Color::GREEN; // Benar
            } else {
                color = Color::RED;   // Salah
                // Jika salah ketik spasi, tampilkan underscore merah
                if (targetChar == ' ') {
                    charToDraw = typedChar;
                    if (charToDraw == ' ') charToDraw = '_';
                }
            }
        }

        terminal.setColor(color);
        terminal.printAt(curX, curY, std::string(1, charToDraw));

        // Gambar indikator posisi kursor (tanda ^ di bawah huruf)
        terminal.setCursor(curX, curY + 1);
        if ((int)i == cursorPosition) {
            terminal.setColor(Color::YELLOW);
            terminal.print("^");
        } else {
            terminal.print(" "); // Hapus bekas kursor lama
        }

        curX++;
    }
    terminal.resetColor();
    
    // Instruksi mulai
    if (!isGameStarted) {
        printCentered(cy + 6, "Type to start...", Color::WHITE);
    } else {
         printCentered(cy + 6, "                 ", Color::WHITE); // Bersihkan teks
    }
    
    terminal.hideCursor(); 
}

// Loop utama permainan (mengetik)
void GameEngine::gameLoop() {
    terminal.clear(); 
    
    while (currentState == GameState::PLAYING) {
        
        // Logika Timer
        if (isGameStarted) {
            auto now = std::chrono::steady_clock::now();
            int elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();
            
            if (selectedDuration != -1) {
                int remaining = selectedDuration - elapsed;
                timeLimitSeconds = remaining;

                // Waktu habis -> Ke layar hasil
                if (remaining <= 0) {
                    currentState = GameState::RESULTS;
                    return;
                }
            } else {
                timeLimitSeconds = elapsed; 
            }
        }

        renderGame();
        
        // Cek input non-blocking
        if (terminal.hasInput()) {
            char c = terminal.getInput();
            processInput(c);
        } else {
            // Sleep sedikit untuk mengurangi penggunaan CPU
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
}

// Memproses setiap tombol yang ditekan saat bermain
void GameEngine::processInput(char c) {
    if (c == 27) { // ESC -> Kembali ke menu
        currentState = GameState::MENU_DIFFICULTY; 
        return;
    }
    if (c == 9) { // TAB -> Restart sesi
        resetSession();
        return;
    }
    
    // Handle Backspace
    if (c == 127 || c == '\b' || c == 8) { 
        if (cursorPosition > 0) {
            cursorPosition--;
            typedString.pop_back();
        }
    } else {
        // Karakter pertama ditekan -> Mulai timer
        if (!isGameStarted) {
            isGameStarted = true;
            startTime = std::chrono::steady_clock::now();
        }

        // Proses karakter
        if (cursorPosition < (int)flatTargetString.length()) {
            char targetChar = flatTargetString[cursorPosition];
            
            typedString += c;
            
            // Cek kebenaran ketikan
            if (targetChar == c) {
                currentStats.correctKeystrokes++;
            } else {
                currentStats.errors++;
                terminal.beep(); // Bunyi jika salah
            }
            currentStats.totalKeystrokes++;
            cursorPosition++;
            
            // Jika sudah sampai akhir teks -> Selesai
            if (cursorPosition >= (int)flatTargetString.length()) {
                currentState = GameState::RESULTS;
            }
        }
    }
}

// Menampilkan layar hasil statistik
void GameEngine::showResults() {
    double seconds = 0;
    if (isGameStarted) {
        auto now = std::chrono::steady_clock::now();
        seconds = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count() / 1000.0;
        // Cap waktu max sesuai durasi agar WPM tidak aneh
        if (selectedDuration != -1 && seconds > selectedDuration) seconds = selectedDuration;
    }
    
    // Hitung statistik final
    currentStats.timeTaken = seconds;
    currentStats.calculate((int)flatTargetString.length());
    
    terminal.clear();
    int h = terminal.getHeight();
    int w = terminal.getWidth();
    int cy = h / 2;
    int cx = w / 2;
    
    // Gambar Kotak Hasil
    int boxW = 50;
    int boxH = 16;
    drawBox(cx - boxW/2, cy - boxH/2, boxW, boxH, Color::CYAN);

    printCentered(cy - 6, "RESULTS", Color::CYAN);
    
    int labelX = cx - 12;
    int valueX = cx + 5;
    
    // Tampilkan Data
    terminal.setCursor(labelX, cy - 3);
    terminal.print("WPM:");
    terminal.setCursor(valueX, cy - 3);
    terminal.setColor(Color::GREEN);
    terminal.print(std::to_string((int)currentStats.wpm));
    terminal.resetColor();
    
    terminal.setCursor(labelX, cy - 2);
    terminal.print("Accuracy:");
    terminal.setCursor(valueX, cy - 2);
    terminal.print(std::to_string((int)currentStats.accuracy) + "%");

    terminal.setCursor(labelX, cy - 1);
    terminal.print("Time:");
    terminal.setCursor(valueX, cy - 1);
    
    std::stringstream stream;
    stream << std::fixed << std::setprecision(1) << seconds;
    terminal.print(stream.str() + "s");

    terminal.setCursor(labelX, cy);
    terminal.print("Errors:");
    terminal.setCursor(valueX, cy);
    terminal.setColor(Color::RED);
    terminal.print(std::to_string(currentStats.errors));
    terminal.resetColor();

    if (currentMode == "manual") {
        terminal.setCursor(labelX, cy + 1);
        terminal.print("Target:");
        terminal.setCursor(labelX + 20, cy + 1);
        terminal.setColor(Color::YELLOW);
        terminal.print(std::to_string(targetWPM));
        terminal.resetColor();
    }

    // Evaluasi Keberhasilan (Campaign/Manual)
    std::string msg = "";
    Color msgColor = Color::YELLOW;
    
    if (currentMode == "campaign") {
        bool pass = false;
        // Kriteria kelulusan
        if (currentDifficulty == Difficulty::EASY) {
            if (currentStats.wpm >= 40 && currentStats.accuracy >= 80) pass = true;
        } else if (currentDifficulty == Difficulty::MEDIUM) {
            if (currentStats.wpm >= 60 && currentStats.accuracy >= 90) pass = true;
        }
        
        if (pass) {
            msg = "Level Passed! Unlocked Next.";
            // Unlock level berikutnya
            if (currentDifficulty == Difficulty::EASY) unlockedDifficulties[Difficulty::MEDIUM] = true;
            if (currentDifficulty == Difficulty::MEDIUM) unlockedDifficulties[Difficulty::HARD] = true;
        } else {
            if (currentDifficulty != Difficulty::PROGRAMMER && currentDifficulty != Difficulty::HARD)
                msg = "Level Failed.";
        }
    } else if (currentMode == "manual") {
        if (currentStats.wpm >= targetWPM) {
            msg = "TARGET REACHED!";
            msgColor = Color::GREEN;
        } else {
            msg = "TARGET MISSED!";
            msgColor = Color::RED;
        }
    }
    
    if (!msg.empty()) printCentered(cy + 3, msg, msgColor);
    
    // Pause sebentar sebelum bisa lanjut
    printCentered(cy + 6, "...", Color::WHITE);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    while (terminal.hasInput()) terminal.getInput(); // Buang buffer input sisa

    printCentered(cy + 6, "Press ENTER to continue", Color::WHITE); 
    
    // Tunggu ENTER
    while (true) {
        if (terminal.hasInput()) {
            char c = terminal.getInput();
            if (c == 10 || c == 13) break;
        }
    }
    
    currentState = GameState::MENU_DIFFICULTY;
}