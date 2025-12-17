#include "GameEngine.h"
#include <iostream>
#include <sstream>
#include <thread>      // Cross-platform sleep
#include <chrono>      // Cross-platform time
#include <iomanip>
#include <cmath>

GameEngine::GameEngine() : currentState(GameState::MENU_LANGUAGE) {
    terminal.initialize();
    
    // Initial Unlocks
    unlockedDifficulties[Difficulty::EASY] = true;
    unlockedDifficulties[Difficulty::MEDIUM] = false;
    unlockedDifficulties[Difficulty::HARD] = false;
    unlockedDifficulties[Difficulty::PROGRAMMER] = true; 
    
    // Load Words
    textProvider.loadWords("id", "id.txt");
    textProvider.loadWords("en", "en.txt");
    textProvider.loadWords("prog", "prog.txt");
    
    selectedDuration = 30; // Default
}

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

// === UI HELPERS ===

void GameEngine::drawBox(int x, int y, int w, int h, Color color) {
    terminal.setColor(color);
    terminal.setCursor(x, y);
    terminal.print("+");
    for(int i=0; i<w-2; ++i) terminal.print("-");
    terminal.print("+");
    
    for(int i=1; i<h-1; ++i) {
        terminal.setCursor(x, y+i);
        terminal.print("|");
        terminal.setCursor(x+w-1, y+i);
        terminal.print("|");
    }
    
    terminal.setCursor(x, y+h-1);
    terminal.print("+");
    for(int i=0; i<w-2; ++i) terminal.print("-");
    terminal.print("+");
    terminal.resetColor();
}

void GameEngine::printCentered(int y, std::string text, Color color) {
    int cx = terminal.getWidth() / 2;
    int x = cx - (text.length() / 2);
    terminal.setCursor(x, y);
    if (color != Color::DEFAULT) terminal.setColor(color);
    terminal.print(text);
    terminal.resetColor();
}

void GameEngine::drawStatusBar() {
    int w = terminal.getWidth();
    int h = terminal.getHeight();
    int y = h - 2;
    
    std::string lang = currentLanguage.empty() ? "N/A" : (currentLanguage == "id" ? "ID" : (currentLanguage == "en" ? "EN" : "PROG"));
    std::string time;
    if (selectedDuration == -1) time = "Inf";
    else if (selectedDuration == 0) time = "30s";
    else time = std::to_string(selectedDuration) + "s";

    std::string mode = currentMode.empty() ? "N/A" : (currentMode == "manual" ? "Manual" : "Campaign");
    if (currentLanguage == "prog") mode = "Programmer";
    
    std::string status = " Lang: " + lang + " | Time: " + time + " | Mode: " + mode + " ";
    
    terminal.setBackgroundColor(Color::BLUE);
    for(int i=0; i<w; ++i) {
        terminal.setCursor(i, y);
        terminal.print(" ");
    }
    printCentered(y, status, Color::WHITE);
    terminal.resetColor();
}

std::string GameEngine::getStringInput(bool digitsOnly) {
    std::string inputBuf = "";
    int startX = terminal.getWidth() / 2 - 10;
    int startY = terminal.getHeight() / 2 + 1;
    
    terminal.setCursor(startX + inputBuf.length(), startY);
    terminal.showCursor();

    while(true) {
        if (terminal.hasInput()) {
            char c = terminal.getInput();
            if (c == 27) { terminal.hideCursor(); return ""; }
            if (c == 10 || c == 13) { terminal.hideCursor(); return inputBuf; }
            
            if (c == 127 || c == '\b' || c == 8) { // Backspace (8 for Windows)
                if (!inputBuf.empty()) {
                    inputBuf.pop_back();
                    terminal.setCursor(startX + inputBuf.length(), startY);
                    terminal.print(" ");
                    terminal.setCursor(startX + inputBuf.length(), startY);
                }
            } else if (!digitsOnly || (c >= '0' && c <= '9')) {
                if (c >= 32 && c <= 126 && inputBuf.length() < 20) {
                     inputBuf += c;
                     terminal.print(std::string(1, c));
                }
            }
        }
    }
}

