#ifndef STATS_H
#define STATS_H

// Struktur data sederhana untuk menyimpan statistik permainan
struct Stats {
    double wpm = 0.0;
    double accuracy = 0.0;
    double timeTaken = 0.0;
    int totalKeystrokes = 0;
    int correctKeystrokes = 0;
    int errors = 0;
    
    // Menghitung WPM dan Akurasi berdasarkan data yang ada
    void calculate(int totalMappedChars) {
        // Rumus WPM: (Jumlah Karakter Benar / 5) / (Waktu dalam Menit)
        // Angka 5 adalah standar rata-rata panjang kata dalam mengetik.
        
        double minutes = timeTaken / 60.0;
        if (minutes > 0) {
            wpm = (correctKeystrokes / 5.0) / minutes;
        } else {
            wpm = 0;
        }
        
        // Akurasi: Persentase huruf benar dibanding total tombol ditekan
        if (totalKeystrokes > 0) {
            accuracy = (static_cast<double>(correctKeystrokes) / totalKeystrokes) * 100.0;
        } else {
            accuracy = 0.0;
        }
    }
    
    // Reset statistik ke 0
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