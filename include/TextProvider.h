#ifndef TEXTPROVIDER_H
#define TEXTPROVIDER_H

#include <string>
#include <vector>
#include <map>

enum class Difficulty {
    EASY,
    MEDIUM,
    HARD,
    PROGRAMMER
};

class TextProvider {
public:
    TextProvider();
    
    // Load words from a file into a specific language bank
    bool loadWords(const std::string& language, const std::string& filename);
    
    // Get a random set of words based on language and difficulty
    // count: number of words to return
    std::vector<std::string> getWords(const std::string& language, Difficulty difficulty, int count);

private:
    // Language -> Word List
    std::map<std::string, std::vector<std::string>> wordBanks;
    
    // Helper to filter words based on difficulty criteria
    bool isWordValidForDifficulty(const std::string& word, Difficulty difficulty);
};

#endif
