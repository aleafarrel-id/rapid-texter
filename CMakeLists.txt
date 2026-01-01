cmake_minimum_required(VERSION 3.10)
project(RapidTexter)

set(CMAKE_CXX_STANDARD 17)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
file(GLOB SOURCES "src/*.cpp")

# ------------------------------------------------------------------------------
# 1. PERSIAPAN RESOURCES (Dilakukan SEBELUM add_executable)
# ------------------------------------------------------------------------------
if(WIN32)
    # File ini berisi icon dan metadata
    set(RESOURCE_FILE "${CMAKE_SOURCE_DIR}/resources/app.rc")
    
    if(EXISTS ${RESOURCE_FILE})
        message(STATUS "Adding Windows resource file: ${RESOURCE_FILE}")
        # Tambahkan file .rc ke dalam list SOURCES agar ikut dicompile
        list(APPEND SOURCES ${RESOURCE_FILE})
    else()
        message(WARNING "Resource file not found: ${RESOURCE_FILE}")
    endif()
endif()

# ------------------------------------------------------------------------------
# 2. PEMBUATAN TARGET (PENTING: Harus di sini)
# ------------------------------------------------------------------------------
# Kita membuat 'mobil'-nya dulu di sini, baru bisa dimodifikasi di bawahnya
add_executable(RapidTexter ${SOURCES})

# ------------------------------------------------------------------------------
# 3. KONFIGURASI LINKING & LIBRARY (Dilakukan SETELAH add_executable)
# ------------------------------------------------------------------------------
if(WIN32)
    # Jika menggunakan MinGW/GCC/Clang, gunakan Static Linking agar portable
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_link_options(RapidTexter PRIVATE 
            -static 
            -static-libgcc 
            -static-libstdc++
        )
    endif()
    
    # Link library multimedia Windows dan Shell32 untuk path access
    target_link_libraries(RapidTexter PRIVATE winmm Shell32)

elseif(UNIX AND NOT APPLE)
    find_package(Threads REQUIRED)
    
    # SDL2 and SDL2_mixer for audio pool (overlapping audio support)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED sdl2)
    pkg_check_modules(SDL2_MIXER REQUIRED SDL2_mixer)
    
    target_include_directories(RapidTexter PRIVATE ${SDL2_INCLUDE_DIRS} ${SDL2_MIXER_INCLUDE_DIRS})
    target_link_libraries(RapidTexter PRIVATE 
        Threads::Threads 
        ${SDL2_LIBRARIES} 
        ${SDL2_MIXER_LIBRARIES}
    )
endif()

# ------------------------------------------------------------------------------
# 4. COPY ASSETS
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

# Copy resources directory (opsional, untuk distribusi)
add_custom_command(TARGET RapidTexter POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/resources
    $<TARGET_FILE_DIR:RapidTexter>/resources
    COMMENT "Copying resources directory..."
)