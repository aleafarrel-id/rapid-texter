/**
 * @file Terminal.cpp
 * @brief Implementasi cross-platform terminal manipulation wrapper
 * @author Alea Farrel
 * @date 2025
 *
 * File ini mengimplementasikan class Terminal yang menyediakan abstraksi
 * untuk operasi terminal yang bekerja di Windows dan Linux/macOS.
 *
 * @section features Fitur Utama
 * - Raw mode input (non-blocking, no echo)
 * - ANSI color support dengan enum abstraction
 * - Cursor control (hide/show/move)
 * - Terminal size detection
 * - Alternate screen buffer untuk clean UI
 * - Internal output buffering untuk optimasi performa
 *
 * @section platform Platform Support
 *
 * @subsection windows Windows Implementation
 * - Menggunakan Windows Console API (GetStdHandle, SetConsoleMode)
 * - Virtual Terminal Processing untuk ANSI escape codes
 * - _kbhit() dan _getch() untuk non-blocking input
 * - GetConsoleScreenBufferInfo() untuk ukuran terminal
 *
 * @subsection linux Linux/macOS Implementation
 * - Menggunakan termios untuk terminal mode control
 * - select() untuk non-blocking input check
 * - ioctl() dengan TIOCGWINSZ untuk ukuran terminal
 * - read() untuk membaca karakter
 *
 * @section ansi ANSI Escape Codes
 * File ini menggunakan ANSI escape sequences untuk:
 * - Cursor positioning: \033[row;colH
 * - Colors: \033[30-37m (foreground), \033[40-47m (background)
 * - Screen control: \033[2J (clear), \033[?1049h (alternate buffer)
 * - Cursor visibility: \033[?25l (hide), \033[?25h (show)
 */

#include "Terminal.h"
#include <iostream>

// ============================================================================
// PLATFORM-SPECIFIC INCLUDES AND STATIC VARIABLES
// ============================================================================

// Platform Detection Block
#ifdef _WIN32
// ===== WINDOWS IMPLEMENTATION =====
#include <conio.h> // Untuk _getch(), _kbhit()
#include <windows.h>

/**
 * @name Windows Static Variables
 * @brief Variabel statis untuk menyimpan handle dan state console Windows
 *
 * Variabel-variabel ini menyimpan state asli console sebelum aplikasi
 * memodifikasinya, sehingga dapat dipulihkan saat cleanup.
 * @{
 */
static HANDLE hStdin = nullptr;      ///< Handle ke standard input
static HANDLE hStdout = nullptr;     ///< Handle ke standard output
static DWORD originalInputMode = 0;  ///< Mode input asli sebelum raw mode
static DWORD originalOutputMode = 0; ///< Mode output asli
static bool isRaw = false;           ///< Flag: apakah sedang dalam raw mode
                                     /** @} */

#else
// ===== LINUX/MACOS IMPLEMENTATION =====
#include <sys/ioctl.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

/**
 * @name Linux Static Variables
 * @brief Variabel statis untuk menyimpan state termios asli
 * @{
 */
static struct termios originalTermios; ///< Termios settings asli
static bool isRaw = false;             ///< Flag: apakah sedang dalam raw mode
                                       /** @} */
#endif

// ============================================================================
// CONSTRUCTOR & DESTRUCTOR
// ============================================================================

/**
 * @brief Constructor - Inisialisasi member variables
 *
 * Pada Windows, mengatur handle ke nullptr.
 * Constructor tidak melakukan inisialisasi terminal apapun,
 * itu dilakukan di initialize().
 *
 * @note Constructor tidak memulai raw mode atau alternate screen.
 *       Panggil initialize() untuk memulai terminal mode.
 */
Terminal::Terminal() {
#ifdef _WIN32
  hStdin = nullptr;
  hStdout = nullptr;
  originalMode = 0;
#endif
}

/**
 * @brief Destructor - Cleanup terminal state sebelum objek dihancurkan
 *
 * Memastikan terminal kembali ke kondisi normal (cooked mode, main screen)
 * saat objek Terminal dihancurkan, bahkan jika cleanup() tidak dipanggil
 * secara eksplisit.
 *
 * @note Ini adalah safety net - idealnya cleanup() sudah dipanggil
 *       sebelum destructor dieksekusi.
 */
Terminal::~Terminal() {
  cleanup(); // Pastikan terminal kembali normal saat objek dihancurkan
}

// ============================================================================
// INITIALIZATION
// ============================================================================

