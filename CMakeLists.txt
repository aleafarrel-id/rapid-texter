cmake_minimum_required(VERSION 3.10)

# Nama Proyek dan Versi
project(RapidTexter VERSION 1.0.0 LANGUAGES CXX)

# Menentukan Standar C++ (C++17 disarankan untuk fitur modern)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Menambahkan folder 'include' agar header file (.h) bisa ditemukan
include_directories(include)

# Mencari seluruh file source (.cpp) di dalam folder src
file(GLOB SOURCES "src/*.cpp")

# Membuat file eksekusi (executable) bernama 'rapid_texter' dari source yang ditemukan
add_executable(rapid_texter ${SOURCES})

# --- BAGIAN PENTING: Manajemen Aset ---
# Skrip di bawah ini otomatis menyalin file .txt (id.txt, en.txt, dll)
# dari folder source ke folder build tempat file .exe berada.
# Ini mencegah error "File not found" saat program dijalankan.
file(GLOB TEXT_ASSETS "*.txt")
foreach(asset ${TEXT_ASSETS})
    add_custom_command(TARGET rapid_texter POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${asset}
        $<TARGET_FILE_DIR:rapid_texter>
        COMMENT "Menyalin aset teks ${asset} ke folder output..."
    )
endforeach()