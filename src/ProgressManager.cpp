/**
 * @file ProgressManager.cpp
 * @brief Implementasi ProgressManager untuk pengelolaan progress campaign
 * @author Alea Farrel & Team
 * @date 2025
 * 
 * File ini mengimplementasikan class ProgressManager yang bertanggung jawab
 * untuk menyimpan, memuat, dan mengelola progress campaign pemain.
 * 
 * @section features Fitur Utama
 * - Penyimpanan progress dalam format JSON
 * - Tracking terpisah per bahasa (ID, EN)
 * - Unlock system berdasarkan achievement
 * - Cross-platform data directory
 * 
 * @section unlock Sistem Unlock
 * Level unlocking berdasarkan pencapaian di level sebelumnya:
 * - Easy: Unlocked by default
 * - Medium: Unlock setelah Easy dengan 40 WPM, 80% accuracy
 * - Hard: Unlock setelah Medium dengan 60 WPM, 90% accuracy
 * - Programmer: Selalu unlocked (bonus mode)
 * 
 * @section json_format Format JSON
 * File progress.json memiliki struktur:
 * @code{.json}
 * {
 *   "languages": {
 *     "id": {
 *       "easy_unlocked": true,
 *       "medium_unlocked": false,
 *       "hard_unlocked": false,
 *       "programmer_unlocked": true,
 *       "easy_completed": false,
 *       "medium_completed": false,
 *       "hard_completed": false,
 *       "programmer_completed": false,
 *       "hard_completed_ever": false
 *     },
 *     "en": { ... }
 *   }
 * }
 * @endcode
 * 
 * @section storage Lokasi Penyimpanan
 * - Windows: %APPDATA%\\RapidTexter\\progress.json
 * - Linux: ~/.local/share/RapidTexter/progress.json
 */

#include "ProgressManager.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>  // untuk getenv
#include <sys/stat.h>  // untuk stat

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>  // untuk SHGetFolderPath
#include <direct.h>  // untuk _mkdir
#endif

// ============================================================================
// HELPER FUNCTIONS (Static)
// ============================================================================

/**
 * @brief Membuat direktori jika belum ada
 * 
 * Helper function cross-platform untuk memastikan direktori
 * penyimpanan data sudah ada sebelum menulis file.
 * 
 * @param path Path ke direktori yang akan dibuat
 * @return true jika direktori sudah ada atau berhasil dibuat
 * @return false jika gagal membuat direktori
 * 
 * @par Windows Implementation
 * Menggunakan _stat() untuk cek dan _mkdir() untuk buat.
 * 
 * @par Linux Implementation
 * Menggunakan stat() untuk cek dan mkdir() dengan permission 0755.
 */
static bool ensureDirectoryExists(const std::string& path) {
#ifdef _WIN32
    // Windows: gunakan _mkdir
    struct _stat st;
    if (_stat(path.c_str(), &st) != 0) {
        return _mkdir(path.c_str()) == 0;
    }
    return (st.st_mode & _S_IFDIR) != 0;
#else
    // Linux/macOS: gunakan mkdir
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        return mkdir(path.c_str(), 0755) == 0;
    }
    return S_ISDIR(st.st_mode);
#endif
}

/**
 * @brief Mendapatkan path direktori data aplikasi sesuai platform
 * 
 * @return std::string Path ke direktori data dengan trailing separator
 * 
 * @par Windows
 * Menggunakan CSIDL_APPDATA yang mengarah ke:
 * C:\\Users\\{username}\\AppData\\Roaming\\RapidTexter\\
 * 
 * @par Linux/macOS
 * Mengikuti XDG Base Directory Specification:
 * - Jika XDG_DATA_HOME di-set: $XDG_DATA_HOME/RapidTexter/
 * - Jika tidak: ~/.local/share/RapidTexter/
 * 
 * @note Direktori akan otomatis dibuat jika belum ada
 */
static std::string getDataDirectory() {
#ifdef _WIN32
    // Windows: gunakan %APPDATA%
    char appDataPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, appDataPath))) {
        std::string dataDir = std::string(appDataPath) + "\\RapidTexter";
        ensureDirectoryExists(dataDir);
        return dataDir + "\\";
    }
    // Fallback ke current directory
    return "";
#else
    // Linux: gunakan XDG_DATA_HOME atau ~/.local/share
    const char* xdgDataHome = std::getenv("XDG_DATA_HOME");
    std::string baseDir;
    
    if (xdgDataHome && xdgDataHome[0] != '\0') {
        baseDir = std::string(xdgDataHome);
    } else {
        const char* home = std::getenv("HOME");
        if (home) {
            baseDir = std::string(home) + "/.local/share";
        } else {
            // Fallback ke current directory
            return "";
        }
    }
    
    // Buat directory jika belum ada
    ensureDirectoryExists(baseDir);
    std::string dataDir = baseDir + "/RapidTexter";
    ensureDirectoryExists(dataDir);
    return dataDir + "/";
