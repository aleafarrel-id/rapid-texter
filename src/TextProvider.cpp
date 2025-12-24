#include "TextProvider.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <random>
#include <ctime>

namespace {
    // Helper function untuk membersihkan karakter non-ASCII
    std::string sanitizeWord(const std::string& word) {
        std::string cleaned;
        cleaned.reserve(word.length());
        for (char c : word) {
            // Hanya terima karakter ASCII printable (32-126)
            if (c >= 32 && c <= 126) {
                cleaned += c;
            }
        }
        return cleaned;
    }
}

TextProvider::TextProvider() {
    // Seed random number generator saat inisialisasi
    std::srand(std::time(nullptr));
}

// Memuat daftar kata dari file teks ke dalam map
bool TextProvider::loadWords(const std::string& language, const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }

    std::string word;
    std::vector<std::string> words;
    // Baca kata per kata (dipisahkan spasi/newline)
    while (file >> word) {
        if (!word.empty()) {
            std::string cleaned = sanitizeWord(word);
            if (!cleaned.empty()) {  // Pastikan masih ada isi setelah cleaning
                words.push_back(cleaned);
            }
        }
    }
    
    wordBanks[language] = words;
    return true;
}

// Mengambil sejumlah kata acak berdasarkan kriteria
std::vector<std::string> TextProvider::getWords(const std::string& language, Difficulty difficulty, int count) {
    std::vector<std::string> result;
    // Cek ketersediaan bahasa
    if (wordBanks.find(language) == wordBanks.end()) {
        return result;
    }

    const auto& allWords = wordBanks[language];
    std::vector<std::string> filtered;

    // Filter kata berdasarkan tingkat kesulitan
    for (const auto& w : allWords) {
        if (isWordValidForDifficulty(w, difficulty)) {
            filtered.push_back(w);
        }
    }

    if (filtered.empty()) return result;

    // Pilih kata secara acak dari hasil filter
    for (int i = 0; i < count; ++i) {
        result.push_back(filtered[std::rand() % filtered.size()]);
    }

    return result;
}

// Logika penentuan kesulitan berdasarkan panjang kata
bool TextProvider::isWordValidForDifficulty(const std::string& word, Difficulty difficulty) {
    size_t len = word.length();
    
    switch (difficulty) {
        case Difficulty::EASY:
            return len <= 6; // Kata pendek
        case Difficulty::MEDIUM:
            return len <= 10; // Kata sedang
        case Difficulty::HARD:
            return len <= 14; // Kata panjang
        case Difficulty::PROGRAMMER:
            return true; // Programmer mode: semua kata (biasanya syntax) valid
    }
    return true;
}