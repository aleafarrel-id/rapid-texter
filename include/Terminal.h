#ifndef TERMINAL_H
#define TERMINAL_H

#include <string>

// Enum warna untuk abstraksi kode warna ANSI
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

// Kelas Wrapper untuk menangani interaksi konsol lintas platform (Windows/Linux)
class Terminal {
public:
    Terminal();
    ~Terminal();

    void initialize();
    void cleanup(); // Mengembalikan terminal ke mode normal

    // Fungsi menggambar/rendering
    void clear();
    void setCursor(int x, int y);
    void setColor(Color color);
    void setBackgroundColor(Color color);
    void resetColor();
    void beep(); // Bunyi sistem
    
    void print(const std::string& text);
    void printAt(int x, int y, const std::string& text);
    
    void hideCursor();
    void showCursor();

    // Fungsi Input
    bool hasInput(); // Cek apakah ada tombol ditekan (non-blocking)
    char getInput(); // Baca tombol
    
    // Mode Raw (tanpa buffer enter, tanpa echo)
    void enableRawMode();
    void disableRawMode();

    // Info layar
    int getWidth();
    int getHeight();

private:
#ifdef _WIN32
    void* hStdin;   // Windows Handle untuk stdin
    void* hStdout;  // Windows Handle untuk stdout
    unsigned long originalMode;
#else
    // Linux menggunakan struct termios yang didefinisikan di .cpp
#endif
};

#endif