#endif
}

// ============================================================================
// CONSTRUCTOR
// ============================================================================

/**
 * @brief Constructor - Inisialisasi progress manager dan load existing data
 * 
 * Proses inisialisasi:
 * 1. Menentukan path file progress.json
 * 2. Inisialisasi default progress untuk bahasa ID dan EN
 * 3. Load progress yang sudah ada dari file (jika ada)
 * 
 * @note Progress hanya di-track untuk bahasa ID dan EN.
 *       "prog" adalah mode, bukan bahasa terpisah, jadi certification
 *       nya disimpan di bahasa yang dipilih user (ID/EN).
 */
ProgressManager::ProgressManager() : filename(getDataDirectory() + "progress.json") {
    // Initialize default progress HANYA untuk bahasa sebenarnya (id, en)
    // "prog" bukan bahasa melainkan mode (Programmer Mode)
    progressData["id"] = LanguageProgress();
    progressData["en"] = LanguageProgress();
    
    // Load existing progress jika ada
    loadProgress();
}

// ============================================================================
// LOAD PROGRESS (JSON PARSER)
// ============================================================================

/**
 * @brief Memuat progress dari file JSON ke memory
 * 
 * Membaca dan mem-parse file progress.json menggunakan simple
 * line-by-line JSON parser.
 * 
 * @return true jika berhasil mem-parse file
 * @return false jika file tidak ada (akan menggunakan default values)
 * 
 * @par Algoritma Parsing
 * Parser menggunakan state machine dengan currentLanguage untuk tracking:
 * 1. Cari "id", "en", atau "prog" untuk set currentLanguage
 * 2. Parse fields unlocked/completed berdasarkan currentLanguage
 * 3. Simpan ke progressData map
 * 
 * @par Fields yang Di-parse
 * - easy_unlocked, medium_unlocked, hard_unlocked, programmer_unlocked
 * - easy_completed, medium_completed, hard_completed, programmer_completed
 * - hard_completed_ever (untuk Rick Roll tracking)
 * 
 * @note File tidak ada adalah kondisi normal untuk user baru
 */
bool ProgressManager::loadProgress() {
    std::ifstream file(filename);
    if (!file.is_open()) {
        // File tidak ada, gunakan default
        return false;
    }
    
    // Simple JSON parser untuk structure
    std::string line;
    std::string currentLanguage = "";
    
    while (std::getline(file, line)) {
        // Skip whitespace dan brackets
        size_t pos = line.find_first_not_of(" \t\r\n{}[],");
        if (pos == std::string::npos) continue;
        line = line.substr(pos);
        
        // Parse language section
        // Cari "id", "en", atau "prog" untuk menentukan bahasa aktif
        if (line.find("\"id\"") != std::string::npos || 
            line.find("\"en\"") != std::string::npos || 
            line.find("\"prog\"") != std::string::npos) {
            
            if (line.find("\"id\"") != std::string::npos) currentLanguage = "id";
            else if (line.find("\"en\"") != std::string::npos) currentLanguage = "en";
            else if (line.find("\"prog\"") != std::string::npos) currentLanguage = "prog";
            continue;
        }
        
        if (currentLanguage.empty()) continue;
        
        // Parse unlocked status
        if (line.find("\"easy_unlocked\"") != std::string::npos) {
            bool value = line.find("true") != std::string::npos;
            progressData[currentLanguage].unlocked[Difficulty::EASY] = value;
        }
        else if (line.find("\"medium_unlocked\"") != std::string::npos) {
            bool value = line.find("true") != std::string::npos;
            progressData[currentLanguage].unlocked[Difficulty::MEDIUM] = value;
        }
        else if (line.find("\"hard_unlocked\"") != std::string::npos) {
            bool value = line.find("true") != std::string::npos;
            progressData[currentLanguage].unlocked[Difficulty::HARD] = value;
        }
        else if (line.find("\"programmer_unlocked\"") != std::string::npos) {
            bool value = line.find("true") != std::string::npos;
            progressData[currentLanguage].unlocked[Difficulty::PROGRAMMER] = value;
        }
        
        // Parse completed status
        else if (line.find("\"easy_completed\"") != std::string::npos) {
            bool value = line.find("true") != std::string::npos;
            progressData[currentLanguage].completed[Difficulty::EASY] = value;
        }
        else if (line.find("\"medium_completed\"") != std::string::npos) {
            bool value = line.find("true") != std::string::npos;
            progressData[currentLanguage].completed[Difficulty::MEDIUM] = value;
        }
        else if (line.find("\"hard_completed\"") != std::string::npos) {
            bool value = line.find("true") != std::string::npos;
            progressData[currentLanguage].completed[Difficulty::HARD] = value;
        }
        else if (line.find("\"programmer_completed\"") != std::string::npos) {
            bool value = line.find("true") != std::string::npos;
            progressData[currentLanguage].completed[Difficulty::PROGRAMMER] = value;
        }
        
        // Parse hard completed flag (untuk Rick Roll logic)
        else if (line.find("\"hard_completed_ever\"") != std::string::npos) {
            bool value = line.find("true") != std::string::npos;
            progressData[currentLanguage].hardCompletedEver = value;
        }
    }
    
    file.close();
    return true;
}

