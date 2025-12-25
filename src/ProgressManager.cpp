/**
 * @file ProgressManager.cpp
 * @brief Implementasi ProgressManager dengan simple JSON parser
 * @author Alea Farrel
 * @date 2025
 */

#include "ProgressManager.h"
#include <fstream>
#include <sstream>
#include <iostream>

ProgressManager::ProgressManager() : filename("progress.json") {
    // Initialize default progress untuk semua bahasa
    progressData["id"] = LanguageProgress();
    progressData["en"] = LanguageProgress();
    progressData["prog"] = LanguageProgress();
    
    // Load existing progress jika ada
    loadProgress();
}

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
        
        // Parse hard completed flag
        else if (line.find("\"hard_completed_ever\"") != std::string::npos) {
            bool value = line.find("true") != std::string::npos;
            progressData[currentLanguage].hardCompletedEver = value;
        }
    }
    
    file.close();
    return true;
}

bool ProgressManager::saveProgress() {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to save progress to " << filename << std::endl;
        return false;
    }
    
    // Write JSON dengan indentation manual
    file << "{\n";
    file << "  \"languages\": {\n";
    
    // Array bahasa untuk iterasi dengan comma handling
    std::string languages[] = {"id", "en", "prog"};
    
    for (int i = 0; i < 3; ++i) {
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
        
        file << "    }" << (i < 2 ? "," : "") << "\n";
    }
    
    file << "  }\n";
    file << "}\n";
    
    file.close();
    return true;
}

bool ProgressManager::resetProgress() {
    // Reset ke default values
    progressData["id"] = LanguageProgress();
    progressData["en"] = LanguageProgress();
    progressData["prog"] = LanguageProgress();
    
    // Delete file (atau save default)
    std::remove(filename.c_str());
    
    // Save fresh progress file
    return saveProgress();
}

LanguageProgress& ProgressManager::getLanguageProgress(const std::string& language) {
    return progressData[language];
}

bool ProgressManager::isUnlocked(const std::string& language, Difficulty difficulty) {
    if (progressData.find(language) == progressData.end()) {
        return false;
    }
    return progressData[language].unlocked[difficulty];
}

bool ProgressManager::isCompleted(const std::string& language, Difficulty difficulty) {
    if (progressData.find(language) == progressData.end()) {
        return false;
    }
    return progressData[language].completed[difficulty];
}

void ProgressManager::setUnlocked(const std::string& language, Difficulty difficulty, bool unlocked) {
    progressData[language].unlocked[difficulty] = unlocked;
}

void ProgressManager::setCompleted(const std::string& language, Difficulty difficulty, bool completed) {
    progressData[language].completed[difficulty] = completed;
}

bool ProgressManager::wasHardCompletedBefore(const std::string& language) {
    if (progressData.find(language) == progressData.end()) {
        return false;
    }
    return progressData[language].hardCompletedEver;
}

void ProgressManager::markHardCompleted(const std::string& language) {
    progressData[language].hardCompletedEver = true;
}

Difficulty ProgressManager::stringToDifficulty(const std::string& diffStr) {
    if (diffStr == "easy") return Difficulty::EASY;
    if (diffStr == "medium") return Difficulty::MEDIUM;
    if (diffStr == "hard") return Difficulty::HARD;
    if (diffStr == "programmer") return Difficulty::PROGRAMMER;
    return Difficulty::EASY;
}

std::string ProgressManager::difficultyToString(Difficulty diff) {
    switch (diff) {
        case Difficulty::EASY: return "easy";
        case Difficulty::MEDIUM: return "medium";
        case Difficulty::HARD: return "hard";
        case Difficulty::PROGRAMMER: return "programmer";
    }
    return "easy";
}