#ifndef STATS_H
#define STATS_H

struct Stats {
    double wpm = 0.0;
    double accuracy = 0.0;
    double timeTaken = 0.0;
    int totalKeystrokes = 0;
    int correctKeystrokes = 0;
    int errors = 0;
    
    void calculate(int totalMappedChars) {
        // WPM calculation: (all typed / 5) / time_in_minutes
        // Standard definition often uses correct ones, or all.
        // Usually (Total Characters / 5) / (Time / 60) -> Net WPM deducts errors.
        // User asked for "Kecepatan dalam WPM". I will use standard Gross/Net.
        // Let's go with (Correct / 5) / (Time / 60) for accuracy focused WPM.
        
        double minutes = timeTaken / 60.0;
        if (minutes > 0) {
            wpm = (correctKeystrokes / 5.0) / minutes;
        } else {
            wpm = 0;
        }
        
        if (totalKeystrokes > 0) {
            accuracy = (static_cast<double>(correctKeystrokes) / totalKeystrokes) * 100.0;
        } else {
            accuracy = 0.0;
        }
    }
    
    void reset() {
        wpm = 0;
        accuracy = 0;
        timeTaken = 0;
        totalKeystrokes = 0;
        correctKeystrokes = 0;
        errors = 0;
    }
};

#endif