// ============================================================================
// SAVE PROGRESS (JSON WRITER)
// ============================================================================

/**
 * @brief Menyimpan seluruh progress ke file JSON
 * 
 * Menulis progress untuk semua bahasa ke file dalam format
 * JSON yang pretty-printed dengan indentasi.
 * 
 * @return true jika berhasil menyimpan ke file
 * @return false jika gagal membuka file untuk penulisan
 * 
 * @note Hanya menyimpan progress untuk bahasa ID dan EN.
 *       Programmer certification disimpan di bahasa masing-masing.
 * 
 * @warning File yang sudah ada akan ditimpa
 */
bool ProgressManager::saveProgress() {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to save progress to " << filename << std::endl;
        return false;
    }
    
    file << "{\n";
    file << "  \"languages\": {\n";
    
    // Array bahasa HANYA id dan en (programmer bukan bahasa sebenarnya melainkan mode)
    std::string languages[] = {"id", "en"};
    
    for (int i = 0; i < 2; ++i) {  // Loop hanya 2 kali
        const std::string& lang = languages[i];
        const auto& progress = progressData[lang];
        
        file << "    \"" << lang << "\": {\n";
        
        // Unlocked status
        file << "      \"easy_unlocked\": " << (progress.unlocked.at(Difficulty::EASY) ? "true" : "false") << ",\n";
        file << "      \"medium_unlocked\": " << (progress.unlocked.at(Difficulty::MEDIUM) ? "true" : "false") << ",\n";
        file << "      \"hard_unlocked\": " << (progress.unlocked.at(Difficulty::HARD) ? "true" : "false") << ",\n";
        file << "      \"programmer_unlocked\": " << (progress.unlocked.at(Difficulty::PROGRAMMER) ? "true" : "false") << ",\n";
        
        // Completed status
        file << "      \"easy_completed\": " << (progress.completed.at(Difficulty::EASY) ? "true" : "false") << ",\n";
        file << "      \"medium_completed\": " << (progress.completed.at(Difficulty::MEDIUM) ? "true" : "false") << ",\n";
        file << "      \"hard_completed\": " << (progress.completed.at(Difficulty::HARD) ? "true" : "false") << ",\n";
        file << "      \"programmer_completed\": " << (progress.completed.at(Difficulty::PROGRAMMER) ? "true" : "false") << ",\n";
        
        // Hard completed flag
        file << "      \"hard_completed_ever\": " << (progress.hardCompletedEver ? "true" : "false") << "\n";
        
        file << "    }" << (i < 1 ? "," : "") << "\n";  // i < 1 karena hanya 2 item
    }
    
    file << "  }\n";
    file << "}\n";
    
    file.close();
    return true;
}

// ============================================================================
// RESET PROGRESS
// ============================================================================

/**
 * @brief Reset semua progress ke nilai default
 * 
 * Menghapus semua progress yang tersimpan dan mengembalikan
 * ke kondisi awal (hanya Easy dan Programmer yang unlocked).
 * 
 * @return true jika berhasil reset
 * 
 * @par Proses Reset
 * 1. Re-initialize progressData untuk semua bahasa
 * 2. Hapus file progress.json
 * 3. Simpan file baru dengan default values
 * 
 * @warning Operasi ini tidak dapat di-undo!
 */
bool ProgressManager::resetProgress() {
    // Reset kedua bahasa ke default
    progressData["id"] = LanguageProgress();
    progressData["en"] = LanguageProgress();
    
    // Hapus file lama dan buat baru dengan default values
    std::remove(filename.c_str());
    return saveProgress();
}

// ============================================================================
// GETTERS
// ============================================================================

