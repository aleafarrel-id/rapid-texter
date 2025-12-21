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

# --- BAGIAN PENTING: Manajemen Aset & Script ---

# Copy folder 'assets' (Word database)
add_custom_command(TARGET rapid_texter POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:rapid_texter>/assets
    COMMENT "Menyalin folder assets game..."
)

# Copy folder 'roll' (Rickroll scripts & assets)
# Ini akan menyalin folder roll/ beserta isinya (roll.ps1, roll.sh, roll.wav, astley.txt)
# ke folder build yang sama dengan executable.
add_custom_command(TARGET rapid_texter POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/roll
    $<TARGET_FILE_DIR:rapid_texter>/roll
    COMMENT "Menyalin folder rickroll..."
)