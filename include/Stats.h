/**
 * @file Stats.h
 * @brief Struktur data untuk statistik gameplay
 * @author Alea Farrel & Team
 * @date 2025
 * 
 * Menyimpan dan menghitung statistik permainan seperti WPM, accuracy,
 * total keystroke, dan error count.
 */

#ifndef STATS_H
#define STATS_H

/**
 * @struct Stats
 * @brief Struktur data untuk menyimpan statistik permainan
 * 
 * Stats menghitung:
 * - WPM (Words Per Minute): (correct_chars / 5) / minutes
 * - Accuracy: (correct_keystrokes / total_keystrokes) * 100%
 * - Time taken
 * - Error count
 */
struct Stats {
    // ========================================================================
    // Data Members
    // ========================================================================
    
    double wpm = 0.0;               ///< Words Per Minute
    double accuracy = 0.0;          ///< Percentage akurasi (0-100)
    double timeTaken = 0.0;         ///< Waktu yang dihabiskan (detik)
    int totalKeystrokes = 0;        ///< Total tombol yang ditekan
    int correctKeystrokes = 0;      ///< Jumlah keystroke yang benar
    int errors = 0;                 ///< Jumlah kesalahan
    
    // ========================================================================
    // Methods
    // ========================================================================
    
    /**
     * @brief Menghitung WPM dan Accuracy berdasarkan data yang ada
     * @param totalMappedChars Jumlah karakter target yang dimapped
     * 
     * Formula:
     * - WPM = (correctKeystrokes / 5) / (timeTaken / 60)
     *   Angka 5 adalah standar rata-rata panjang kata dalam typing test
     * 
     * - Accuracy = (correctKeystrokes / totalKeystrokes) * 100
     */
    void calculate(int totalMappedChars) {
        // Hitung WPM
        double minutes = timeTaken / 60.0;
        if (minutes > 0) {
            wpm = (correctKeystrokes / 5.0) / minutes;
        } else {
            wpm = 0;
        }
        
        // Hitung Accuracy
        if (totalKeystrokes > 0) {
            accuracy = (static_cast<double>(correctKeystrokes) / totalKeystrokes) * 100.0;
        } else {
            accuracy = 0.0;
        }
    }
    
    /**
     * @brief Reset semua statistik ke nilai awal (0)
     */
    void reset() {
        wpm = 0;
        accuracy = 0;
        timeTaken = 0;
        totalKeystrokes = 0;
        correctKeystrokes = 0;
        errors = 0;
    }
};

#endif // STATS_H