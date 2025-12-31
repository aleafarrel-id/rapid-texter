/**
 * @file SFXManager.cpp
 * @brief Implementasi Sound Effects Manager
 * @author Alea Farrel
 * @date 2025
 * 
 * Menggunakan Windows Multimedia API (PlaySound) untuk Windows.
 * Pada Linux, fungsi audio kosong (silent fallback).
 */

#include "SFXManager.h"

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#endif

// ============================================================================
// STATIC MEMBER INITIALIZATION
// ============================================================================

// Default: SFX enabled
bool SFXManager::sfxEnabled = true;

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
    if (!sfxEnabled) return;
    
#ifdef _WIN32
    PlaySound(TEXT("assets\\true.wav"), NULL, SND_FILENAME | SND_ASYNC);
#endif
    // Linux: silent (no-op)
}

/**
 * @brief Play false.wav (aksi gagal/invalid)
 * 
 * Menggunakan SND_ASYNC agar audio tidak memblokir UI.
 * Path relatif dari executable: assets/false.wav
 */
void SFXManager::playFalse() {
    if (!sfxEnabled) return;
    
#ifdef _WIN32
    PlaySound(TEXT("assets\\false.wav"), NULL, SND_FILENAME | SND_ASYNC);
#endif
    // Linux: silent (no-op)
}

// ============================================================================
// TOGGLE & STATUS
// ============================================================================

/**
 * @brief Toggle SFX on/off
 * 
 * Saat SFX di-enable kembali, lakukan preload untuk mencegah delay
 * pada pemutaran suara berikutnya.
 */
void SFXManager::toggle() {
    sfxEnabled = !sfxEnabled;
    
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
 */
void SFXManager::preload() {
#ifdef _WIN32
    // Play true.wav secara silent untuk warm-up audio system
    // SND_NOSTOP: Jangan hentikan audio yang sedang diputar
    // SND_ASYNC: Non-blocking
    PlaySound(TEXT("assets\\true.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_NOSTOP);
    
    // Hentikan segera setelah audio subsystem ter-initialize
    // Ini memastikan suara tidak benar-benar terdengar
    Sleep(10); // Beri waktu sedikit untuk inisialisasi
    PlaySound(NULL, NULL, 0); // Stop playback
#endif
}
