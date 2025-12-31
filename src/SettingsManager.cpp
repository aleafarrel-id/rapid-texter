/**
 * @file SettingsManager.cpp
 * @brief Implementasi Settings Manager untuk penyimpanan pengaturan user
 * @author Alea Farrel
 * @date 2025
 *
 * File ini mengimplementasikan class SettingsManager yang bertanggung jawab
 * untuk menyimpan dan memuat pengaturan user dalam format JSON.
 *
 * @section json_format Format JSON
 * File settings.json memiliki struktur:
 * @code{.json}
 * {
 *   "sfx_enabled": true,
 *   "default_duration": 30
 * }
 * @endcode
 *
 * @section storage Lokasi Penyimpanan
 * - Windows: %APPDATA%\RapidTexter\settings.json
 * - Linux: ~/.local/share/RapidTexter/settings.json
 */

#include "SettingsManager.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#include <shlobj.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

// ============================================================================
// STATIC MEMBER INITIALIZATION
// ============================================================================

bool SettingsManager::sfxEnabled = true;   // Default: SFX on
int SettingsManager::defaultDuration = 30; // Default: 30 detik
bool SettingsManager::isLoaded = false;
std::string SettingsManager::filename = "";

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * @brief Mendapatkan path direktori data aplikasi sesuai platform
 *
 * @par Windows
 * Menggunakan CSIDL_APPDATA (%APPDATA%) yang biasanya mengarah ke:
 * C:\Users\{username}\AppData\Roaming\RapidTexter\
 *
 * @par Linux/macOS
 * Mengikuti XDG Base Directory Specification:
 * - Jika XDG_DATA_HOME di-set: $XDG_DATA_HOME/RapidTexter/
 * - Jika tidak: ~/.local/share/RapidTexter/
 *
 * @return std::string Path ke direktori data dengan trailing separator
 */
std::string SettingsManager::getDataDirectory() {
#ifdef _WIN32
  char appDataPath[MAX_PATH];
  if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, appDataPath))) {
    std::string dataDir = std::string(appDataPath) + "\\RapidTexter";
    struct _stat st;
    if (_stat(dataDir.c_str(), &st) != 0) {
      _mkdir(dataDir.c_str());
    }
    return dataDir + "\\";
  }
  return "";
#else
  const char *xdgDataHome = std::getenv("XDG_DATA_HOME");
  std::string baseDir;

  if (xdgDataHome && xdgDataHome[0] != '\0') {
    baseDir = std::string(xdgDataHome);
  } else {
    const char *home = std::getenv("HOME");
    if (home) {
      baseDir = std::string(home) + "/.local/share";
    } else {
      return "";
    }
  }

  struct stat st;
  if (stat(baseDir.c_str(), &st) != 0) {
    mkdir(baseDir.c_str(), 0755);
  }

  std::string dataDir = baseDir + "/RapidTexter";
  if (stat(dataDir.c_str(), &st) != 0) {
    mkdir(dataDir.c_str(), 0755);
  }
  return dataDir + "/";
#endif
}

// ============================================================================
// LOAD SETTINGS
// ============================================================================

/**
 * @brief Memuat settings dari file JSON
 *
 * Parser sederhana yang membaca line-by-line dan mengekstrak nilai
 * dari format JSON yang dihasilkan oleh save().
 *
 * @return true jika berhasil load dari file
 * @return false jika file tidak ada
 */
bool SettingsManager::load() {
  // Set filename jika belum
  if (filename.empty()) {
    filename = getDataDirectory() + "settings.json";
  }

  std::ifstream file(filename);
  if (!file.is_open()) {
    // File tidak ada, gunakan default values
    isLoaded = true;
    return false;
  }

  std::string line;
  while (std::getline(file, line)) {
    // Parse sfx_enabled
    if (line.find("\"sfx_enabled\"") != std::string::npos) {
      if (line.find("true") != std::string::npos) {
        sfxEnabled = true;
      } else if (line.find("false") != std::string::npos) {
        sfxEnabled = false;
      }
    }
    // Parse default_duration
    else if (line.find("\"default_duration\"") != std::string::npos) {
      size_t colonPos = line.find(":");
      if (colonPos != std::string::npos) {
        std::string value = line.substr(colonPos + 1);
        // Trim whitespace dan koma
        value.erase(0, value.find_first_not_of(" \t"));
        if (!value.empty() && (value.back() == ',' || value.back() == '\n' ||
                               value.back() == '\r')) {
          value.pop_back();
        }
        try {
          defaultDuration = std::stoi(value);
        } catch (...) {
          defaultDuration = 30; // Fallback ke default
        }
      }
    }
  }

  file.close();
  isLoaded = true;
  return true;
}

// ============================================================================
// SAVE SETTINGS
// ============================================================================

/**
 * @brief Menyimpan settings ke file JSON
 *
 * Menulis settings dalam format JSON yang pretty-printed.
 *
 * @return true jika berhasil menyimpan
 * @return false jika gagal membuka file
 */
bool SettingsManager::save() {
  // Set filename jika belum
  if (filename.empty()) {
    filename = getDataDirectory() + "settings.json";
  }

  std::ofstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Failed to save settings to " << filename << std::endl;
    return false;
  }

  file << "{\n";
  file << "  \"sfx_enabled\": " << (sfxEnabled ? "true" : "false") << ",\n";
  file << "  \"default_duration\": " << defaultDuration << "\n";
  file << "}\n";

  file.close();
  return true;
}

// ============================================================================
// GETTERS & SETTERS
// ============================================================================

/**
 * @brief Mendapatkan status SFX
 *
 * Akan otomatis load settings jika belum di-load.
 *
 * @return true jika SFX enabled
 */
bool SettingsManager::getSfxEnabled() {
  if (!isLoaded) {
    load();
  }
  return sfxEnabled;
}

/**
 * @brief Mengatur status SFX dan menyimpan ke file
 *
 * @param enabled Status SFX yang baru
 */
void SettingsManager::setSfxEnabled(bool enabled) {
  sfxEnabled = enabled;
  save();
}

/**
 * @brief Mendapatkan durasi default
 *
 * Akan otomatis load settings jika belum di-load.
 *
 * @return Durasi dalam detik (-1 untuk unlimited)
 */
int SettingsManager::getDefaultDuration() {
  if (!isLoaded) {
    load();
  }
  return defaultDuration;
}

/**
 * @brief Mengatur durasi default dan menyimpan ke file
 *
 * @param duration Durasi dalam detik (-1 untuk unlimited)
 */
void SettingsManager::setDefaultDuration(int duration) {
  defaultDuration = duration;
  save();
}
