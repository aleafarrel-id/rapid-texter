#ifndef TERMINAL_H
#define TERMINAL_H

#include <string>

// ===== ENUM COLOR DIPINDAHKAN KE SINI AGAR ACCESSIBLE =====
enum class Color {
    DEFAULT,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    CYAN,
    MAGENTA,
    WHITE,
    BLACK
};

class Terminal {
public:
    Terminal();
    ~Terminal();

    void initialize();
    void cleanup();

    void clear();
    void setCursor(int x, int y);
    void setColor(Color color);
    void setBackgroundColor(Color color);
    void resetColor();
    void beep();
    
    void print(const std::string& text);
    void printAt(int x, int y, const std::string& text);
    
    void hideCursor();
    void showCursor();

    bool hasInput();
    char getInput();
    
    void enableRawMode();
    void disableRawMode();

    int getWidth();
    int getHeight();

private:
#ifdef _WIN32
    void* hStdin;   // HANDLE untuk stdin
    void* hStdout;  // HANDLE untuk stdout
    unsigned long originalMode;
#else
    // Linux: akan disimpan di .cpp
#endif
};

#endif