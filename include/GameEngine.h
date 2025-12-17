#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include "Terminal.h"
#include "TextProvider.h"
#include "Stats.h"
#include <string>
#include <vector>
#include <chrono>

enum class GameState {
    MENU_LANGUAGE,
    MENU_DURATION,
    MENU_MODE,
    MENU_DIFFICULTY, // or Manual Setup
    PLAYING,
    RESULTS,
    EXIT
};

class GameEngine {
public:
    GameEngine();
    void run();

private:
    Terminal terminal;
    TextProvider textProvider;
    GameState currentState;
    
    // User Settings
    std::string currentLanguage; // "id" or "en"
    std::string currentMode; // "manual" or "campaign"
    Difficulty currentDifficulty;
    int targetWPM; // For manual mode
    int selectedDuration; // 15, 30, 60
    
    // Campaign Progress
    std::map<Difficulty, bool> unlockedDifficulties; 
    
    // Game Session Data
    std::vector<std::string> targetWords;
    std::string flatTargetString; 
    std::string typedString;
    int cursorPosition;
    std::chrono::steady_clock::time_point startTime;
    Stats currentStats;
    int timeLimitSeconds;
    
    // Logic
    bool isGameStarted; // Timer starts on first input

    // Handlers
    void handleMenuLanguage();
    void handleMenuDuration();
    void handleMenuMode();
    void handleMenuDifficulty();
    void gameLoop();
    void showResults();

    // Helpers
    void renderGame();
    void processInput(char c);
    void resetSession();
    
    // UI Helpers
    void drawBox(int x, int y, int w, int h, Color color = Color::WHITE);
    void printCentered(int y, std::string text, Color color = Color::DEFAULT);
    void drawStatusBar();
    std::string getStringInput(bool digitsOnly = false);
};

#endif
