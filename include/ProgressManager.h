/**
 * @file ProgressManager.h
 * @brief Progress persistence system dengan tracking per-bahasa
 * @author Alea Farrel
 * @date 2025
 * 
 * Mengelola penyimpanan dan pembacaan progress campaign dari file JSON.
 * Progress di-track terpisah untuk setiap bahasa (ID, EN, PROG).
 */

#ifndef PROGRESSMANAGER_H
#define PROGRESSMANAGER_H

#include "TextProvider.h" // Untuk enum Difficulty
#include <string>
#include <map>

/**
 * @struct LanguageProgress
 * @brief Struktur data untuk menyimpan progress per bahasa
 * 
 * Setiap bahasa (ID/EN/PROG) memiliki tracking terpisah untuk:
 * - Status unlock setiap difficulty
 * - Status completion setiap difficulty
 * - Flag khusus untuk hard completion (trigger Rick Roll)
 */
struct LanguageProgress {
    std::map<Difficulty, bool> unlocked;     ///< Map difficulty -> apakah sudah unlocked
    std::map<Difficulty, bool> completed;    ///< Map difficulty -> apakah sudah completed
    bool hardCompletedEver;                  ///< Flag untuk tracking hard completion (Rick Roll)
    
    /**
     * @brief Constructor dengan default values
     */
    LanguageProgress() : hardCompletedEver(false) {
        // Default: Easy dan Programmer unlocked, rest locked
        unlocked[Difficulty::EASY] = true;
        unlocked[Difficulty::MEDIUM] = false;
        unlocked[Difficulty::HARD] = false;
        unlocked[Difficulty::PROGRAMMER] = true; // Bonus mode, always unlocked
        
        // Default: Semua belum completed
        completed[Difficulty::EASY] = false;
        completed[Difficulty::MEDIUM] = false;
        completed[Difficulty::HARD] = false;
        completed[Difficulty::PROGRAMMER] = false;
    }
};

/**
 * @class ProgressManager
 * @brief Class untuk mengelola persistence progress ke file JSON
 * 
 * Features:
 * - Save/Load progress ke/dari progress.json
 * - Tracking terpisah per bahasa (ID, EN, PROG)
 * - Unlock system berdasarkan achievement
 * - Reset progress (dengan konfirmasi)
 */
class ProgressManager {
public:
    /**
     * @brief Constructor - Load progress dari file jika ada
     */
    ProgressManager();
    
    /**
     * @brief Load progress dari file JSON
     * @return true jika berhasil, false jika file tidak ada (akan create default)
     */
    bool loadProgress();
    
    /**
     * @brief Save progress ke file JSON
     * @return true jika berhasil, false jika gagal write
     */
    bool saveProgress();
    
    /**
     * @brief Reset semua progress ke default (delete file)
     * @return true jika berhasil
     */
    bool resetProgress();
    
    /**
     * @brief Get reference ke progress data bahasa tertentu
     * @param language Kode bahasa ("id", "en", "prog")
     * @return Reference ke LanguageProgress untuk bahasa tersebut
     */
    LanguageProgress& getLanguageProgress(const std::string& language);
    
    /**
     * @brief Check apakah difficulty sudah unlocked untuk bahasa tertentu
     * @param language Kode bahasa
     * @param difficulty Difficulty yang dicek
     * @return true jika unlocked
     */
    bool isUnlocked(const std::string& language, Difficulty difficulty);
    
    /**
     * @brief Check apakah difficulty sudah completed untuk bahasa tertentu
     * @param language Kode bahasa
     * @param difficulty Difficulty yang dicek
     * @return true jika completed
     */
    bool isCompleted(const std::string& language, Difficulty difficulty);
    
    /**
     * @brief Set status unlocked untuk difficulty tertentu
     * @param language Kode bahasa
     * @param difficulty Difficulty yang di-unlock
     * @param unlocked Status unlock (default: true)
     */
    void setUnlocked(const std::string& language, Difficulty difficulty, bool unlocked = true);
    
    /**
     * @brief Set status completed untuk difficulty tertentu
     * @param language Kode bahasa
     * @param difficulty Difficulty yang di-complete
     * @param completed Status completion (default: true)
     */
    void setCompleted(const std::string& language, Difficulty difficulty, bool completed = true);
    
    /**
     * @brief Check apakah hard pernah completed di bahasa ini (untuk Rick Roll logic)
     * @param language Kode bahasa
     * @return true jika hard pernah completed sebelumnya
     */
    bool wasHardCompletedBefore(const std::string& language);
    
    /**
     * @brief Mark hard sebagai completed di bahasa ini (untuk Rick Roll logic)
     * @param language Kode bahasa
     */
    void markHardCompleted(const std::string& language);

private:
    std::map<std::string, LanguageProgress> progressData; ///< Map language -> progress
    std::string filename; ///< Path ke file JSON
    
    /**
     * @brief Helper untuk parsing difficulty string ke enum
     * @param diffStr String difficulty ("easy", "medium", "hard", "programmer")
     * @return Difficulty enum
     */
    Difficulty stringToDifficulty(const std::string& diffStr);
    
    /**
     * @brief Helper untuk convert difficulty enum ke string
     * @param diff Difficulty enum
     * @return String difficulty
     */
    std::string difficultyToString(Difficulty diff);
};

#endif // PROGRESSMANAGER_H