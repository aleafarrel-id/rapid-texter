/**
 * @file GameUI.h
 * @brief UI/UX Helper class untuk Rapid Texter
 * @author Alea Farrel
 * @date 2025
 * 
 * Memisahkan semua fungsi rendering UI dari GameEngine untuk modularitas.
 */

#ifndef GAMEUI_H
#define GAMEUI_H

#include "Terminal.h"
#include <string>

class GameUI {
public:
    /**
     * @brief Constructor - Initialize dengan Terminal reference
     */
    explicit GameUI(Terminal& term);
    
    /**
     * @brief Menggambar kotak/border ASCII
     */
    void drawBox(int x, int y, int w, int h, Color color = Color::WHITE);
    
    /**
     * @brief Mencetak teks di tengah layar secara horizontal
     */
    void printCentered(int y, std::string text, Color color = Color::DEFAULT);
    
    /**
     * @brief Menggambar status bar di bawah layar
     * @param sfxEnabled Status SFX (true = On, false = Off)
     */
    void drawStatusBar(const std::string& language, int duration, const std::string& mode, bool sfxEnabled = true);
    
    /**
     * @brief Mendapatkan input string dari user
     */
    std::string getStringInput(bool digitsOnly = false);

private:
    Terminal& terminal;
};

#endif // GAMEUI_H
