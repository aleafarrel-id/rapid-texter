/**
 * @file SFXManager.h
 * @brief Sound Effects Manager untuk Rapid Texter
 * @author Alea Farrel
 * @date 2025
 * 
 * Mengelola pemutaran sound effects (SFX) dengan fitur toggle on/off.
 * Menggunakan Windows Multimedia API (winmm) untuk Windows.
 */

#ifndef SFXMANAGER_H
#define SFXMANAGER_H

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
     * @brief Preload audio system untuk menghilangkan delay pada pemutaran pertama
     * 
     * Dipanggil saat aplikasi pertama kali dijalankan untuk "warm up"
     * audio subsystem Windows. Ini mencegah delay yang terasa saat
     * user pertama kali memutar suara.
     */
    static void preload();

private:
    static bool sfxEnabled;  ///< Status global SFX (default: true)
};

#endif // SFXMANAGER_H
