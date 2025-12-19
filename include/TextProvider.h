#ifndef TEXTPROVIDER_H
#define TEXTPROVIDER_H

#include <string>
#include <vector>
#include <map>

// Tingkat kesulitan yang tersedia
enum class Difficulty {
    EASY,       // Kata pendek
    MEDIUM,     // Kata sedang
    HARD,       // Kata panjang
    PROGRAMMER  // Kode/Sintaks (tanpa filter panjang)
};

class TextProvider {
public:
    TextProvider();
    
    // Memuat kata dari file teks (misal: "id.txt") ke dalam memori
    bool loadWords(const std::string& language, const std::string& filename);
    
    // Mendapatkan daftar kata acak sesuai kriteria
    std::vector<std::string> getWords(const std::string& language, Difficulty difficulty, int count);

private:
    // Penyimpanan kata: Map[Kode Bahasa] -> List Kata
    std::map<std::string, std::vector<std::string>> wordBanks;
    
    // Validasi apakah kata cocok untuk kesulitan tertentu
    bool isWordValidForDifficulty(const std::string& word, Difficulty difficulty);
};

#endif