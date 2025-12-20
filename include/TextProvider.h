/**
 * @file TextProvider.h
 * @brief Provider untuk database kata-kata berdasarkan bahasa dan kesulitan
 * @author Alea Farrel
 * @date 2025
 * 
 * TextProvider mengelola loading dan filtering kata-kata dari file eksternal
 * berdasarkan bahasa dan tingkat kesulitan.
 */

#ifndef TEXTPROVIDER_H
#define TEXTPROVIDER_H

#include <string>
#include <vector>
#include <map>

/**
 * @enum Difficulty
 * @brief Tingkat kesulitan yang tersedia
 * 
 * Difficulty menentukan filter panjang kata:
 * - EASY: Kata pendek (≤6 karakter)
 * - MEDIUM: Kata sedang (≤10 karakter)
 * - HARD: Kata panjang (≤14 karakter)
 * - PROGRAMMER: Semua kata/sintaks (tanpa filter panjang)
 */
enum class Difficulty {
    EASY,           ///< Kata pendek untuk pemula
    MEDIUM,         ///< Kata sedang untuk intermediate
    HARD,           ///< Kata panjang untuk advanced
    PROGRAMMER      ///< Sintaks koding (tanpa filter panjang)
};

/**
 * @class TextProvider
 * @brief Class untuk mengelola database kata-kata
 * 
 * Features:
 * - Load kata dari file teks
 * - Filter kata berdasarkan kesulitan
 * - Random selection
 * - Multi-language support (ID, EN, PROG)
 */
class TextProvider {
public:
    /**
     * @brief Constructor - Initialize random seed
     */
    TextProvider();
    
    /**
     * @brief Load kata-kata dari file ke dalam memory
     * @param language Kode bahasa ("id", "en", "prog")
     * @param filename Path ke file database kata
     * @return true jika berhasil, false jika file tidak ditemukan
     */
    bool loadWords(const std::string& language, const std::string& filename);
    
    /**
     * @brief Mendapatkan list kata acak sesuai kriteria
     * @param language Kode bahasa
     * @param difficulty Tingkat kesulitan
     * @param count Jumlah kata yang diinginkan
     * @return Vector berisi kata-kata acak yang sudah difilter
     */
    std::vector<std::string> getWords(
        const std::string& language, 
        Difficulty difficulty, 
        int count
    );

private:
    /**
     * @brief Map untuk menyimpan database kata
     * 
     * Key: Kode bahasa ("id", "en", "prog")
     * Value: Vector berisi semua kata dalam bahasa tersebut
     */
    std::map<std::string, std::vector<std::string>> wordBanks;
    
    /**
     * @brief Validasi apakah kata cocok untuk difficulty tertentu
     * @param word Kata yang akan divalidasi
     * @param difficulty Tingkat kesulitan
     * @return true jika kata valid untuk difficulty tersebut
     */
    bool isWordValidForDifficulty(const std::string& word, Difficulty difficulty);
};

#endif // TEXTPROVIDER_H