/**
 * @brief Initialize terminal untuk aplikasi
 *
 * Menyiapkan terminal untuk mode aplikasi full-screen dengan:
 * 1. Mendapatkan handle ke stdin/stdout (Windows)
 * 2. Menyimpan mode asli untuk restore nanti
 * 3. Mengaktifkan Virtual Terminal Processing (Windows 10+)
 * 4. Mengaktifkan raw mode
 * 5. Masuk ke alternate screen buffer
 * 6. Menyembunyikan cursor
 *
 * @par Alternate Screen Buffer
 * Alternate screen adalah layar terpisah yang digunakan oleh aplikasi
 * full-screen. Saat keluar (cleanup), terminal akan kembali ke main
 * screen dengan konten asli yang tidak terpengaruh.
 *
 * @par Virtual Terminal Processing (Windows)
 * Windows 10 dan newer mendukung ANSI escape codes melalui
 * ENABLE_VIRTUAL_TERMINAL_PROCESSING flag. Ini memungkinkan
 * penggunaan escape sequence yang sama dengan Linux.
 *
 * @see cleanup()
 * @see enableRawMode()
 */
void Terminal::initialize() {
#ifdef _WIN32
  // Dapatkan handle ke input dan output standar
  hStdin = GetStdHandle(STD_INPUT_HANDLE);
  hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

  // Simpan mode original untuk dipulihkan nanti
  GetConsoleMode(hStdin, &originalInputMode);
  GetConsoleMode(hStdout, &originalOutputMode);

  // CRITICAL: Aktifkan Virtual Terminal Processing agar ANSI codes (\033...)
  // bekerja di Windows 10/11
  DWORD outputMode = originalOutputMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  SetConsoleMode(hStdout, outputMode);

  enableRawMode();
#else
  enableRawMode();
#endif
  // Masuk ke Alternate Screen Buffer (Layar khusus aplikasi)
  // \033[?1049h adalah escape sequence untuk switch ke alternate buffer
  outputBuffer << "\033[?1049h";
  flush();      // Flush sekali di initialization
  hideCursor(); // Sembunyikan kursor agar tampilan game lebih bersih
}

// ============================================================================
// CLEANUP
// ============================================================================

/**
 * @brief Membersihkan state terminal dan mengembalikan ke kondisi normal
 *
 * Comprehensive terminal state reset yang memastikan terminal kembali
 * ke kondisi normal setelah aplikasi selesai. Ini penting karena:
 * - Rick Roll script mungkin meninggalkan state yang kotor
 * - Crash atau exit abnormal harus tetap cleanup dengan baik
 *
 * @par Urutan Reset
 * 1. Reset SGR (Select Graphic Rendition) - colors, bold, italic
 * 2. Reset scroll region ke full screen
 * 3. Reset character set ke ASCII default
 * 4. Show cursor
 * 5. Reset cursor style
 * 6. Clear screen dan pindahkan cursor ke home
 * 7. Exit alternate screen buffer
 * 8. Reset window title
 * 9. Restore console modes (Windows) atau termios (Linux)
 *
 * @note Fungsi ini dipanggil otomatis oleh destructor
 *
 * @see initialize()
 */
void Terminal::cleanup() {
  // COMPREHENSIVE TERMINAL STATE RESET
  // Problem: ANSI state dari Rick Roll script tidak ter-reset sempurna
  // Solution: Reset SEMUA possible ANSI attributes secara eksplisit

  // Reset ALL SGR (Select Graphic Rendition) attributes
  std::cout << "\033[0m"; // Reset colors, bold, italic, underline, dll

  // Reset scrolling region (jika ter-set oleh script)
  std::cout << "\033[r"; // Reset scroll region to full screen

  // Reset character sets (jika ter-change)
  std::cout << "\033(B"; // Select default character set (ASCII)

  // Show cursor (jika hidden)
  std::cout << "\033[?25h"; // Show cursor

  // Reset cursor style (jika ter-change)
  std::cout << "\033[0 q"; // Reset to default cursor style

  // Clear any remaining screen content
  std::cout << "\033[2J"; // Clear entire screen
  std::cout << "\033[H";  // Move cursor to home (1,1)

  // Exit alternate screen buffer (kembali ke main screen)
  std::cout << "\033[?1049l"; // Exit alternate screen

// Reset window title (jika ter-change oleh script)
#ifdef _WIN32
  std::cout << "\033]0;Windows PowerShell\007"; // Restore default title
#else
  std::cout << "\033]0;Terminal\007"; // Restore default title
#endif

  // Force flush semua reset codes
  std::cout.flush();

  // Restore console modes (Windows) atau termios (Linux)
#ifdef _WIN32
  if (hStdin && hStdout) {
    // Pulihkan mode asli Windows Console
    SetConsoleMode(hStdin, originalInputMode);
    SetConsoleMode(hStdout, originalOutputMode);
  }
#else
  disableRawMode();
#endif

  // Extra safety: print newline untuk memastikan terminal state normal
  std::cout << std::endl;
  std::cout.flush();
}