/**
 * @brief Mendapatkan reference ke progress suatu bahasa
 * 
 * @param language Kode bahasa ("id", "en", "prog")
 * @return LanguageProgress& Reference ke data progress
 * 
 * @note Jika bahasa tidak ada, akan otomatis dibuat entry baru
 */
LanguageProgress& ProgressManager::getLanguageProgress(const std::string& language) {
    return progressData[language];
}

/**
 * @brief Cek apakah difficulty sudah unlocked
 * 
 * @param language Kode bahasa
 * @param difficulty Difficulty yang dicek
 * @return true jika unlocked, false jika locked atau bahasa tidak ada
 */
bool ProgressManager::isUnlocked(const std::string& language, Difficulty difficulty) {
    if (progressData.find(language) == progressData.end()) {
        return false;
    }
    return progressData[language].unlocked[difficulty];
}

/**
 * @brief Cek apakah difficulty sudah completed
 * 
 * @param language Kode bahasa
 * @param difficulty Difficulty yang dicek
 * @return true jika completed, false jika belum atau bahasa tidak ada
 */
bool ProgressManager::isCompleted(const std::string& language, Difficulty difficulty) {
    if (progressData.find(language) == progressData.end()) {
        return false;
    }
    return progressData[language].completed[difficulty];
}

// ============================================================================
// SETTERS
// ============================================================================

/**
 * @brief Set status unlocked untuk difficulty tertentu
 * 
 * @param language Kode bahasa
 * @param difficulty Difficulty yang akan di-set
 * @param unlocked Status unlock (default: true)
 * 
 * @note Tidak otomatis save - panggil saveProgress() untuk persist
 */
void ProgressManager::setUnlocked(const std::string& language, Difficulty difficulty, bool unlocked) {
    progressData[language].unlocked[difficulty] = unlocked;
}

/**
 * @brief Set status completed untuk difficulty tertentu
 * 
 * @param language Kode bahasa
 * @param difficulty Difficulty yang akan di-set
 * @param completed Status completion (default: true)
 * 
 * @note Tidak otomatis save - panggil saveProgress() untuk persist
 */
void ProgressManager::setCompleted(const std::string& language, Difficulty difficulty, bool completed) {
    progressData[language].completed[difficulty] = completed;
}

// ============================================================================
// RICK ROLL TRACKING
// ============================================================================

/**
 * @brief Cek apakah Hard pernah completed sebelumnya di bahasa ini
 * 
 * Digunakan untuk menentukan apakah Rick Roll easter egg
 * harus ditampilkan. Rick Roll hanya muncul sekali per bahasa
 * saat pertama kali completing Hard mode.
 * 
 * @param language Kode bahasa
 * @return true jika Hard pernah completed sebelumnya
 * @return false jika ini pertama kali atau bahasa tidak ada
 */
bool ProgressManager::wasHardCompletedBefore(const std::string& language) {
    if (progressData.find(language) == progressData.end()) {
        return false;
    }
    return progressData[language].hardCompletedEver;
}

/**
 * @brief Mark bahwa Hard sudah pernah completed di bahasa ini
 * 
 * Dipanggil setelah menampilkan Rick Roll untuk memastikan
 * easter egg tidak muncul lagi di bahasa yang sama.
 * 
 * @param language Kode bahasa
 * 
 * @note Tidak otomatis save - panggil saveProgress() untuk persist
 */
void ProgressManager::markHardCompleted(const std::string& language) {
    progressData[language].hardCompletedEver = true;
}

// ============================================================================
// HELPER METHODS
// ============================================================================

/**
 * @brief Konversi string difficulty ke enum Difficulty
 * 
 * @param diffStr String difficulty ("easy", "medium", "hard", "programmer")
 * @return Difficulty Enum yang sesuai, EASY jika tidak dikenali
 */
Difficulty ProgressManager::stringToDifficulty(const std::string& diffStr) {
    if (diffStr == "easy") return Difficulty::EASY;
    if (diffStr == "medium") return Difficulty::MEDIUM;
    if (diffStr == "hard") return Difficulty::HARD;
    if (diffStr == "programmer") return Difficulty::PROGRAMMER;
    return Difficulty::EASY;  // Default fallback
}

/**
 * @brief Konversi enum Difficulty ke string
 * 
 * @param diff Enum Difficulty
 * @return std::string String representation
 */
std::string ProgressManager::difficultyToString(Difficulty diff) {
    switch (diff) {
        case Difficulty::EASY: return "easy";
        case Difficulty::MEDIUM: return "medium";
        case Difficulty::HARD: return "hard";
        case Difficulty::PROGRAMMER: return "programmer";
    }
    return "easy";  // Default fallback
}