// === HANDLERS ===

void GameEngine::handleMenuLanguage() {
    terminal.clear();
    int h = terminal.getHeight();
    int w = terminal.getWidth();
    int cy = h / 2;
    int cx = w / 2;

    std::string title1 = " ____  _  _  ____  ____  ____ ";
    std::string title2 = "(  _ \\/ )( \\(  _ \\(_  _)(    \\";
    std::string title3 = " )   /) __ ( ) __/ _)(_  ) D (";
    std::string title4 = "(__\\_)\\_)(_/(__)  (____)(____/";
    std::string subtitle = "RAPID TEXTER";

    int boxW = 50;
    int boxH = 14;
    drawBox(cx - boxW/2, cy - boxH/2, boxW, boxH, Color::CYAN);

    printCentered(cy - 5, title1, Color::CYAN);
    printCentered(cy - 4, title2, Color::CYAN);
    printCentered(cy - 3, title3, Color::CYAN);
    printCentered(cy - 2, title4, Color::CYAN);
    printCentered(cy, subtitle, Color::BLUE);

    printCentered(cy + 2, "[1] Indonesia (ID)");
    printCentered(cy + 3, "[2] English (EN)");
    printCentered(cy + 5, "(Q) Quit", Color::RED);
    
    drawStatusBar();

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
    
    if (currentMode == "manual") {
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
            targetWPM = 40;
        }
        currentDifficulty = Difficulty::MEDIUM;
        currentState = GameState::PLAYING;
        resetSession();
        return; 
    } else {
        int boxW = 50;
        int boxH = 16;
        drawBox(cx - boxW/2, cy - boxH/2, boxW, boxH, Color::MAGENTA);
        
        printCentered(cy - 6, "CAMPAIGN DIFFICULTY", Color::MAGENTA);
        
        std::string e = unlockedDifficulties[Difficulty::EASY] ? " " : " [LOCKED]";
        std::string m = unlockedDifficulties[Difficulty::MEDIUM] ? " " : " [LOCKED]";
        std::string h = unlockedDifficulties[Difficulty::HARD] ? " " : " [LOCKED]";
        
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
                
                bool valid = false;
                if (d == '1' && unlockedDifficulties[Difficulty::EASY]) { currentDifficulty = Difficulty::EASY; valid = true; }
                if (d == '2' && unlockedDifficulties[Difficulty::MEDIUM]) { currentDifficulty = Difficulty::MEDIUM; valid = true; }
                if (d == '3' && unlockedDifficulties[Difficulty::HARD]) { currentDifficulty = Difficulty::HARD; valid = true; }
                if (d == '4') { currentDifficulty = Difficulty::PROGRAMMER; currentLanguage = "prog"; valid = true; } 
                
                if (valid) {
                    if (currentDifficulty == Difficulty::PROGRAMMER) currentLanguage = "prog";
                    currentState = GameState::PLAYING;
                    resetSession();
                    return;
                }
             }
        }
    }
}