// ============================================================================
// RAW MODE CONTROL
// ============================================================================

/**
 * @brief Mengaktifkan raw mode untuk input terminal
 *
 * Raw mode berbeda dari mode normal (cooked mode):
 * - Input langsung dibaca tanpa menunggu Enter
 * - Tidak ada echo otomatis ke layar
 * - Karakter kontrol tidak diproses secara default
 *
 * @par Windows Implementation
 * Menggunakan SetConsoleMode() untuk mematikan:
 * - ENABLE_ECHO_INPUT: Input tidak di-echo ke layar
 * - ENABLE_LINE_INPUT: Input tidak di-buffer per baris
 *
 * @par Linux Implementation
 * Menggunakan tcsetattr() untuk mematikan:
 * - ECHO: Input tidak di-echo
 * - ICANON: Non-canonical mode (no line buffering)
 *
 * @note Panggil disableRawMode() untuk mengembalikan ke mode normal
 *
 * @see disableRawMode()
 */
void Terminal::enableRawMode() {
#ifdef _WIN32
  if (isRaw)
    return; // Sudah dalam raw mode
  DWORD mode = 0;
  GetConsoleMode(hStdin, &mode);
  // Matikan Echo dan Line Input (buffer per baris)
  mode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);
  SetConsoleMode(hStdin, mode);
  isRaw = true;
#else
  if (isRaw)
    return; // Sudah dalam raw mode
  tcgetattr(STDIN_FILENO, &originalTermios);
  struct termios raw = originalTermios;
  // Matikan Echo dan Canonical Mode (input buffering)
  raw.c_lflag &= ~(ECHO | ICANON);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
  isRaw = true;
#endif
}

/**
 * @brief Menonaktifkan raw mode dan kembali ke mode normal (cooked mode)
 *
 * Mengembalikan terminal ke mode normal dengan:
 * - Input menunggu Enter sebelum diproses
 * - Karakter di-echo ke layar
 * - Editing shortcuts (backspace, left/right) aktif
 *
 * @par Penggunaan
 * Dipanggil sebelum:
 * - Menjalankan shell command (Rick Roll)
 * - Cleanup saat aplikasi selesai
 *
 * @see enableRawMode()
 */
void Terminal::disableRawMode() {
#ifdef _WIN32
  if (!isRaw)
    return; // Sudah dalam mode normal
  SetConsoleMode(hStdin, originalInputMode);
  isRaw = false;
#else
  if (!isRaw)
    return; // Sudah dalam mode normal
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalTermios);
  isRaw = false;
#endif
}

// ============================================================================
// SCREEN CONTROL
// ============================================================================

/**
 * @brief Membersihkan seluruh layar terminal
 *
 * Menggunakan ANSI escape codes untuk clear screen dan
 * memindahkan cursor ke posisi home (1,1).
 *
 * @par ANSI Codes
 * - \033[2J: Clear entire screen (tidak hanya visible area)
 * - \033[H: Move cursor to home position (1,1)
 *
 * @note Clear adalah operasi penting, langsung di-flush
 */
void Terminal::clear() {
  // Gunakan ANSI escape codes (works on both platforms jika VT processing
  // aktif)
  // \033[2J: Clear screen, \033[H: Move cursor to home (0,0)
  outputBuffer << "\033[2J\033[H";
  // OPTIMIZATION: Clear adalah operasi penting, langsung flush
  flush();
}

/**
 * @brief Memindahkan cursor ke posisi tertentu
 *
 * @param x Posisi horizontal (kolom), 1-indexed
 * @param y Posisi vertikal (baris), 1-indexed
 *
 * @par ANSI Escape Code Format
 * \033[{row};{col}H
 *
 * @note Tidak langsung flush untuk optimasi - multiple setCursor
 *       calls akan di-batch sampai flush() dipanggil
 */
void Terminal::setCursor(int x, int y) {
  // ANSI Code untuk memindahkan kursor: \033[<baris>;<kolom>H
  // OPTIMIZATION: Tidak flush, tunggu batch rendering selesai
  outputBuffer << "\033[" << y << ";" << x << "H";
}

