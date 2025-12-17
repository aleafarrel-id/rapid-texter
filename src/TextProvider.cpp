#include "TextProvider.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <random>
#include <ctime>

TextProvider::TextProvider() {
    std::srand(std::time(nullptr));
}

bool TextProvider::loadWords(const std::string& language, const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }

    std::string word;
    std::vector<std::string> words;
    while (file >> word) {
        // Basic cleanup if needed, though assume clean input for now
        if (!word.empty()) {
            words.push_back(word);
        }
    }
    
    wordBanks[language] = words;
    return true;
}

std::vector<std::string> TextProvider::getWords(const std::string& language, Difficulty difficulty, int count) {
    std::vector<std::string> result;
    if (wordBanks.find(language) == wordBanks.end()) {
        return result;
    }

    const auto& allWords = wordBanks[language];
    std::vector<std::string> filtered;

    for (const auto& w : allWords) {
        if (isWordValidForDifficulty(w, difficulty)) {
            filtered.push_back(w);
        }
    }

    if (filtered.empty()) return result;

    for (int i = 0; i < count; ++i) {
        result.push_back(filtered[std::rand() % filtered.size()]);
    }

    return result;
}

bool TextProvider::isWordValidForDifficulty(const std::string& word, Difficulty difficulty) {
    size_t len = word.length();
    
    switch (difficulty) {
        case Difficulty::EASY:
            return len <= 6;
        case Difficulty::MEDIUM:
            return len <= 10;
        case Difficulty::HARD:
            return len <= 14; 
        case Difficulty::PROGRAMMER:
            return true; // No length restriction for programmer logic (controlled by specific file usually)
    }
    return true;
}
