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
    outputBuffer << "\033[?1049h";
    flush(); // Flush sekali di initialization
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
    flush(); // Flush terakhir saat cleanup
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
    outputBuffer << "\033[2J\033[H";
    // OPTIMIZATION: Clear adalah operasi penting, langsung flush
    flush();
}

void Terminal::setCursor(int x, int y) {
    // ANSI Code untuk memindahkan kursor: \033[<baris>;<kolom>H
    // OPTIMIZATION: Tidak flush, tunggu batch rendering selesai
    outputBuffer << "\033[" << y << ";" << x << "H";
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
    // OPTIMIZATION: Tidak flush, buffer color code
    outputBuffer << "\033[" << code << "m";
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
    // OPTIMIZATION: Tidak flush, buffer color code
    outputBuffer << "\033[" << code << "m";
}

void Terminal::resetColor() {
    // OPTIMIZATION: Tidak flush, buffer reset code
    outputBuffer << "\033[0m"; // ANSI Reset Code
}

void Terminal::beep() {
    // Beep perlu langsung dieksekusi untuk feedback audio
    std::cout << '\a';
    std::cout.flush();
}

void Terminal::print(const std::string& text) {
    // OPTIMIZATION: Buffer output, tidak langsung flush
    // Ini adalah optimasi utama - mengurangi ratusan flush() calls per frame
    outputBuffer << text;
}

void Terminal::printAt(int x, int y, const std::string& text) {
    setCursor(x, y);
    print(text);
    // OPTIMIZATION: Tidak flush, akan di-flush batch di akhir frame
}

void Terminal::hideCursor() {
    // OPTIMIZATION: Buffer hide cursor command
    outputBuffer << "\033[?25l"; // ANSI Hide Cursor
    // Cursor visibility change perlu flush untuk konsistensi visual
    flush();
}

void Terminal::showCursor() {
    // OPTIMIZATION: Buffer show cursor command
    outputBuffer << "\033[?25h"; // ANSI Show Cursor
    // Cursor visibility change perlu flush untuk konsistensi visual
    flush();
}

void Terminal::flush() {
    // OPTIMIZATION CRITICAL: Fungsi ini dipanggil sekali per frame
    // Menggantikan ratusan flush() calls individual
    std::string buffered = outputBuffer.str();
    if (!buffered.empty()) {
        std::cout << buffered;
        std::cout.flush();
        outputBuffer.str(""); // Clear buffer
        outputBuffer.clear(); // Clear state flags
    }
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