/**
 * @brief Mengatur warna foreground (teks)
 *
 * @param color Enum Color yang menentukan warna
 *
 * @par ANSI Color Codes (Foreground)
 * | Color   | Code |
 * |---------|------|
 * | BLACK   | 30   |
 * | RED     | 31   |
 * | GREEN   | 32   |
 * | YELLOW  | 33   |
 * | BLUE    | 34   |
 * | MAGENTA | 35   |
 * | CYAN    | 36   |
 * | WHITE   | 37   |
 * | DEFAULT | 39   |
 *
 * @note Tidak langsung flush untuk optimasi
 */
void Terminal::setColor(Color color) {
  int code = 39; // Default Text Color
  switch (color) {
  case Color::RED:
    code = 31;
    break;
  case Color::GREEN:
    code = 32;
    break;
  case Color::YELLOW:
    code = 33;
    break;
  case Color::BLUE:
    code = 34;
    break;
  case Color::MAGENTA:
    code = 35;
    break;
  case Color::CYAN:
    code = 36;
    break;
  case Color::WHITE:
    code = 37;
    break;
  case Color::BLACK:
    code = 30;
    break;
  default:
    code = 39;
    break;
  }
  // OPTIMIZATION: Tidak flush, buffer color code
  outputBuffer << "\033[" << code << "m";
}

/**
 * @brief Mengatur warna background
 *
 * @param color Enum Color yang menentukan warna background
 *
 * @par ANSI Color Codes (Background)
 * | Color   | Code |
 * |---------|------|
 * | BLACK   | 40   |
 * | RED     | 41   |
 * | GREEN   | 42   |
 * | YELLOW  | 43   |
 * | BLUE    | 44   |
 * | MAGENTA | 45   |
 * | CYAN    | 46   |
 * | WHITE   | 47   |
 * | DEFAULT | 49   |
 *
 * @note Tidak langsung flush untuk optimasi
 */
void Terminal::setBackgroundColor(Color color) {
  int code = 49; // Default Background Color
  switch (color) {
  case Color::RED:
    code = 41;
    break;
  case Color::GREEN:
    code = 42;
    break;
  case Color::YELLOW:
    code = 43;
    break;
  case Color::BLUE:
    code = 44;
    break;
  case Color::MAGENTA:
    code = 45;
    break;
  case Color::CYAN:
    code = 46;
    break;
  case Color::WHITE:
    code = 47;
    break;
  case Color::BLACK:
    code = 40;
    break;
  default:
    code = 49;
    break;
  }
  // OPTIMIZATION: Tidak flush, buffer color code
  outputBuffer << "\033[" << code << "m";
}

/**
 * @brief Reset semua warna ke default terminal
 *
 * Mengembalikan foreground dan background color ke setting
 * default terminal user.
 *
 * @par ANSI Code
 * \033[0m - Reset ALL SGR parameters
 */
void Terminal::resetColor() {
  // OPTIMIZATION: Tidak flush, buffer reset code
  outputBuffer << "\033[0m"; // ANSI Reset Code
}

/**
 * @brief Memainkan system beep sound
 *
 * Mengirimkan karakter BEL (ASCII 7) ke terminal.
 * Apakah suara terdengar tergantung pada setting terminal user.
 *
 * @note Langsung flush karena ini adalah audio feedback
 */
void Terminal::beep() {
  // Beep perlu langsung dieksekusi untuk feedback audio
  std::cout << '\a';
  std::cout.flush();
}

// ============================================================================
// TEXT OUTPUT
// ============================================================================

/**
 * @brief Mencetak teks ke output buffer
 *
 * Menambahkan teks ke internal buffer. Teks akan ditampilkan
 * ke layar saat flush() dipanggil.
 *
 * @param text String yang akan dicetak
 *
 * @par Optimasi
 * Tidak langsung flush ke stdout. Multiple print() calls akan
 * di-batch untuk mengurangi I/O overhead. Ini menggantikan ratusan
 * flush() calls per frame yang menyebabkan delay.
 *
 * @see flush()
 */
void Terminal::print(const std::string &text) {
  // OPTIMIZATION: Buffer output, tidak langsung flush
  // Ini adalah optimasi utama - mengurangi ratusan flush() calls per frame
  outputBuffer << text;
}

