/**
 * @file TextProvider.cpp
 * @brief Implementasi TextProvider untuk pengelolaan database kata-kata
 * @author Alea Farrel & Team
 * @date 2025
 * 
 * File ini mengimplementasikan class TextProvider yang bertanggung jawab
 * untuk memuat, menyimpan, dan menyediakan kata-kata untuk typing test.
 * 
 * @section features Fitur Utama
 * - Loading kata dari file teks eksternal
 * - Filtering kata berdasarkan tingkat kesulitan
 * - Random selection untuk variasi gameplay
 * - Multi-language support (ID, EN, PROG)
 * 
 * @section difficulty Sistem Kesulitan
 * Kata-kata difilter berdasarkan panjang:
 * - EASY: Kata pendek (≤6 karakter) - cocok untuk pemula
 * - MEDIUM: Kata sedang (≤10 karakter) - untuk intermediate
 * - HARD: Kata panjang (≤14 karakter) - untuk advanced
 * - PROGRAMMER: Semua kata/sintaks tanpa filter panjang
 * 
 * @section storage Format Penyimpanan
 * File database kata menggunakan format plain text dengan satu kata per
 * baris atau dipisahkan oleh whitespace. Karakter non-ASCII akan
 * otomatis difilter saat loading.
 */

#include "TextProvider.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <random>
#include <ctime>

// ============================================================================
// ANONYMOUS NAMESPACE - Helper Functions
// ============================================================================

/**
 * @namespace (anonymous)
 * @brief Namespace anonim untuk helper functions internal
 * 
 * Fungsi-fungsi dalam namespace ini bersifat private dan hanya
 * digunakan oleh TextProvider.cpp. Tidak dapat diakses dari file lain.
 */
namespace {
    /**
     * @brief Membersihkan karakter non-ASCII dari string kata
     * 
     * Fungsi ini memfilter karakter yang tidak valid untuk typing test.
     * Hanya karakter ASCII printable (kode 32-126) yang dipertahankan.
     * Ini mencakup:
     * - Spasi (32)
     * - Tanda baca (33-47, 58-64, 91-96, 123-126)
     * - Angka (48-57)
     * - Huruf besar (65-90)
     * - Huruf kecil (97-122)
     * 
     * @param word String kata yang akan dibersihkan
     * @return std::string Kata yang sudah dibersihkan dari karakter non-ASCII
     * 
     * @par Alasan Filtering
     * - Mencegah karakter UTF-8 multi-byte yang tidak bisa diketik
     * - Menghindari karakter kontrol yang merusak tampilan terminal
     * - Memastikan konsistensi input keyboard standar
     * 
     * @par Contoh
     * @code
     * sanitizeWord("café");   // return "caf" (é dihapus)
     * sanitizeWord("hello");  // return "hello" (tidak berubah)
     * sanitizeWord("日本語"); // return "" (semua dihapus)
     * @endcode
     */
    std::string sanitizeWord(const std::string& word) {
        std::string cleaned;
        cleaned.reserve(word.length());  // Pre-allocate untuk efisiensi
        for (char c : word) {
            // Hanya terima karakter ASCII printable (32-126)
            if (c >= 32 && c <= 126) {
                cleaned += c;
            }
        }
        return cleaned;
    }
}

// ============================================================================
// CONSTRUCTOR
// ============================================================================

/**
 * @brief Constructor - Inisialisasi random number generator
 * 
 * Melakukan seeding untuk std::rand() menggunakan waktu saat ini.
 * Ini memastikan kata-kata yang dipilih akan berbeda setiap kali
 * aplikasi dijalankan.
 * 
 * @note Menggunakan std::srand() untuk kompatibilitas dengan std::rand()
 *       yang digunakan di getWords(). Untuk aplikasi yang membutuhkan
 *       randomness berkualitas tinggi, pertimbangkan menggunakan
 *       <random> library dengan proper seeding.
 */
TextProvider::TextProvider() {
    // Seed random number generator saat inisialisasi
    // time(nullptr) mengembalikan Unix timestamp saat ini
    std::srand(std::time(nullptr));
}

// ============================================================================
// WORD LOADING
// ============================================================================

/**
 * @brief Memuat daftar kata dari file teks ke dalam memory
 * 
 * Membaca file database kata dan menyimpannya dalam map internal
 * yang diindeks berdasarkan kode bahasa.
 * 
 * @param language Kode bahasa untuk mengindeks database ("id", "en", "prog")
 * @param filename Path ke file database kata (relatif atau absolut)
 * @return true jika file berhasil dibaca
 * @return false jika file tidak dapat dibuka
 * 
 * @par Format File Input
 * File dapat mengandung kata-kata yang dipisahkan oleh:
 * - Spasi
 * - Tab
 * - Newline (satu kata per baris)
 * 
 * @par Proses Loading
 * 1. Buka file menggunakan ifstream
 * 2. Baca kata per kata menggunakan operator >>
 * 3. Sanitize setiap kata (hapus karakter non-ASCII)
 * 4. Simpan kata yang valid ke vector
 * 5. Masukkan vector ke map dengan key = language
 * 
 * @par Contoh Penggunaan
 * @code
 * TextProvider provider;
 * provider.loadWords("id", "assets/id.txt");
 * provider.loadWords("en", "assets/en.txt");
 * provider.loadWords("prog", "assets/prog.txt");
 * @endcode
 * 
 * @warning Jika file tidak ditemukan, pesan error akan dicetak ke stderr
 * 
 * @see sanitizeWord()
 */
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

