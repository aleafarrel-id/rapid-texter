#include "GameUI.h"
#include <thread>
#include <chrono>

GameUI::GameUI(Terminal& term) : terminal(term) {}

void GameUI::drawBox(int x, int y, int w, int h, Color color) {
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

void GameUI::printCentered(int y, std::string text, Color color) {
    int cx = terminal.getWidth() / 2;
    int x = cx - (text.length() / 2);
    terminal.setCursor(x, y);
    if (color != Color::DEFAULT) terminal.setColor(color);
    terminal.print(text);
    terminal.resetColor();
}

void GameUI::drawStatusBar(const std::string& language, int duration, const std::string& mode) {
    int w = terminal.getWidth();
    int h = terminal.getHeight();
    int y = h - 2;

    // Siapkan teks status
    std::string lang = language.empty() ? "N/A" : 
                      (language == "id" ? "ID" : 
                      (language == "en" ? "EN" : "PROG"));
    
    std::string time;
    if (duration == -1) time = "Inf";
    else if (duration == 0) time = "30s";
    else time = std::to_string(duration) + "s";

    std::string modeText = mode.empty() ? "N/A" : 
                          (mode == "manual" ? "Manual" : "Campaign");
    if (language == "prog") modeText = "Programmer";

    std::string status = " Lang: " + lang + " | Time: " + time + " | Mode: " + modeText + " ";

    // Tentukan lebar bar
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

std::string GameUI::getStringInput(bool digitsOnly) {
    std::string inputBuf = "";
    int startX = terminal.getWidth() / 2 - 10;
    int startY = terminal.getHeight() / 2 + 1;
    
    terminal.setCursor(startX + inputBuf.length(), startY);
    terminal.showCursor();

    while(true) {
        if (terminal.hasInput()) {
            char c = terminal.getInput();
            
            if (c == 27) { 
                terminal.hideCursor(); 
                return ""; 
            }
            
            if (c == 10 || c == 13) { 
                terminal.hideCursor(); 
                return inputBuf; 
            }
            
            if (c == 127 || c == '\b' || c == 8) { 
                if (!inputBuf.empty()) {
                    inputBuf.pop_back();
                    terminal.setCursor(startX + inputBuf.length(), startY);
                    terminal.print(" ");
                    terminal.setCursor(startX + inputBuf.length(), startY);
                    terminal.flush();
                }
            } 
            else if (!digitsOnly || (c >= '0' && c <= '9')) {
                if (c >= 32 && c <= 126 && inputBuf.length() < 20) {
                     inputBuf += c;
                     terminal.print(std::string(1, c));
                     terminal.flush();
                }
            }
        }
    }
}