/**
 * @brief Mencetak teks di posisi tertentu
 *
 * Kombinasi setCursor() dan print() dalam satu fungsi untuk kemudahan.
 *
 * @param x Posisi horizontal (kolom)
 * @param y Posisi vertikal (baris)
 * @param text String yang akan dicetak
 *
 * @note Tidak langsung flush - akan di-batch dengan operasi lain
 *
 * @see setCursor()
 * @see print()
 */
void Terminal::printAt(int x, int y, const std::string &text) {
  setCursor(x, y);
  print(text);
  // OPTIMIZATION: Tidak flush, akan di-flush batch di akhir frame
}

// ============================================================================
// CURSOR CONTROL
// ============================================================================

/**
 * @brief Menyembunyikan cursor terminal
 *
 * Menggunakan ANSI escape code untuk menyembunyikan cursor.
 * Berguna untuk gameplay agar cursor tidak mengganggu tampilan.
 *
 * @par ANSI Code
 * \033[?25l - Hide cursor
 *
 * @note Langsung flush untuk memastikan cursor tersembunyi segera
 *
 * @see showCursor()
 */
void Terminal::hideCursor() {
  // OPTIMIZATION: Buffer hide cursor command
  outputBuffer << "\033[?25l"; // ANSI Hide Cursor
  // Cursor visibility change perlu flush untuk konsistensi visual
  flush();
}

/**
 * @brief Menampilkan cursor terminal
 *
 * Menggunakan ANSI escape code untuk menampilkan cursor.
 * Dipanggil saat memerlukan input dari user (seperti custom WPM input).
 *
 * @par ANSI Code
 * \033[?25h - Show cursor
 *
 * @note Langsung flush untuk memastikan cursor terlihat segera
 *
 * @see hideCursor()
 */
void Terminal::showCursor() {
  // OPTIMIZATION: Buffer show cursor command
  outputBuffer << "\033[?25h"; // ANSI Show Cursor
  // Cursor visibility change perlu flush untuk konsistensi visual
  flush();
}

/**
 * @brief Flush internal buffer ke layar
 *
 * Mengirimkan semua konten yang sudah di-buffer ke stdout.
 * Ini adalah optimasi utama yang menggantikan ratusan individual
 * flush() calls yang menyebabkan I/O delay.
 *
 * @par Penggunaan
 * Dipanggil sekali di akhir setiap frame rendering:
 * - Setelah renderGame() di gameplay loop
 * - Setelah selesai menggambar menu
 * - Setelah clear screen operations
 *
 * @par Mekanisme
 * 1. Ambil string dari ostringstream buffer
 * 2. Jika tidak kosong, kirim ke std::cout
 * 3. Flush std::cout ke OS
 * 4. Clear buffer untuk frame berikutnya
 */
void Terminal::flush() {
  // OPTIMIZATION: Fungsi ini dipanggil sekali per frame
  // Menggantikan ratusan flush() calls individual
  std::string buffered = outputBuffer.str();
  if (!buffered.empty()) {
    std::cout << buffered;
    std::cout.flush();
    outputBuffer.str(""); // Clear buffer
    outputBuffer.clear(); // Clear state flags
  }
}

// ============================================================================
// INPUT HANDLING
// ============================================================================

/**
 * @brief Mengecek apakah ada input yang tersedia (non-blocking)
 *
 * Fungsi ini tidak menunggu input, langsung return true/false.
 * Berguna untuk gameplay loop yang perlu terus berjalan
 * tanpa menunggu user input.
 *
 * @return true jika ada tombol yang ditekan
 * @return false jika tidak ada input
 *
 * @par Windows Implementation
 * Menggunakan _kbhit() dari <conio.h> yang mengecek
 * keyboard buffer secara langsung.
 *
 * @par Linux Implementation
 * Menggunakan select() dengan timeout 0 untuk mengecek
 * apakah ada data di stdin buffer tanpa blocking.
 *
 * @see getInput()
 */
bool Terminal::hasInput() {
#ifdef _WIN32
  return _kbhit() != 0; // Windows specific
#else
  // Linux specific: Menggunakan select() untuk cek stdin buffer
  struct timeval tv;
  fd_set fds;
  tv.tv_sec = 0;  // Timeout: 0 detik
  tv.tv_usec = 0; // Timeout: 0 mikrodetik (instant check)
  FD_ZERO(&fds);
  FD_SET(STDIN_FILENO, &fds);
  select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
  return FD_ISSET(STDIN_FILENO, &fds);
#endif
}

