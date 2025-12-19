#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include "Terminal.h"
#include "TextProvider.h"
#include "Stats.h"
#include <string>
#include <vector>
#include <chrono>

// Enum untuk mengelola Status Aplikasi (State Machine)
enum class GameState {
    MENU_LANGUAGE,
    MENU_DURATION,
    MENU_MODE,
    MENU_DIFFICULTY, // Menu pemilihan kesulitan atau manual setup
    PLAYING,         // Saat game sedang berjalan
    RESULTS,         // Layar skor akhir
    EXIT             // Keluar dari program
};

class GameEngine {
public:
    GameEngine();
    void run(); // Loop utama

private:
    Terminal terminal;
    TextProvider textProvider;
    GameState currentState;
    
    // Setting Pengguna
    std::string currentLanguage; // "id" atau "en"
    std::string currentMode; // "manual" atau "campaign"
    Difficulty currentDifficulty;
    int targetWPM; // Target WPM untuk mode manual
    int selectedDuration; // 15, 30, 60 detik (-1 untuk tanpa batas)
    
    // Progress Campaign (Sistem Unlock Level)
    std::map<Difficulty, bool> unlockedDifficulties; 
    
    // Data Sesi Game Saat Ini
    std::vector<std::string> targetWords;
    std::string flatTargetString; // Semua kata digabung jadi satu string panjang
    std::string typedString;      // Apa yang sudah diketik user
    int cursorPosition;           // Posisi karakter yang sedang diketik
    std::chrono::steady_clock::time_point startTime;
    Stats currentStats;
    int timeLimitSeconds;
    
    // Logic Flags
    bool isGameStarted; // True jika user sudah mulai mengetik huruf pertama

    // Handlers (Fungsi per State)
    void handleMenuLanguage();
    void handleMenuDuration();
    void handleMenuMode();
    void handleMenuDifficulty();
    void gameLoop();
    void showResults();

    // Helpers Logika Game
    void renderGame();
    void processInput(char c);
    void resetSession();
    
    // Helpers Tampilan UI
    void drawBox(int x, int y, int w, int h, Color color = Color::WHITE);
    void printCentered(int y, std::string text, Color color = Color::DEFAULT);
    void drawStatusBar();
    std::string getStringInput(bool digitsOnly = false);
};

#endif