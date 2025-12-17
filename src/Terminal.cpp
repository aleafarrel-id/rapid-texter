#include "Terminal.h"
#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/select.h> // For select()

struct termios originalTermios;
bool isRaw = false;

Terminal::Terminal() {
    // initialize(); // Better to call explicitly? Or in constructor. User wants modular.
}

Terminal::~Terminal() {
    cleanup();
}

void Terminal::initialize() {
    enableRawMode();
    hideCursor();
}

void Terminal::cleanup() {
    disableRawMode();
    showCursor();
}

void Terminal::hideCursor() {
    std::cout << "\033[?25l";
    std::cout.flush();
}

void Terminal::showCursor() {
    std::cout << "\033[?25h";
    std::cout.flush();
}

void Terminal::enableRawMode() {
    if (isRaw) return;
    tcgetattr(STDIN_FILENO, &originalTermios);
    struct termios raw = originalTermios;
    raw.c_lflag &= ~(ECHO | ICANON); // Disable echo and canonical mode
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    isRaw = true;
}

void Terminal::disableRawMode() {
    if (!isRaw) return;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalTermios);
    isRaw = false;
}

void Terminal::clear() {
    std::cout << "\033[2J\033[H";
    std::cout.flush();
}

void Terminal::setCursor(int x, int y) {
    // ANSI uses 1-based indexing
    std::cout << "\033[" << y << ";" << x << "H";
    std::cout.flush();
}

void Terminal::setColor(Color color) {
    // Basic ANSI colors
    int code = 39; // Default
    switch (color) {
        case Color::RED: code = 31; break;
        case Color::GREEN: code = 32; break;
        case Color::YELLOW: code = 33; break;
        case Color::BLUE: code = 34; break;
        case Color::MAGENTA: code = 35; break;
        case Color::CYAN: code = 36; break;
        case Color::WHITE: code = 37; break;
        case Color::BLACK: code = 30; break;
        default: code = 39; break;
    }
    std::cout << "\033[" << code << "m";
}

void Terminal::setBackgroundColor(Color color) {
    int code = 49; // Default background
    switch (color) {
        case Color::RED: code = 41; break;
        case Color::GREEN: code = 42; break;
        case Color::YELLOW: code = 43; break;
        case Color::BLUE: code = 44; break;
        case Color::MAGENTA: code = 45; break;
        case Color::CYAN: code = 46; break;
        case Color::WHITE: code = 47; break;
        case Color::BLACK: code = 40; break;
        default: code = 49; break;
    }
    std::cout << "\033[" << code << "m";
}

void Terminal::resetColor() {
    std::cout << "\033[0m";
}

void Terminal::beep() {
    std::cout << '\a';
    std::cout.flush();
}

void Terminal::print(const std::string& text) {
    std::cout << text;
    std::cout.flush();
}

void Terminal::printAt(int x, int y, const std::string& text) {
    setCursor(x, y);
    print(text);
}

bool Terminal::hasInput() {
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}

char Terminal::getInput() {
    char c = 0;
    read(STDIN_FILENO, &c, 1);
    return c;
}

int Terminal::getWidth() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
}

int Terminal::getHeight() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_row;
}
