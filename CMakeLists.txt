cmake_minimum_required(VERSION 3.10)
project(RapidTexter)

set(CMAKE_CXX_STANDARD 17)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
file(GLOB SOURCES "src/*.cpp")

# Create executable
add_executable(RapidTexter ${SOURCES})

# ------------------------------------------------------------------------------
# KONFIGURASI PLATFORM (OS)
# ------------------------------------------------------------------------------

# 1. WINDOWS
if(WIN32)
    # Jika menggunakan MinGW/GCC/Clang, gunakan Static Linking agar portable
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_link_options(RapidTexter PRIVATE 
            -static 
            -static-libgcc 
            -static-libstdc++
        )
    endif()
    
    # PENTING: Link library multimedia Windows (winmm) untuk fungsi PlaySound
    target_link_libraries(RapidTexter PRIVATE -static -lwinmm)
    
# 2. LINUX / UNIX
elseif(UNIX AND NOT APPLE)
    # Di Linux, kita perlu mencari dan me-link library Threads (pthread) secara eksplisit
    find_package(Threads REQUIRED)
    target_link_libraries(RapidTexter PRIVATE Threads::Threads)
endif()

# ------------------------------------------------------------------------------
# COPY ASSETS (Berjalan di semua OS)
# ------------------------------------------------------------------------------

# Copy assets directory
add_custom_command(TARGET RapidTexter POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:RapidTexter>/assets
    COMMENT "Copying assets directory..."
)

# Copy roll directory
add_custom_command(TARGET RapidTexter POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/roll
    $<TARGET_FILE_DIR:RapidTexter>/roll
    COMMENT "Copying roll directory..."
)