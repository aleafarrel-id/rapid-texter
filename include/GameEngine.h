/**
 * @file GameEngine.h
 * @brief Header file untuk Game Engine Rapid Texter
 * @author Alea Farrel
 * @date 2025
 *
 * Mendefinisikan class GameEngine yang mengelola State Machine aplikasi
 * dan menangani semua logic gameplay, menu, dan UI.
 */

#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include "GameUI.h"
#include "HistoryManager.h"
#include "ProgressManager.h"
#include "SFXManager.h"
#include "SettingsManager.h"
#include "Stats.h"
#include "Terminal.h"
#include "TextProvider.h"
#include <chrono>
#include <string>
#include <vector>

// ============================================================================
// ENUMERATIONS
// ============================================================================

/**
 * @enum GameState
 * @brief Enum untuk mengelola status aplikasi menggunakan State Machine
 *
 * Setiap state merepresentasikan layar/tampilan berbeda dalam aplikasi.
 */
enum class GameState {
  MENU_MAIN,       ///< Menu utama (Start, History, Quit)
  MENU_LANGUAGE,   ///< Menu pemilihan bahasa (ID/EN)
  MENU_DURATION,   ///< Menu pemilihan durasi (15s/30s/60s/Custom/Unlimited)
  MENU_MODE,       ///< Menu pemilihan mode (Manual/Campaign)
  MENU_DIFFICULTY, ///< Menu kesulitan atau setup manual
  MENU_HISTORY,    ///< Menu tampilan history - BARU
  PLAYING,         ///< State saat game sedang berlangsung
  RESULTS,         ///< Layar hasil/statistik akhir
  CREDITS,         ///< Layar credits
  EXIT             ///< Keluar dari program
};

// ============================================================================
// GAME ENGINE CLASS
// ============================================================================

/**
 * @class GameEngine
 * @brief Class utama yang mengelola seluruh logic aplikasi Rapid Texter
 *
 * GameEngine bertanggung jawab untuk:
 * - Mengelola State Machine (perpindahan antar menu/screen)
 * - Menangani input user
 * - Me-render UI dan gameplay
 * - Menghitung statistik (WPM, accuracy, dll)
 * - Mengelola campaign progression (unlock system)
 */
class GameEngine {
public:
  // ========================================================================
  // PUBLIC METHODS
  // ========================================================================

  /**
   * @brief Constructor - Initialize game engine
   */
  GameEngine();

  /**
   * @brief Menjalankan loop utama aplikasi (State Machine)
   */
  void run();

private:
  // ========================================================================
  // PRIVATE MEMBERS - Components
  // ========================================================================

  Terminal terminal; ///< Handler untuk operasi terminal (I/O, colors, cursor)
  GameUI gameUI;     ///< Handler untuk UI rendering
  TextProvider textProvider; ///< Provider untuk database kata-kata
  ProgressManager
      progressManager; ///< Manager untuk penyimpanan progress campaign
  HistoryManager
      historyManager;      ///< Manager untuk penyimpanan history permainan
  GameState currentState;  ///< State aktif saat ini dalam State Machine
  GameState previousState; ///< State sebelumnya (untuk kembali dari credits)

  // ========================================================================
  // PRIVATE MEMBERS - User Settings
  // ========================================================================

  std::string currentLanguage;  ///< Bahasa yang dipilih ("id", "en", "prog")
  std::string originalLanguage; ///< Bahasa asli yang dipilih user (ID/EN) -
                                ///< untuk restore setelah Programmer mode
  std::string currentMode;      ///< Mode game ("manual" atau "campaign")
  Difficulty
      currentDifficulty; ///< Tingkat kesulitan (EASY/MEDIUM/HARD/PROGRAMMER)
  int targetWPM;         ///< Target WPM untuk mode manual
  int selectedDuration;  ///< Durasi yang dipilih (detik, -1 untuk unlimited)

  // ========================================================================
  // PRIVATE MEMBERS - Campaign Progress
  // ========================================================================

  /**
   * @brief Map untuk tracking level mana yang sudah di-unlock
   *
   * Key: Difficulty level
   * Value: true jika sudah unlock, false jika masih terkunci
   *
   * Unlock Requirements:
   * - Easy: Unlocked by default
   * - Medium: Unlock dengan 40 WPM + 80% accuracy di Easy
   * - Hard: Unlock dengan 60 WPM + 90% accuracy di Medium
   * - Programmer: Selalu unlocked (bonus mode)
   */
  bool rickRollAlreadyShown; ///< Flag untuk mencegah Rick Roll dipanggil dua
                             ///< kali berturut-turut

  // ========================================================================
  // PRIVATE MEMBERS - Current Game Session Data
  // ========================================================================

  std::vector<std::string> targetWords; ///< List kata yang harus diketik
  std::string flatTargetString;         ///< Semua kata digabung jadi 1 string
  std::string typedString;              ///< String yang sudah diketik user
  int cursorPosition;                   ///< Posisi karakter yang sedang diketik
  std::chrono::steady_clock::time_point startTime; ///< Waktu mulai mengetik
  Stats currentStats;                              ///< Statistik sesi saat ini
  int timeLimitSeconds;                            ///< Sisa waktu (detik)
  bool isGameStarted; ///< Flag: apakah user sudah mulai mengetik?

  // ========================================================================
  // PRIVATE METHODS - State Handlers
  // ========================================================================

  /**
   * @brief Handler untuk menu utama (Start, History, Quit)
   */
  void handleMenuMain();

  /**
   * @brief Handler untuk menu history dengan pagination
   */
  void handleMenuHistory();

  /**
   * @brief Handler untuk menu pemilihan bahasa
   */
  void handleMenuLanguage();

  /**
   * @brief Handler untuk menu pemilihan durasi
   */
  void handleMenuDuration();

  /**
   * @brief Handler untuk menu pemilihan mode (manual/campaign)
   */
  void handleMenuMode();

  /**
   * @brief Handler untuk menu pemilihan kesulitan atau setup manual
   */
  void handleMenuDifficulty();

  /**
   * @brief Loop utama gameplay (input handling + rendering)
   */
  void gameLoop();

  /**
   * @brief Handler untuk layar hasil/statistik
   */
  void showResults();

  /**
   * @brief Handler untuk layar credits
   */
  void showCredits();

  // ========================================================================
  // PRIVATE METHODS - Game Logic Helpers
  // ========================================================================

  /**
   * @brief Me-render tampilan gameplay (kata-kata dengan highlight warna)
   */
  void renderGame();

  /**
   * @brief Memproses setiap input keyboard saat gameplay
   * @param c Karakter yang ditekan
   */
  void processInput(char c);

  /**
   * @brief Reset semua data sesi game (kata, statistik, timer)
   */
  void resetSession();

  /**
   * @brief Menjalankan Rick Roll easter egg
   */
  void playRickRoll();

  /**
   * @brief Restore bahasa ke pilihan asli setelah Programmer Mode
   *
   * Function ini memastikan bahasa kembali ke ID/EN yang dipilih user
   * setelah selesai atau keluar dari Programmer Mode
   */
  void restoreLanguageFromProgrammerMode();

  /**
   * @brief Menampilkan konfirmasi reset progress dengan warning
   * @return true jika user konfirmasi reset, false jika cancel
   */
  bool showResetConfirmation();

  /**
   * @brief Menampilkan konfirmasi clear history dengan warning
   * @return true jika user konfirmasi clear, false jika cancel
   */
  bool showClearHistoryConfirmation();
};

#endif // GAMEENGINE_H