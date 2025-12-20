/**
 * @file Terminal.h
 * @brief Cross-platform terminal manipulation wrapper
 * @author Alea Farrel
 * @date 2025
 * 
 * Menyediakan abstraksi untuk operasi terminal yang bekerja di Windows dan Linux.
 * Mendukung: colors, cursor control, raw mode input, dan responsive sizing.
 */

#ifndef TERMINAL_H
#define TERMINAL_H

#include <string>

/**
 * @enum Color
 * @brief Enum untuk warna ANSI terminal
 * 
 * Abstraksi warna yang akan diterjemahkan ke ANSI escape codes
 */
enum class Color {
    DEFAULT,    ///< Warna default terminal
    RED,        ///< Merah (untuk error/salah)
    GREEN,      ///< Hijau (untuk benar/sukses)
    YELLOW,     ///< Kuning (untuk warning/info)
    BLUE,       ///< Biru (untuk header/accent)
    CYAN,       ///< Cyan (untuk highlight)
    MAGENTA,    ///< Magenta (untuk accent)
    WHITE,      ///< Putih (teks normal)
    BLACK       ///< Hitam
};

/**
 * @class Terminal
 * @brief Wrapper class untuk operasi terminal lintas platform
 * 
 * Class ini mengabstraksi perbedaan antara Windows dan Linux terminal APIs,
 * sehingga code yang sama bisa berjalan di kedua platform.
 * 
 * Features:
 * - Raw mode input (non-blocking, no echo)
 * - ANSI color support
 * - Cursor control (hide/show/move)
 * - Terminal size detection
 * - Clear screen
 */
class Terminal {
public:
    /**
     * @brief Constructor
     */
    Terminal();
    
    /**
     * @brief Destructor - Cleanup terminal state
     */
    ~Terminal();

    // ========================================================================
    // Initialization & Cleanup
    // ========================================================================
    
    /**
     * @brief Initialize terminal (enable raw mode, ANSI support, alternate screen)
     */
    void initialize();
    
    /**
     * @brief Cleanup terminal (restore original mode)
     */
    void cleanup();

    // ========================================================================
    // Drawing & Rendering
    // ========================================================================
    
    /**
     * @brief Clear entire screen
     */
    void clear();
    
    /**
     * @brief Set cursor position
     * @param x Horizontal position (column, 1-based)
     * @param y Vertical position (row, 1-based)
     */
    void setCursor(int x, int y);
    
    /**
     * @brief Set text foreground color
     * @param color Color enum value
     */
    void setColor(Color color);
    
    /**
     * @brief Set text background color
     * @param color Color enum value
     */
    void setBackgroundColor(Color color);
    
    /**
     * @brief Reset all colors to default
     */
    void resetColor();
    
    /**
     * @brief Play system beep sound
     */
    void beep();
    
    /**
     * @brief Print text at current cursor position
     * @param text Text to print
     */
    void print(const std::string& text);
    
    /**
     * @brief Print text at specific position
     * @param x Horizontal position
     * @param y Vertical position
     * @param text Text to print
     */
    void printAt(int x, int y, const std::string& text);
    
    /**
     * @brief Hide cursor
     */
    void hideCursor();
    
    /**
     * @brief Show cursor
     */
    void showCursor();

    // ========================================================================
    // Input Handling
    // ========================================================================
    
    /**
     * @brief Check if there's input available (non-blocking)
     * @return true if key is pressed, false otherwise
     */
    bool hasInput();
    
    /**
     * @brief Read one character from input
     * @return Character code
     */
    char getInput();
    
    /**
     * @brief Enable raw mode (no echo, no line buffering)
     */
    void enableRawMode();
    
    /**
     * @brief Disable raw mode (restore normal mode)
     */
    void disableRawMode();

    // ========================================================================
    // Terminal Info
    // ========================================================================
    
    /**
     * @brief Get terminal width
     * @return Width in columns
     */
    int getWidth();
    
    /**
     * @brief Get terminal height
     * @return Height in rows
     */
    int getHeight();

private:
#ifdef _WIN32
    void* hStdin;               ///< Windows stdin handle
    void* hStdout;              ///< Windows stdout handle
    unsigned long originalMode; ///< Original console mode (for restore)
#else
    // Linux menggunakan struct termios (didefinisikan di .cpp)
#endif
};

#endif // TERMINAL_H