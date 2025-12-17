#ifndef TERMINAL_H
#define TERMINAL_H

#include <string>

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
    void resetColor();
    void beep();
    
    // Print text at current cursor header
    void print(const std::string& text);
    
    // Print text at specific location
    void printAt(int x, int y, const std::string& text);
    
    void hideCursor();
    void showCursor();

    // Check if input is available (non-blocking)
    bool hasInput();

    // Get a single character input (blocking or non-blocking logic can be handled inside or via flags, 
    // for now this returns the char read)
    char getInput();
    
    // Enable/Disable raw mode (already handled in init/cleanup usually, but exposed if needed)
    void enableRawMode();
    void disableRawMode();

    int getWidth();
    int getHeight();

private:
    struct termios_impl; // PImpl or just hiding details if we wanted, but header-only struct is easier. 
    // Actually we'll just put implementation details in cpp.
};

#endif
