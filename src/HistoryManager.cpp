/**
 * @file HistoryManager.cpp
 * @brief Implementasi HistoryManager untuk pengelolaan history permainan
 * @author Alea Farrel & Team
 * @date 2025
 * 
 * File ini mengimplementasikan class HistoryManager yang bertanggung jawab
 * untuk menyimpan, memuat, dan mengelola riwayat permainan pengguna.
 * 
 * @section features Fitur Utama
 * - Penyimpanan history dalam format JSON
 * - Cross-platform data directory (Windows: %APPDATA%, Linux/Mac: XDG_DATA_HOME)
 * - Pagination untuk menampilkan history secara bertahap
 * - Auto-timestamp untuk setiap entry
 * 
 * @section json_format Format JSON
 * File history.json memiliki struktur:
 * @code{.json}
 * {
 *   "entries": [
 *     {
 *       "wpm": 45.5,
 *       "accuracy": 95.2,
 *       "errors": 3,
 *       "difficulty": "Medium",
 *       "language": "ID",
 *       "mode": "Campaign",
 *       "timestamp": "30/12/2025 17:30:00"
 *     }
 *   ]
 * }
 * @endcode
 * 
 * @section storage Lokasi Penyimpanan
 * - Windows: %APPDATA%\\RapidTexter\\history.json
 * - Linux: $XDG_DATA_HOME/RapidTexter/history.json atau ~/.local/share/RapidTexter/history.json
 * - macOS: ~/.local/share/RapidTexter/history.json
 */

#include "HistoryManager.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#include <direct.h>
#else
#include <unistd.h>
#endif

// ============================================================================
// HELPER FUNCTIONS (Static/Internal)
// ============================================================================

/**
 * @brief Mendapatkan path direktori data aplikasi sesuai platform
 * 
 * Fungsi ini menentukan lokasi penyimpanan data yang sesuai dengan
 * konvensi masing-masing sistem operasi:
 * 
 * @par Windows
 * Menggunakan CSIDL_APPDATA (%APPDATA%) yang biasanya mengarah ke:
 * C:\\Users\\{username}\\AppData\\Roaming\\RapidTexter\\
 * 
 * @par Linux/macOS
 * Mengikuti XDG Base Directory Specification:
 * - Jika XDG_DATA_HOME di-set: $XDG_DATA_HOME/RapidTexter/
 * - Jika tidak: ~/.local/share/RapidTexter/
 * 
 * @note Fungsi ini akan otomatis membuat direktori jika belum ada
 * 
 * @return std::string Path ke direktori data dengan trailing separator,
 *         atau string kosong jika gagal mendapatkan path
 * 
 * @warning Fungsi ini bersifat static dan hanya digunakan internal
 */
