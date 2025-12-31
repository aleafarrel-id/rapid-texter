/**
 * @file SettingsManager.h
 * @brief Settings Manager untuk Rapid Texter
 * @author Alea Farrel
 * @date 2025
 *
 * Mengelola penyimpanan dan pemuatan pengaturan user dalam format JSON.
 * Settings disimpan di direktori data aplikasi:
 * - Windows: %APPDATA%\RapidTexter\settings.json
 * - Linux: ~/.local/share/RapidTexter/settings.json
 */

#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <string>

/**
 * @class SettingsManager
 * @brief Static class untuk mengelola pengaturan user yang persisten
 *
 * SettingsManager menyediakan:
 * - Load/save settings dari/ke file JSON
 * - Getter/setter untuk setiap setting
 * - Auto-create file jika tidak ada
 *
 * Settings yang disimpan:
 * - sfx_enabled: Status SFX (true/false)
 * - default_duration: Durasi default dalam detik (-1 untuk unlimited)
 */
class SettingsManager {
public:
  /**
   * @brief Memuat settings dari file JSON
   *
   * Jika file tidak ada, akan menggunakan nilai default:
   * - sfx_enabled: true
   * - default_duration: 30
   *
   * @return true jika berhasil load dari file
   * @return false jika file tidak ada (menggunakan default)
   */
  static bool load();

  /**
   * @brief Menyimpan settings ke file JSON
   *
   * @return true jika berhasil menyimpan
   * @return false jika gagal membuka file
   */
  static bool save();

  /**
   * @brief Mendapatkan status SFX
   * @return true jika SFX enabled
   */
  static bool getSfxEnabled();

  /**
   * @brief Mengatur status SFX dan langsung menyimpan ke file
   * @param enabled Status SFX yang baru
   */
  static void setSfxEnabled(bool enabled);

  /**
   * @brief Mendapatkan durasi default
   * @return Durasi dalam detik (-1 untuk unlimited)
   */
  static int getDefaultDuration();

  /**
   * @brief Mengatur durasi default dan langsung menyimpan ke file
   * @param duration Durasi dalam detik (-1 untuk unlimited)
   */
  static void setDefaultDuration(int duration);

private:
  static bool sfxEnabled;      ///< Status SFX (default: true)
  static int defaultDuration;  ///< Durasi default (default: 30)
  static bool isLoaded;        ///< Flag: settings sudah di-load
  static std::string filename; ///< Path ke file settings.json

  /**
   * @brief Mendapatkan path direktori data aplikasi
   * @return Path dengan trailing separator
   */
  static std::string getDataDirectory();
};

#endif // SETTINGSMANAGER_H