void GameEngine::resetSession() {
    targetWords = textProvider.getWords(currentLanguage, currentDifficulty, 30); 
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

void GameEngine::renderGame() {
    int w = terminal.getWidth();
    int h = terminal.getHeight();
    int cy = h / 2;
    int cx = w / 2;
    
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

    std::string info = " MODE: " + currentMode + " | TIME: " + timeStr + "   ";
    terminal.setCursor(cx - info.length()/2, 2);
    terminal.resetColor();
    terminal.print(info);

    std::string footer = "TAB: Restart | ESC: Exit";
    terminal.setCursor(cx - (footer.length()/2), h - 2);
    terminal.setColor(Color::YELLOW);
    terminal.print(footer);
    terminal.resetColor();

    int boxWidth = 60;
    if (w < 64) boxWidth = w - 4;
    int startX = (w - boxWidth) / 2;
    int startY = cy - 5; 

    int curX = startX;
    int curY = startY;
    
    for (size_t i = 0; i < flatTargetString.length(); ++i) {
        if (curX >= startX + boxWidth) {
            curX = startX;
            curY += 2; 
        }

        char targetChar = flatTargetString[i];
        char charToDraw = targetChar;
        Color color = Color::WHITE; 

        if (i < typedString.length()) {
            char typedChar = typedString[i];
            if (typedChar == targetChar) {
                color = Color::GREEN; 
            } else {
                color = Color::RED; 
                if (targetChar == ' ') {
                    charToDraw = typedChar;
                    if (charToDraw == ' ') charToDraw = '_';
                }
            }
        }

        terminal.setColor(color);
        terminal.printAt(curX, curY, std::string(1, charToDraw));

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
    
    if (!isGameStarted) {
        printCentered(cy + 6, "Type to start...", Color::WHITE);
    } else {
         printCentered(cy + 6, "                 ", Color::WHITE); 
    }
    
    terminal.hideCursor(); 
}

void GameEngine::gameLoop() {
    terminal.clear(); 
    
    while (currentState == GameState::PLAYING) {
        
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
        
        if (terminal.hasInput()) {
            char c = terminal.getInput();
            processInput(c);
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
}

void GameEngine::processInput(char c) {
    if (c == 27) {
        currentState = GameState::MENU_DIFFICULTY; 
        return;
    }
    if (c == 9) {
        resetSession();
        return;
    }
    
    if (c == 127 || c == '\b' || c == 8) { // Backspace (8 for Windows)
        if (cursorPosition > 0) {
            cursorPosition--;
            typedString.pop_back();
        }
    } else {
        if (!isGameStarted) {
            isGameStarted = true;
            startTime = std::chrono::steady_clock::now();
        }

        if (cursorPosition < (int)flatTargetString.length()) {
            char targetChar = flatTargetString[cursorPosition];
            
            typedString += c;
            
            if (targetChar == c) {
                currentStats.correctKeystrokes++;
            } else {
                currentStats.errors++;
                terminal.beep();
            }
            currentStats.totalKeystrokes++;
            cursorPosition++;
            
            if (cursorPosition >= (int)flatTargetString.length()) {
                currentState = GameState::RESULTS;
            }
        }
    }
}

void GameEngine::showResults() {
    double seconds = 0;
    if (isGameStarted) {
        auto now = std::chrono::steady_clock::now();
        seconds = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count() / 1000.0;
        if (selectedDuration != -1 && seconds > selectedDuration) seconds = selectedDuration;
    }
    
    currentStats.timeTaken = seconds;
    currentStats.calculate((int)flatTargetString.length());
    
    terminal.clear();
    int h = terminal.getHeight();
    int w = terminal.getWidth();
    int cy = h / 2;
    int cx = w / 2;
    
    int boxW = 50;
    int boxH = 16;
    drawBox(cx - boxW/2, cy - boxH/2, boxW, boxH, Color::CYAN);

    printCentered(cy - 6, "RESULTS", Color::CYAN);
    
    int labelX = cx - 12;
    int valueX = cx + 5;
    
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

    std::string msg = "";
    Color msgColor = Color::YELLOW;
    
    if (currentMode == "campaign") {
        bool pass = false;
        if (currentDifficulty == Difficulty::EASY) {
            if (currentStats.wpm >= 40 && currentStats.accuracy >= 80) pass = true;
        } else if (currentDifficulty == Difficulty::MEDIUM) {
            if (currentStats.wpm >= 60 && currentStats.accuracy >= 90) pass = true;
        }
        
        if (pass) {
            msg = "Level Passed! Unlocked Next.";
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
    
    printCentered(cy + 6, "...", Color::WHITE);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    while (terminal.hasInput()) terminal.getInput();

    printCentered(cy + 6, "Press ENTER to continue", Color::WHITE); 
    
    while (true) {
        if (terminal.hasInput()) {
            char c = terminal.getInput();
            if (c == 10 || c == 13) break;
        }
    }
    
    currentState = GameState::MENU_DIFFICULTY;
}