static std::string getDataDirectory() {
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
    const char* xdgDataHome = std::getenv("XDG_DATA_HOME");
    std::string baseDir;
    
    if (xdgDataHome && xdgDataHome[0] != '\0') {
        baseDir = std::string(xdgDataHome);
    } else {
        const char* home = std::getenv("HOME");
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

/**
 * @brief Melakukan escape pada karakter khusus untuk format JSON
 * 
 * Mengkonversi karakter-karakter yang memiliki makna khusus dalam JSON
 * menjadi escape sequence yang valid. Karakter yang di-escape:
 * - " (double quote) -> \"
 * - \\ (backslash) -> \\\\
 * - \\n (newline) -> \\n
 * - \\r (carriage return) -> \\r
 * - \\t (tab) -> \\t
 * 
 * @param str String input yang akan di-escape
 * @return std::string String yang sudah di-escape, aman untuk disimpan dalam JSON
 * 
 * @par Contoh Penggunaan
 * @code
 * std::string raw = "Hello \"World\"";
 * std::string escaped = escapeJsonString(raw);
 * // escaped = "Hello \\\"World\\\""
 * @endcode
 * 
 * @note Fungsi ini bersifat static dan hanya digunakan internal oleh saveHistory()
 */
static std::string escapeJsonString(const std::string& str) {
    std::string escaped;
    for (char c : str) {
        if (c == '"') escaped += "\\\"";
        else if (c == '\\') escaped += "\\\\";
        else if (c == '\n') escaped += "\\n";
        else if (c == '\r') escaped += "\\r";
        else if (c == '\t') escaped += "\\t";
        else escaped += c;
    }
    return escaped;
}

// ============================================================================
// CONSTRUCTOR
// ============================================================================

/**
 * @brief Constructor - Inisialisasi HistoryManager dan load history existing
 * 
 * Constructor akan:
 * 1. Menentukan path file history.json berdasarkan platform
 * 2. Memuat history yang sudah ada dari file (jika ada)
 * 
 * @note History akan otomatis di-load saat objek dibuat
 * @see loadHistory()
 */
HistoryManager::HistoryManager() : filename(getDataDirectory() + "history.json") {
    loadHistory();
}

// ============================================================================
// TIMESTAMP HELPER
// ============================================================================

/**
 * @brief Mendapatkan timestamp saat ini dalam format DD/MM/YYYY HH:MM:SS
 * 
 * Menggunakan waktu lokal sistem untuk menghasilkan timestamp yang
 * human-readable untuk setiap entry history.
 * 
 * @return std::string Timestamp dalam format "DD/MM/YYYY HH:MM:SS"
 * 
 * @par Format Output
 * - DD: Tanggal (01-31)
 * - MM: Bulan (01-12)
 * - YYYY: Tahun (4 digit)
 * - HH: Jam (00-23)
 * - MM: Menit (00-59)
 * - SS: Detik (00-59)
 * 
 * @par Contoh Output
 * "30/12/2025 17:30:45"
 * 
 * @note Fungsi ini menggunakan std::localtime() yang bersifat thread-unsafe.
 *       Untuk aplikasi multi-threaded, pertimbangkan menggunakan localtime_r() atau localtime_s()
 */
std::string HistoryManager::getCurrentTimestamp() {
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);
    
    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", localTime);
    return std::string(buffer);
}

// ============================================================================
// SAVE ENTRY
// ============================================================================

/**
 * @brief Menyimpan entry permainan baru ke history
 * 
 * Menambahkan entry baru ke awal list history (urutan terbaru di atas)
 * dan langsung menyimpan ke file. Timestamp akan otomatis di-set jika kosong.
 * 
 * @param entry HistoryEntry yang berisi data permainan yang akan disimpan
 * 
 * @par Proses Penyimpanan
 * 1. Membuat copy dari entry untuk modifikasi
 * 2. Jika timestamp kosong, set dengan waktu saat ini
 * 3. Insert entry ke awal vector (index 0)
 * 4. Save seluruh history ke file JSON
 * 
 * @par Urutan Entry
 * Entry baru selalu dimasukkan di awal list, sehingga entry terbaru
 * akan muncul pertama saat ditampilkan (halaman 1, baris pertama).
 * 
 * @note Fungsi ini akan otomatis memanggil saveHistory() setelah menambahkan entry
 * 
 * @see HistoryEntry
 * @see saveHistory()
 * @see getCurrentTimestamp()
 */
void HistoryManager::saveEntry(const HistoryEntry& entry) {
    HistoryEntry entryToSave = entry;  
    
    // Auto-set timestamp jika kosong  
    if (entryToSave.timestamp.empty()) {  // Jika timestamp belum di-set
        entryToSave.timestamp = getCurrentTimestamp();  // Set timestamp saat ini
    }  
    
    // Tambahkan entry baru ke awal list (paling baru di atas)
    entries.insert(entries.begin(), entryToSave);
    
    // Save ke file
    saveHistory();
}

// ============================================================================
// LOAD HISTORY (JSON PARSER)
// ============================================================================

/**
 * @brief Memuat history dari file JSON ke memory
 * 
 * Membaca dan mem-parse file history.json menggunakan simple line-by-line
 * JSON parser. Parser ini dirancang khusus untuk format JSON yang dihasilkan
 * oleh saveHistory().
 * 
 * @return true jika file berhasil dibaca dan di-parse
 * @return false jika file tidak ada atau tidak bisa dibuka
 * 
 * @par Algoritma Parsing
 * Parser menggunakan state machine dengan 2 flag:
 * - inEntriesArray: true jika sudah memasuki array "entries"
 * - inEntry: true jika sedang mem-parse objek entry individual
 * 
 * Langkah parsing:
 * 1. Cari baris yang mengandung '"entries"' dan '[' -> masuk array
 * 2. Cari '{' setelah masuk array -> mulai parse entry baru
 * 3. Parse setiap field (wpm, accuracy, errors, difficulty, language, mode, timestamp)
 * 4. Cari '}' -> selesai parse entry, simpan ke vector
 * 5. Cari ']' -> keluar dari array
 * 
 * @par Penanganan Root Object
 * Parser secara eksplisit mengabaikan '{' dan '}' dari root JSON object
 * dengan hanya memproses entry saat sudah berada di dalam array "entries".
 * Ini mencegah pembuatan entry kosong dari struktur root object.
 * 
 * @par Field yang Di-parse
 * | Field      | Tipe   | Parser Function |
 * |------------|--------|-----------------|
 * | wpm        | double | std::atof()     |
 * | accuracy   | double | std::atof()     |
 * | errors     | int    | std::atoi()     |
 * | difficulty | string | quote extraction|
 * | language   | string | quote extraction|
 * | mode       | string | quote extraction|
 * | timestamp  | string | quote extraction|
 * 
 * @warning Parser ini tidak menangani nested objects atau arrays di dalam entry.
 *          Hanya kompatibel dengan format yang dihasilkan oleh saveHistory().
 * 
 * @note History yang sudah ada akan di-clear sebelum loading
 * 
 * @see saveHistory()
 */
bool HistoryManager::loadHistory() {
    std::ifstream file(filename);
    if (!file.is_open()) {
        // File tidak ada, history kosong
        return false;
    }
    
    entries.clear();
    
    // Simple JSON parser untuk array of objects
    std::string line;
    HistoryEntry currentEntry;
    bool inEntry = false;
    bool inEntriesArray = false;  // Track apakah sudah di dalam array "entries"
    
    while (std::getline(file, line)) {
        // Trim whitespace
        size_t pos = line.find_first_not_of(" \t\r\n");
        if (pos == std::string::npos) continue;
        line = line.substr(pos);
        
        // Detect start of entries array
        if (line.find("\"entries\"") != std::string::npos && line.find("[") != std::string::npos) {
            inEntriesArray = true;
            continue;
        }
        
        // Detect end of entries array
        if (inEntriesArray && line.find("]") != std::string::npos) {
            inEntriesArray = false;
            continue;
        }
        
        // Detect start of entry object
        // HANYA proses { jika sudah di dalam array "entries"
        if (inEntriesArray && line.find("{") != std::string::npos && !inEntry) {
            inEntry = true;
            currentEntry = HistoryEntry();
            continue;
        }
        
        // Detect end of entry object
        if (line.find("}") != std::string::npos && inEntry) {
            entries.push_back(currentEntry);
            inEntry = false;
            continue;
        }
        
        if (!inEntry) continue;
        
        // ================================================================
        // FIELD PARSING
        // Setiap field di-parse dengan mencari key, lalu mengekstrak value
        // ================================================================
        
        // Parse "wpm" field (numeric double)
        if (line.find("\"wpm\"") != std::string::npos) {
            size_t colonPos = line.find(":");
            if (colonPos != std::string::npos) {
                std::string value = line.substr(colonPos + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                if (!value.empty() && value.back() == ',') value.pop_back();
                currentEntry.wpm = std::atof(value.c_str());
            }
        }
        // Parse "accuracy" field (numeric double)
        else if (line.find("\"accuracy\"") != std::string::npos) {
            size_t colonPos = line.find(":");
            if (colonPos != std::string::npos) {
                std::string value = line.substr(colonPos + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                if (!value.empty() && value.back() == ',') value.pop_back();
                currentEntry.accuracy = std::atof(value.c_str());
            }
        }
        // Parse "targetWPM" field (numeric int)
        else if (line.find("\"targetWPM\"") != std::string::npos) {
            size_t colonPos = line.find(":");
            if (colonPos != std::string::npos) {
                std::string value = line.substr(colonPos + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                if (!value.empty() && value.back() == ',') value.pop_back();
                currentEntry.targetWPM = std::atoi(value.c_str());
            }
        }
        // Parse "errors" field (numeric int)
        else if (line.find("\"errors\"") != std::string::npos) {
            size_t colonPos = line.find(":");
            if (colonPos != std::string::npos) {
                std::string value = line.substr(colonPos + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                if (!value.empty() && value.back() == ',') value.pop_back();
                currentEntry.errors = std::atoi(value.c_str());
            }
        }
        // Parse "difficulty" field (string with quotes)
        else if (line.find("\"difficulty\"") != std::string::npos) {
            size_t colonPos = line.find(":");
            if (colonPos != std::string::npos) {
                std::string value = line.substr(colonPos + 1);
                size_t firstQuote = value.find("\"");
                size_t lastQuote = value.rfind("\"");
                if (firstQuote != std::string::npos && lastQuote != std::string::npos && firstQuote < lastQuote) {
                    currentEntry.difficulty = value.substr(firstQuote + 1, lastQuote - firstQuote - 1);
                }
            }
        }
        // Parse "language" field (string with quotes)
        else if (line.find("\"language\"") != std::string::npos) {
            size_t colonPos = line.find(":");
            if (colonPos != std::string::npos) {
                std::string value = line.substr(colonPos + 1);
                size_t firstQuote = value.find("\"");
                size_t lastQuote = value.rfind("\"");
                if (firstQuote != std::string::npos && lastQuote != std::string::npos && firstQuote < lastQuote) {
                    currentEntry.language = value.substr(firstQuote + 1, lastQuote - firstQuote - 1);
                }
            }
        }
        // Parse "mode" field (string with quotes)
        else if (line.find("\"mode\"") != std::string::npos) {
            size_t colonPos = line.find(":");
            if (colonPos != std::string::npos) {
                std::string value = line.substr(colonPos + 1);
                size_t firstQuote = value.find("\"");
                size_t lastQuote = value.rfind("\"");
                if (firstQuote != std::string::npos && lastQuote != std::string::npos && firstQuote < lastQuote) {
                    currentEntry.mode = value.substr(firstQuote + 1, lastQuote - firstQuote - 1);
                }
            }
        }
        // Parse "timestamp" field (string with quotes)
        else if (line.find("\"timestamp\"") != std::string::npos) {
            size_t colonPos = line.find(":");
            if (colonPos != std::string::npos) {
                std::string value = line.substr(colonPos + 1);
                size_t firstQuote = value.find("\"");
                size_t lastQuote = value.rfind("\"");
                if (firstQuote != std::string::npos && lastQuote != std::string::npos && firstQuote < lastQuote) {
                    currentEntry.timestamp = value.substr(firstQuote + 1, lastQuote - firstQuote - 1);
                }
            }
        }
    }
    
    file.close();
    return true;
}

// ============================================================================
// SAVE HISTORY (JSON WRITER)
// ============================================================================

/**
 * @brief Menyimpan seluruh history ke file JSON
 * 
 * Menulis semua entry yang ada di memory ke file history.json dalam
 * format JSON yang pretty-printed (dengan indentasi).
 * 
 * @return true jika berhasil menyimpan ke file
 * @return false jika gagal membuka file untuk penulisan
 * 
 * @par Format Output
 * @code{.json}
 * {
 *   "entries": [
 *     {
 *       "wpm": 45.5,
 *       "accuracy": 95.2,
 *       "errors": 3,
 *       "difficulty": "Medium",
 *       "language": "ID",
 *       "mode": "Campaign",
 *       "timestamp": "30/12/2025 17:30:00"
 *     },
 *     ...
 *   ]
 * }
 * @endcode
 * 
 * @par Indentasi
 * - Root object: 0 spaces
 * - "entries" key: 2 spaces
 * - Entry object: 4 spaces
 * - Entry fields: 6 spaces
 * 
 * @note String values akan di-escape menggunakan escapeJsonString()
 *       untuk menghindari karakter yang merusak format JSON
 * 
 * @warning Fungsi ini akan menimpa file yang sudah ada
 * 
 * @see escapeJsonString()
 * @see loadHistory()
 */
bool HistoryManager::saveHistory() {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to save history to " << filename << std::endl;
        return false;
    }
    
    file << "{\n";
    file << "  \"entries\": [\n";
    
    for (size_t i = 0; i < entries.size(); ++i) {
        const auto& entry = entries[i];
        
        file << "    {\n";
        file << "      \"wpm\": " << entry.wpm << ",\n";
        file << "      \"accuracy\": " << entry.accuracy << ",\n";
        file << "      \"targetWPM\": " << entry.targetWPM << ",\n";
        file << "      \"errors\": " << entry.errors << ",\n";
        file << "      \"difficulty\": \"" << escapeJsonString(entry.difficulty) << "\",\n";
        file << "      \"language\": \"" << escapeJsonString(entry.language) << "\",\n";
        file << "      \"mode\": \"" << escapeJsonString(entry.mode) << "\",\n";
        file << "      \"timestamp\": \"" << escapeJsonString(entry.timestamp) << "\"\n";
        file << "    }" << (i < entries.size() - 1 ? "," : "") << "\n";
    }
    
    file << "  ]\n";
    file << "}\n";
    
    file.close();
    return true;
}

// ============================================================================
// PAGINATION
// ============================================================================

/**
 * @brief Mendapatkan entries untuk halaman tertentu (pagination)
 * 
 * Mengambil subset dari entries untuk ditampilkan dalam satu halaman.
 * Pagination berguna untuk menampilkan history di terminal dengan
 * ukuran layar terbatas.
 * 
 * @param pageNumber Nomor halaman yang diminta (1-indexed, halaman pertama = 1)
 * @param pageSize Jumlah entry maksimal per halaman (default: 5)
 * 
 * @return std::vector<HistoryEntry> Vector berisi entries untuk halaman tersebut.
 *         Akan kosong jika:
 *         - pageNumber < 1
 *         - entries kosong
 *         - pageNumber melebihi total halaman
 * 
 * @par Contoh Penggunaan
 * @code
 * // Asumsi ada 12 entries, pageSize = 5
 * auto page1 = getPage(1, 5);  // entries[0..4] - 5 entries
 * auto page2 = getPage(2, 5);  // entries[5..9] - 5 entries
 * auto page3 = getPage(3, 5);  // entries[10..11] - 2 entries
 * auto page4 = getPage(4, 5);  // empty vector (melebihi total)
 * @endcode
 * 
 * @par Kalkulasi Index
 * - startIndex = (pageNumber - 1) * pageSize
 * - endIndex = min(startIndex + pageSize, entries.size())
 * 
 * @see getTotalPages()
 */
std::vector<HistoryEntry> HistoryManager::getPage(int pageNumber, int pageSize) {
    std::vector<HistoryEntry> result;
    
    if (pageNumber < 1 || entries.empty()) {
        return result;
    }
    
    int startIndex = (pageNumber - 1) * pageSize;
    int endIndex = startIndex + pageSize;
    
    if (startIndex >= (int)entries.size()) {
        return result;
    }
    
    if (endIndex > (int)entries.size()) {
        endIndex = entries.size();
    }
    
    for (int i = startIndex; i < endIndex; ++i) {
        result.push_back(entries[i]);
    }
    
    return result;
}

/**
 * @brief Menghitung total halaman yang tersedia
 * 
 * Menghitung berapa halaman yang dibutuhkan untuk menampilkan
 * seluruh entries dengan ukuran halaman tertentu.
 * 
 * @param pageSize Jumlah entry per halaman (default: 5)
 * 
 * @return int Total halaman yang tersedia.
 *         Mengembalikan 0 jika entries kosong.
 * 
 * @par Formula
 * totalPages = ceil(entries.size() / pageSize)
 * 
 * @par Contoh
 * - 12 entries, pageSize 5 -> 3 halaman (5+5+2)
 * - 10 entries, pageSize 5 -> 2 halaman (5+5)
 * - 0 entries -> 0 halaman
 * 
 * @see getPage()
 */
int HistoryManager::getTotalPages(int pageSize) {
    if (entries.empty()) return 0;
    return (int)std::ceil((double)entries.size() / pageSize);
}

/**
 * @brief Mendapatkan total jumlah entry dalam history
 * 
 * @return int Jumlah total entry yang tersimpan dalam history
 * 
 * @note Fungsi ini mengembalikan ukuran vector entries di memory,
 *       yang sudah disinkronkan dengan file setelah load/save
 */
int HistoryManager::getTotalEntries() {
    return entries.size();
}

// ============================================================================
// CLEAR HISTORY
// ============================================================================

/**
 * @brief Menghapus seluruh history
 * 
 * Mengosongkan vector entries di memory dan menyimpan file JSON kosong.
 * Setelah operasi ini, file history.json akan berisi array entries kosong.
 * 
 * @par Hasil File Setelah Clear
 * @code{.json}
 * {
 *   "entries": [
 *   ]
 * }
 * @endcode
 * 
 * @note Operasi ini tidak dapat di-undo. Seluruh history akan hilang permanen.
 * 
 * @warning Pastikan untuk meminta konfirmasi dari user sebelum memanggil fungsi ini
 * 
 * @see saveHistory()
 */
void HistoryManager::clearHistory() {
    entries.clear();
    saveHistory();
}