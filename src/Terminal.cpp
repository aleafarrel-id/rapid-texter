#include "Terminal.h"
#include <iostream>

// Platform Detection Block
#ifdef _WIN32
    // ===== WINDOWS IMPLEMENTATION =====
    #include <windows.h>
    #include <conio.h> // Untuk _getch(), _kbhit()
    
    // Variabel statis untuk menyimpan handle dan state asli console
    static HANDLE hStdin = nullptr;
    static HANDLE hStdout = nullptr;
    static DWORD originalInputMode = 0;
    static DWORD originalOutputMode = 0;
    static bool isRaw = false;

#else
    // ===== LINUX/MACOS IMPLEMENTATION =====
    #include <unistd.h>
    #include <termios.h>
    #include <sys/ioctl.h>
    #include <sys/select.h>
    
    static struct termios originalTermios;
    static bool isRaw = false;
#endif

Terminal::Terminal() {
#ifdef _WIN32
    hStdin = nullptr;
    hStdout = nullptr;
    originalMode = 0;
#endif
}

Terminal::~Terminal() {
    cleanup(); // Pastikan terminal kembali normal saat objek dihancurkan
}

void Terminal::initialize() {
#ifdef _WIN32
    // Dapatkan handle ke input dan output standar
    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    
    // Simpan mode original untuk dipulihkan nanti
    GetConsoleMode(hStdin, &originalInputMode);
    GetConsoleMode(hStdout, &originalOutputMode);
    
    // CRITICAL: Aktifkan Virtual Terminal Processing agar ANSI codes (\033...) bekerja di Windows 10/11
    DWORD outputMode = originalOutputMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hStdout, outputMode);
    
    enableRawMode();
#else
    enableRawMode();
#endif
    // Masuk ke Alternate Screen Buffer (Layar khusus aplikasi)
    std::cout << "\033[?1049h";
    std::cout.flush();
    hideCursor(); // Sembunyikan kursor agar tampilan game lebih bersih
}

void Terminal::cleanup() {
#ifdef _WIN32
    if (hStdin && hStdout) {
        // Pulihkan mode asli Windows Console
        SetConsoleMode(hStdin, originalInputMode);
        SetConsoleMode(hStdout, originalOutputMode);
    }
#else
    disableRawMode();
#endif
    showCursor(); // Tampilkan kembali kursor
}

// Mengaktifkan Raw Mode (Input langsung dibaca tanpa tekan Enter, tidak ada echo otomatis)
void Terminal::enableRawMode() {
#ifdef _WIN32
    if (isRaw) return;
    DWORD mode = 0;
    GetConsoleMode(hStdin, &mode);
    // Matikan Echo dan Line Input (buffer per baris)
    mode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);
    SetConsoleMode(hStdin, mode);
    isRaw = true;
#else
    if (isRaw) return;
    tcgetattr(STDIN_FILENO, &originalTermios);
    struct termios raw = originalTermios;
    // Matikan Echo dan Canonical Mode (input buffering)
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    isRaw = true;
#endif
}

// Menonaktifkan Raw Mode (Kembali ke mode normal/cooked)
void Terminal::disableRawMode() {
#ifdef _WIN32
    if (!isRaw) return;
    SetConsoleMode(hStdin, originalInputMode);
    isRaw = false;
#else
    if (!isRaw) return;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalTermios);
    isRaw = false;
#endif
}

void Terminal::clear() {
    // Gunakan ANSI escape codes (works on both platforms jika VT processing aktif)
    // \033[2J: Clear screen, \033[H: Move cursor to home (0,0)
    std::cout << "\033[2J\033[H";
    std::cout.flush(); // Paksa output segera
}

void Terminal::setCursor(int x, int y) {
    // ANSI Code untuk memindahkan kursor: \033[<baris>;<kolom>H
    std::cout << "\033[" << y << ";" << x << "H";
    std::cout.flush();
}

void Terminal::setColor(Color color) {
    int code = 39; // Default Text Color
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
    int code = 49; // Default Background Color
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
    std::cout << "\033[0m"; // ANSI Reset Code
}

void Terminal::beep() {
    std::cout << '\a'; // Karakter Bell
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

void Terminal::hideCursor() {
    std::cout << "\033[?25l"; // ANSI Hide Cursor
    std::cout.flush();
}

void Terminal::showCursor() {
    std::cout << "\033[?25h"; // ANSI Show Cursor
    std::cout.flush();
}

// Mengecek apakah ada tombol yang ditekan (Non-blocking)
bool Terminal::hasInput() {
#ifdef _WIN32
    return _kbhit() != 0; // Windows specific
#else
    // Linux specific: Menggunakan select() untuk cek stdin buffer
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
#endif
}

// Membaca karakter dari input
char Terminal::getInput() {
#ifdef _WIN32
    int ch = _getch();
    
    // Handle arrow keys (2-byte sequence di Windows)
    if (ch == 0 || ch == 224) {
        ch = _getch(); // Baca byte kedua
        // Disini kita bisa memetakan kode panah jika perlu
        return static_cast<char>(ch);
    }
    return static_cast<char>(ch);
#else
    char c = 0;
    read(STDIN_FILENO, &c, 1);
    return c;
#endif
}

int Terminal::getWidth() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hStdout, &csbi);
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
#else
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
#endif
}

int Terminal::getHeight() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hStdout, &csbi);
    return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#else
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_row;
#endif
}