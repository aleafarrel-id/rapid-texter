#include "GameEngine.h"
#include "SFXManager.h"

// Fungsi utama (entry point) program
int main() {
    // Preload audio files asynchronously
    SFXManager::preload();

    // Membuat instance dari GameEngine
    GameEngine engine;
    
    // Menjalankan loop utama permainan
    engine.run();
    
    return 0;
}