/**
 * @brief Membaca satu karakter dari input
 *
 * Membaca karakter tanpa echo dan tanpa menunggu Enter.
 * Harus dipanggil setelah hasInput() return true, atau akan blocking.
 *
 * @return char Karakter yang ditekan
 *
 * @par Windows Implementation
 * Menggunakan _getch() yang membaca langsung dari keyboard buffer.
 * Special handling untuk arrow keys yang menggunakan 2-byte sequence:
 * - Byte pertama: 0 atau 224
 * - Byte kedua: kode tombol
 *
 * @par Linux Implementation
 * Menggunakan read() untuk membaca 1 byte dari stdin.
 *
 * @note Escape sequences (arrow keys, function keys) memerlukan
 *       handling khusus yang tidak diimplementasikan lengkap di sini.
 *
 * @see hasInput()
 */
char Terminal::getInput() {
#ifdef _WIN32
  int ch = _getch();

  // Handle arrow keys (2-byte sequence di Windows)
  // Byte pertama 0 atau 224 menandakan special key
  if (ch == 0 || ch == 224) {
    ch = _getch(); // Baca byte kedua (kode tombol sebenarnya)
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

// ============================================================================
// TERMINAL INFO
// ============================================================================

/**
 * @brief Mendapatkan lebar terminal dalam kolom (karakter)
 *
 * @return int Jumlah kolom terminal
 *
 * @par Windows Implementation
 * Menggunakan GetConsoleScreenBufferInfo() untuk mendapatkan
 * dimensi aktual window (bukan buffer).
 *
 * @par Linux Implementation
 * Menggunakan ioctl() dengan TIOCGWINSZ untuk query window size.
 *
 * @note Nilai ini akan berubah jika user resize terminal window.
 *       GameEngine memanggil fungsi ini setiap loop iteration
 *       untuk responsive rendering.
 *
 * @see getHeight()
 */
int Terminal::getWidth() {
#ifdef _WIN32
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  GetConsoleScreenBufferInfo(hStdout, &csbi);
  // Hitung lebar dari koordinat window (bukan buffer)
  return csbi.srWindow.Right - csbi.srWindow.Left + 1;
#else
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  return w.ws_col;
#endif
}

/**
 * @brief Mendapatkan tinggi terminal dalam baris
 *
 * @return int Jumlah baris terminal
 *
 * @par Windows Implementation
 * Menggunakan GetConsoleScreenBufferInfo() untuk mendapatkan
 * dimensi aktual window (bukan buffer).
 *
 * @par Linux Implementation
 * Menggunakan ioctl() dengan TIOCGWINSZ untuk query window size.
 *
 * @note Nilai ini akan berubah jika user resize terminal window.
 *
 * @see getWidth()
 */
int Terminal::getHeight() {
#ifdef _WIN32
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  GetConsoleScreenBufferInfo(hStdout, &csbi);
  // Hitung tinggi dari koordinat window (bukan buffer)
  return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#else
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  return w.ws_row;
#endif
}

/**
 * @brief Mengecek apakah Caps Lock sedang aktif
 *
 * Berguna untuk memberi warning kepada user saat typing test
 * bahwa Caps Lock menyala, yang bisa menyebabkan kesalahan ketik.
 *
 * @return true jika Caps Lock ON
 * @return false jika Caps Lock OFF atau tidak dapat ditentukan
 *
 * @par Windows Implementation
 * Menggunakan GetKeyState(VK_CAPITAL) yang mengecek toggle state.
 * Low-order bit menunjukkan apakah Caps Lock aktif.
 *
 * @par Linux Implementation
 * Mencoba membaca dari /sys/class/leds/input0::capslock/brightness.
 * Value '1' berarti Caps Lock menyala.
 * Fallback ke input1 jika input0 tidak ada.
 *
 * @warning Linux implementation bergantung pada sysfs yang mungkin
 *          tidak tersedia di semua distribusi atau konfigurasi.
 */
bool Terminal::isCapsLockOn() {
#ifdef _WIN32
  // GetKeyState returns the toggle state in the low-order bit
  return (GetKeyState(VK_CAPITAL) & 0x0001) != 0;
#else
  // Linux: Check LED state from /sys filesystem
  // Try multiple input paths for better compatibility across different laptops
  char path[64];
  for (int i = 0; i <= 9; i++) {
    snprintf(path, sizeof(path), "/sys/class/leds/input%d::capslock/brightness",
             i);
    FILE *fp = fopen(path, "r");
    if (fp) {
      int state = fgetc(fp);
      fclose(fp);
      return state == '1';
    }
  }
  return false; // Cannot determine, assume off
#endif
}