// ============================================================================
// WORD RETRIEVAL
// ============================================================================

/**
 * @brief Mengambil sejumlah kata acak berdasarkan bahasa dan kesulitan
 * 
 * Fungsi utama yang menyediakan kata-kata untuk gameplay. Kata-kata
 * difilter terlebih dahulu berdasarkan tingkat kesulitan, kemudian
 * dipilih secara acak dari hasil filter.
 * 
 * @param language Kode bahasa ("id", "en", "prog")
 * @param difficulty Tingkat kesulitan (EASY/MEDIUM/HARD/PROGRAMMER)
 * @param count Jumlah kata yang diinginkan
 * @return std::vector<std::string> Vector berisi kata-kata acak yang sudah difilter.
 *         Akan kosong jika:
 *         - Bahasa tidak terdaftar di wordBanks
 *         - Tidak ada kata yang memenuhi kriteria difficulty
 * 
 * @par Algoritma
 * 1. Cek apakah bahasa ada di database (wordBanks)
 * 2. Filter semua kata berdasarkan kriteria difficulty
 * 3. Pilih kata secara acak dari hasil filter sebanyak count
 * 4. Kata yang sama bisa muncul lebih dari sekali (with replacement)
 * 
 * @par Catatan Performa
 * - Filtering dilakukan setiap kali fungsi dipanggil
 * - Untuk optimasi, bisa dipertimbangkan caching hasil filter
 * - Kompleksitas waktu: O(n) untuk filtering + O(count) untuk selection
 * 
 * @par Contoh Penggunaan
 * @code
 * TextProvider provider;
 * provider.loadWords("id", "assets/id.txt");
 * 
 * // Ambil 30 kata Easy untuk bahasa Indonesia
 * auto words = provider.getWords("id", Difficulty::EASY, 30);
 * @endcode
 * 
 * @see isWordValidForDifficulty()
 */
std::vector<std::string> TextProvider::getWords(const std::string& language, Difficulty difficulty, int count) {
    std::vector<std::string> result;
    
    // Cek ketersediaan bahasa dalam database
    // Jika bahasa tidak ditemukan, return vector kosong
    if (wordBanks.find(language) == wordBanks.end()) {
        return result;
    }

    const auto& allWords = wordBanks[language];
    std::vector<std::string> filtered;

    // Filter kata berdasarkan tingkat kesulitan
    // Loop melalui semua kata dan cek apakah valid untuk difficulty
    for (const auto& w : allWords) {
        if (isWordValidForDifficulty(w, difficulty)) {
            filtered.push_back(w);
        }
    }

    // Jika tidak ada kata yang memenuhi kriteria, return kosong
    if (filtered.empty()) return result;

    // Pilih kata secara acak dari hasil filter
    // Menggunakan modulo untuk mendapatkan index acak
    // Kata yang sama bisa terpilih berkali-kali (sampling with replacement)
    for (int i = 0; i < count; ++i) {
        result.push_back(filtered[std::rand() % filtered.size()]);
    }

    return result;
}

// ============================================================================
// DIFFICULTY VALIDATION
// ============================================================================

/**
 * @brief Memvalidasi apakah kata cocok untuk tingkat kesulitan tertentu
 * 
 * Menentukan kelayakan kata berdasarkan panjangnya. Sistem ini
 * memastikan pemain pemula mendapat kata pendek yang mudah,
 * sementara pemain advanced mendapat kata panjang yang menantang.
 * 
 * @param word Kata yang akan divalidasi
 * @param difficulty Tingkat kesulitan yang diinginkan
 * @return true jika kata valid untuk difficulty tersebut
 * @return false jika kata tidak memenuhi kriteria
 * 
 * @par Kriteria Per Difficulty
 * | Difficulty  | Maks Panjang | Contoh Kata |
 * |-------------|--------------|-------------|
 * | EASY        | ≤6 karakter  | "apple", "book", "cat" |
 * | MEDIUM      | ≤10 karakter | "beautiful", "computer" |
 * | HARD        | ≤14 karakter | "extraordinary", "sophisticated" |
 * | PROGRAMMER  | Tanpa batas  | "std::vector<std::string>" |
 * 
 * @par Catatan Desain
 * - Kriteria bersifat inklusif (EASY termasuk kata ≤6, bukan hanya <6)
 * - MEDIUM dan HARD juga mencakup kata-kata yang lebih pendek
 * - PROGRAMMER mode tidak memfilter panjang karena sintaks
 *   programming bisa sangat panjang (contoh: function signatures)
 * 
 * @par Contoh
 * @code
 * isWordValidForDifficulty("cat", Difficulty::EASY);      // true (3 ≤ 6)
 * isWordValidForDifficulty("elephant", Difficulty::EASY); // false (8 > 6)
 * isWordValidForDifficulty("elephant", Difficulty::MEDIUM); // true (8 ≤ 10)
 * isWordValidForDifficulty("any_word", Difficulty::PROGRAMMER); // selalu true
 * @endcode
 */
bool TextProvider::isWordValidForDifficulty(const std::string& word, Difficulty difficulty) {
    size_t len = word.length();
    
    switch (difficulty) {
        case Difficulty::EASY:
            return len <= 6;  // Kata pendek untuk pemula
        case Difficulty::MEDIUM:
            return len <= 10; // Kata sedang
        case Difficulty::HARD:
            return len <= 14; // Kata panjang
        case Difficulty::PROGRAMMER:
            return true;      // Programmer mode: semua kata (biasanya syntax) valid
    }
    return true;  // Default fallback
}