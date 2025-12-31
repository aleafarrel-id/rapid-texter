/**
 * @file SFXManager.cpp
 * @brief Implementasi Sound Effects Manager
 * @author Alea Farrel
 * @date 2025
 *
 * Menggunakan Windows Multimedia API (PlaySound) untuk Windows.
 * Pada Linux, menggunakan aplay (ALSA) atau paplay (PulseAudio).
 */

#include "SFXManager.h"
#include "SettingsManager.h"

#ifdef _WIN32
#include <mmsystem.h>
#include <windows.h>
#else
// Linux includes
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#endif

// ============================================================================
// STATIC MEMBER INITIALIZATION
// ============================================================================

// Default: SFX enabled (will be overwritten by SettingsManager::load())
bool SFXManager::sfxEnabled = true;
bool SFXManager::settingsLoaded = false;

#ifndef _WIN32
// Linux: Audio player command (detected at preload)
std::string SFXManager::audioPlayer = "";
bool SFXManager::audioPlayerDetected = false;

/**
 * @brief Detect available audio player on Linux
 *
 * Checks for paplay (PulseAudio) first, then aplay (ALSA).
 * Returns the command to use for playback.
 */
std::string SFXManager::detectAudioPlayer() {
  // Check if paplay (PulseAudio) is available
  if (system("which paplay > /dev/null 2>&1") == 0) {
    return "paplay";
  }
  // Check if aplay (ALSA) is available
  if (system("which aplay > /dev/null 2>&1") == 0) {
    return "aplay -q"; // -q for quiet mode
  }
  // No audio player found
  return "";
}

/**
 * @brief Play audio file asynchronously on Linux
 *
 * Uses fork() to play audio in background without blocking UI.
 * @param filename Path to the WAV file
 */
void SFXManager::playAudioLinux(const char *filename) {
  if (audioPlayer.empty())
    return;

  pid_t pid = fork();
  if (pid == 0) {
    // Child process: play audio
    // Redirect stdout and stderr to /dev/null
    freopen("/dev/null", "w", stdout);
    freopen("/dev/null", "w", stderr);

    // Build and execute the command
    std::string cmd = audioPlayer + " " + filename;
    execl("/bin/sh", "sh", "-c", cmd.c_str(), (char *)NULL);

    // If execl fails, exit child process
    _exit(1);
  }
  // Parent process: don't wait, let child run in background
  // Clean up zombie processes
  if (pid > 0) {
    // Set SIGCHLD to be ignored to auto-reap zombies
    signal(SIGCHLD, SIG_IGN);
  }
}
#endif

// ============================================================================
// AUDIO PLAYBACK
// ============================================================================

/**
 * @brief Play true.wav (aksi berhasil)
 *
 * Menggunakan SND_ASYNC agar audio tidak memblokir UI.
 * Path relatif dari executable: assets/true.wav
 */
void SFXManager::playTrue() {
  if (!sfxEnabled)
    return;

#ifdef _WIN32
  PlaySound(TEXT("assets\\true.wav"), NULL, SND_FILENAME | SND_ASYNC);
#else
  playAudioLinux("assets/true.wav");
#endif
}

/**
 * @brief Play false.wav (aksi gagal/invalid)
 *
 * Menggunakan SND_ASYNC agar audio tidak memblokir UI.
 * Path relatif dari executable: assets/false.wav
 */
void SFXManager::playFalse() {
  if (!sfxEnabled)
    return;

#ifdef _WIN32
  PlaySound(TEXT("assets\\false.wav"), NULL, SND_FILENAME | SND_ASYNC);
#else
  playAudioLinux("assets/false.wav");
#endif
}

// ============================================================================
// TOGGLE & STATUS
// ============================================================================

/**
 * @brief Toggle SFX on/off
 *
 * Saat SFX di-enable kembali, lakukan preload untuk mencegah delay
 * pada pemutaran suara berikutnya.
 * Status SFX akan disimpan ke settings.json untuk persistensi.
 */
void SFXManager::toggle() {
  sfxEnabled = !sfxEnabled;

  // Simpan ke settings.json agar persisten
  SettingsManager::setSfxEnabled(sfxEnabled);

  // Jika baru di-enable, lakukan warm-up ulang
  if (sfxEnabled) {
    preload();
  }
}

/**
 * @brief Cek status SFX
 * @return true jika enabled
 */
bool SFXManager::isEnabled() {
  // Load settings dari file jika belum
  if (!settingsLoaded) {
    sfxEnabled = SettingsManager::getSfxEnabled();
    settingsLoaded = true;
  }
  return sfxEnabled;
}

// ============================================================================
// AUDIO PRELOAD (WARM-UP)
// ============================================================================

/**
 * @brief Preload audio system untuk menghilangkan delay pertama
 *
 * Teknik: Memutar file audio dengan volume 0 (silent) menggunakan
 * PlaySound dengan flag SND_NOSTOP untuk tidak mengganggu audio lain.
 * Ini memaksa Windows untuk menginisialisasi audio subsystem
 * sehingga pemutaran berikutnya tidak ada delay.
 *
 * Pada Linux, mendeteksi audio player yang tersedia dan melakukan
 * warm-up dengan memutar audio singkat.
 */
void SFXManager::preload() {
#ifdef _WIN32
  // Play true.wav secara silent untuk warm-up audio system
  // SND_NOSTOP: Jangan hentikan audio yang sedang diputar
  // SND_ASYNC: Non-blocking
  PlaySound(TEXT("assets\\true.wav"), NULL,
            SND_FILENAME | SND_ASYNC | SND_NOSTOP);

  // Hentikan segera setelah audio subsystem ter-initialize
  // Ini memastikan suara tidak benar-benar terdengar
  Sleep(10);                // Beri waktu sedikit untuk inisialisasi
  PlaySound(NULL, NULL, 0); // Stop playback
#else
  // Linux: Detect audio player if not done yet
  if (!audioPlayerDetected) {
    audioPlayer = detectAudioPlayer();
    audioPlayerDetected = true;
  }

  // Warm-up: Play audio in background to initialize audio subsystem
  // This reduces delay on first actual playback
  if (!audioPlayer.empty()) {
    pid_t pid = fork();
    if (pid == 0) {
      // Child process: play and immediately kill to warm up
      freopen("/dev/null", "w", stdout);
      freopen("/dev/null", "w", stderr);

      // Play with very low volume or short duration for warm-up
      std::string cmd;
      if (audioPlayer.find("paplay") != std::string::npos) {
        // paplay with very low volume for warm-up
        cmd = "paplay --volume=1 assets/true.wav 2>/dev/null &";
      } else {
        // aplay - just start and kill quickly
        cmd = "timeout 0.01 aplay -q assets/true.wav 2>/dev/null";
      }
      system(cmd.c_str());
      _exit(0);
    }
    // Parent: set to auto-reap zombie processes
    if (pid > 0) {
      signal(SIGCHLD, SIG_IGN);
    }
  }
#endif
}
