/**
 * @file SFXManager.h
 * @brief Sound Effects Manager untuk Rapid Texter
 * @author Alea Farrel
 * @date 2025
 *
 * Mengelola pemutaran sound effects (SFX) dengan fitur toggle on/off.
 * Menggunakan Windows Multimedia API (winmm) untuk Windows.
 * Menggunakan aplay (ALSA) atau paplay (PulseAudio) untuk Linux.
 */

#ifndef SFXMANAGER_H
#define SFXMANAGER_H

#include <csignal>
#include <string>

/**
 * @class SFXManager
 * @brief Static class untuk mengelola sound effects
 *
 * SFXManager menyediakan:
 * - Toggle SFX on/off dengan shortcut 'S'
 * - Playback true.wav (navigasi valid)
 * - Playback false.wav (input invalid / pagination mentok)
 *
 * Audio files yang digunakan:
 * - assets/true.wav: Dimainkan saat aksi berhasil
 * - assets/false.wav: Dimainkan saat aksi gagal/invalid
 */
class SFXManager {
public:
  /**
   * @brief Play sound untuk aksi berhasil (true.wav)
   *
   * Digunakan saat:
   * - User berhasil berpindah halaman/menu
   * - Pagination berhasil (masih ada halaman)
   */
  static void playTrue();

  /**
   * @brief Play sound untuk aksi gagal (false.wav)
   *
   * Digunakan saat:
   * - User menekan tombol yang tidak valid
   * - Pagination sudah mentok (tidak ada halaman lagi)
   */
  static void playFalse();

  /**
   * @brief Toggle status SFX On/Off
   */
  static void toggle();

  /**
   * @brief Cek apakah SFX sedang aktif
   * @return true jika SFX enabled, false jika disabled
   */
  static bool isEnabled();

  /**
   * @brief Preload audio system untuk menghilangkan delay pada pemutaran
   * pertama
   *
   * Dipanggil saat aplikasi pertama kali dijalankan untuk "warm up"
   * audio subsystem. Ini mencegah delay yang terasa saat
   * user pertama kali memutar suara.
   */
  static void preload();

private:
  static bool sfxEnabled;     ///< Status global SFX (default: true)
  static bool settingsLoaded; ///< Flag: settings sudah di-load dari file

#ifndef _WIN32
  // Linux-specific members
  static std::string
      audioPlayer; ///< Detected audio player command (paplay/aplay)
  static bool audioPlayerDetected; ///< Flag: audio player already detected

  /**
   * @brief Detect available audio player on Linux
   * @return Command string for audio playback (paplay or aplay -q)
   */
  static std::string detectAudioPlayer();

  /**
   * @brief Play audio file asynchronously on Linux
   * @param filename Path to the WAV file to play
   */
  static void playAudioLinux(const char *filename);
#endif
};

#endif // SFXMANAGER_H
