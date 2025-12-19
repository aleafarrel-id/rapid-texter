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
# perintah ini akan menyalin SELURUH folder 'assets' dari source code
# ke dalam folder build tempat executable berada.
#
# Hasilnya nanti di folder build akan ada folder: .../build/assets/id.txt, dll.
# Ini sesuai dengan path baru di C++: "assets/id.txt"

add_custom_command(TARGET rapid_texter POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:rapid_texter>/assets
    COMMENT "Menyalin folder assets ke direktori output..."
)