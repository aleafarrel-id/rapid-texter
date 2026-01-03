/**
 * @file HistoryManager.h
 * @brief Manager untuk menyimpan dan menampilkan history permainan
 * @author Alea Farrel & Team
 * @date 2025
 * 
 * HistoryManager mengelola pencatatan history permainan user dalam format JSON.
 * Mendukung pagination untuk menampilkan history secara bertahap.
 */

#ifndef HISTORYMANAGER_H
#define HISTORYMANAGER_H

#include <string>
#include <vector>
#include <ctime>

/**
 * @struct HistoryEntry
 * @brief Struktur data untuk menyimpan satu entry history permainan
 * 
 * Setiap entry mencatat:
 * - WPM (Words Per Minute)
 * - Accuracy (persentase)
 * - Error count
 * - Difficulty level
 * - Language yang digunakan
 * - Timestamp kapan permainan dilakukan
 */
struct HistoryEntry {
    double wpm;               ///< Words Per Minute
    double accuracy;          ///< Accuracy percentage (0-100)
    int targetWPM;            ///< Target WPM untuk pass level
    int errors;               ///< Jumlah kesalahan
    std::string difficulty;   ///< Difficulty level ("Easy", "Medium", "Hard", "Programmer")
    std::string language;     ///< Bahasa yang digunakan ("ID", "EN", "PROG")
    std::string mode;         ///< Mode permainan ("Manual", "Campaign")
    std::string timestamp;    ///< Waktu permainan (format: DD/MM/YYYY HH:MM:SS)
    
    /**
     * @brief Constructor default
     */
    HistoryEntry() : wpm(0), accuracy(0), targetWPM(0), errors(0) {}
};

/**
 * @class HistoryManager
 * @brief Class untuk mengelola history permainan
 * 
 * Features:
 * - Save entry baru ke history
 * - Load history dari file JSON
 * - Pagination support (untuk tampilan terminal)
 * - Auto-save setiap kali ada entry baru
 */
class HistoryManager {
public:
    /**
     * @brief Constructor - Initialize dan load existing history
     */
    HistoryManager();
    
    /**
     * @brief Menyimpan entry baru ke history
     * @param entry HistoryEntry yang akan disimpan
     * 
     * Entry akan ditambahkan ke list dan langsung di-save ke file.
     * History akan diurutkan dari yang terbaru ke terlama.
     */
    void saveEntry(const HistoryEntry& entry);
    
    /**
     * @brief Load history dari file JSON
     * @return true jika berhasil, false jika file tidak ada atau error
     */
    bool loadHistory();
    
    /**
     * @brief Save history ke file JSON
     * @return true jika berhasil
     */
    bool saveHistory();
    
    /**
     * @brief Mendapatkan entries untuk halaman tertentu
     * @param pageNumber Nomor halaman (1-based)
     * @param pageSize Jumlah entry per halaman (default: 5)
     * @return Vector berisi entries untuk halaman tersebut
     * 
     * Pagination membantu menampilkan history secara bertahap di terminal.
     * Halaman 1 = entry paling baru, dst.
     */
    std::vector<HistoryEntry> getPage(int pageNumber, int pageSize = 5);
    
    /**
     * @brief Menghitung total halaman yang tersedia
     * @param pageSize Jumlah entry per halaman (default: 5)
     * @return Jumlah total halaman
     */
    int getTotalPages(int pageSize = 5);
    
    /**
     * @brief Mendapatkan total jumlah entry
     * @return Jumlah total entry
     */
    int getTotalEntries();
    
    /**
     * @brief Menghapus semua history
     * 
     * Menghapus semua entry dan menyimpan file kosong.
     */
    void clearHistory();

private:
    std::vector<HistoryEntry> entries; ///< List semua entry history
    std::string filename;              ///< Path ke file JSON
    
    /**
     * @brief Helper untuk mendapatkan timestamp saat ini
     * @return String timestamp format DD/MM/YYYY HH:MM:SS
     */
    std::string getCurrentTimestamp();
};

#endif // HISTORYMANAGER_H