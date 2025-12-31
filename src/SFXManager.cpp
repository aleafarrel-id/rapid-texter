/**
 * @file SFXManager.cpp
 * @brief Implementasi Sound Effects Manager dengan Overlapping Audio Support
 * @author Alea Farrel
 * @date 2025
 *
 * OVERLAPPING AUDIO IMPLEMENTATION:
 * - Menggunakan waveOut API dengan audio pool (8 channels)
 * - Setiap channel bisa memutar audio secara independen
 * - Audio di-preload ke memory untuk zero-latency
 * - Callback system untuk recycling channels
 */

#include "SFXManager.h"
#include "SettingsManager.h"

#include <fstream>
#include <thread>

#ifdef _WIN32
// Windows headers already included in SFXManager.h
#else
// Linux includes
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#endif

// ============================================================================
// STATIC MEMBER INITIALIZATION
// ============================================================================

bool SFXManager::sfxEnabled = true;
bool SFXManager::settingsLoaded = false;

#ifdef _WIN32
// Audio buffers
std::vector<char> SFXManager::trueWavData;
std::vector<char> SFXManager::falseWavData;
std::atomic<bool> SFXManager::buffersLoaded{false};
std::atomic<bool> SFXManager::isLoading{false};

// Audio pool
std::array<HWAVEOUT, SFXManager::AUDIO_POOL_SIZE> SFXManager::waveOutHandles = {};
std::array<WAVEHDR, SFXManager::AUDIO_POOL_SIZE> SFXManager::waveHeaders = {};
std::array<std::atomic<bool>, SFXManager::AUDIO_POOL_SIZE> SFXManager::channelBusy = {};
std::atomic<int> SFXManager::nextChannel{0};

#else
// Linux
std::string SFXManager::audioPlayer = "";
bool SFXManager::audioPlayerDetected = false;
#endif

// ============================================================================
// WINDOWS: waveOut AUDIO POOL IMPLEMENTATION
// ============================================================================

#ifdef _WIN32

/**
 * @brief Callback ketika audio selesai diputar
 * Menandai channel sebagai available untuk reuse
 */
void CALLBACK SFXManager::waveOutCallback(HWAVEOUT hwo, UINT uMsg,
                                           DWORD_PTR dwInstance,
                                           DWORD_PTR dwParam1,
                                           DWORD_PTR dwParam2) {
  if (uMsg == WOM_DONE) {
    // Channel index disimpan di dwInstance
    int channelIndex = static_cast<int>(dwInstance);
    if (channelIndex >= 0 && channelIndex < AUDIO_POOL_SIZE) {
      // Unprepare header
      WAVEHDR* hdr = reinterpret_cast<WAVEHDR*>(dwParam1);
      waveOutUnprepareHeader(hwo, hdr, sizeof(WAVEHDR));
      
      // Mark channel as available
      channelBusy[channelIndex].store(false, std::memory_order_release);
    }
  }
}

/**
 * @brief Load WAV file ke memory buffer
 * 
 * Membaca seluruh WAV file ke RAM. Format WAV harus standard PCM.
 */
bool SFXManager::loadWavFile(const char* filename, std::vector<char>& buffer) {
  std::ifstream file(filename, std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    return false;
  }

  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  buffer.resize(static_cast<size_t>(size));
  if (!file.read(buffer.data(), size)) {
    buffer.clear();
    return false;
  }

  return true;
}

/**
 * @brief Play audio dari buffer menggunakan available channel
 *
 * Round-robin channel selection dengan fallback jika semua busy.
 * Audio diputar secara asynchronous - tidak blocking.
 */
void SFXManager::playFromPool(const std::vector<char>& wavData) {
  if (wavData.size() < 44) return; // Minimum WAV header size

  // Parse WAV header untuk mendapatkan format
  // WAV structure: RIFF header (12 bytes) + fmt chunk + data chunk
  const char* data = wavData.data();
  
  // Verify RIFF header
  if (memcmp(data, "RIFF", 4) != 0 || memcmp(data + 8, "WAVE", 4) != 0) {
    return;
  }

  // Find fmt chunk
  int pos = 12;
  WAVEFORMATEX wfx = {};
  const char* audioData = nullptr;
  DWORD audioSize = 0;

  while (pos < static_cast<int>(wavData.size()) - 8) {
    char chunkId[5] = {0};
    memcpy(chunkId, data + pos, 4);
    DWORD chunkSize = *reinterpret_cast<const DWORD*>(data + pos + 4);

    if (memcmp(chunkId, "fmt ", 4) == 0) {
      // Parse format chunk
      wfx.wFormatTag = *reinterpret_cast<const WORD*>(data + pos + 8);
      wfx.nChannels = *reinterpret_cast<const WORD*>(data + pos + 10);
      wfx.nSamplesPerSec = *reinterpret_cast<const DWORD*>(data + pos + 12);
      wfx.nAvgBytesPerSec = *reinterpret_cast<const DWORD*>(data + pos + 16);
      wfx.nBlockAlign = *reinterpret_cast<const WORD*>(data + pos + 20);
      wfx.wBitsPerSample = *reinterpret_cast<const WORD*>(data + pos + 22);
      wfx.cbSize = 0;
    } else if (memcmp(chunkId, "data", 4) == 0) {
      audioData = data + pos + 8;
      audioSize = chunkSize;
      break;
    }

    pos += 8 + chunkSize;
    if (chunkSize % 2 == 1) pos++; // Padding
  }

  if (!audioData || audioSize == 0) return;

  // Find available channel (round-robin with busy check)
  int startChannel = nextChannel.load(std::memory_order_relaxed);
  int channel = startChannel;
  bool found = false;

  for (int i = 0; i < AUDIO_POOL_SIZE; i++) {
    if (!channelBusy[channel].load(std::memory_order_acquire)) {
      found = true;
      break;
    }
    channel = (channel + 1) % AUDIO_POOL_SIZE;
  }

  if (!found) {
    // Semua channel busy - skip audio ini (lebih baik daripada crash)
    return;
  }

  // Update next channel untuk round-robin
  nextChannel.store((channel + 1) % AUDIO_POOL_SIZE, std::memory_order_relaxed);

  // Mark channel as busy
  channelBusy[channel].store(true, std::memory_order_release);

  // Close previous handle if exists
  if (waveOutHandles[channel]) {
    waveOutReset(waveOutHandles[channel]);
    waveOutClose(waveOutHandles[channel]);
    waveOutHandles[channel] = nullptr;
  }

  // Open new waveOut device
  MMRESULT result = waveOutOpen(
    &waveOutHandles[channel],
    WAVE_MAPPER,
    &wfx,
    reinterpret_cast<DWORD_PTR>(waveOutCallback),
    static_cast<DWORD_PTR>(channel),
    CALLBACK_FUNCTION
  );

  if (result != MMSYSERR_NOERROR) {
    channelBusy[channel].store(false, std::memory_order_release);
    return;
  }

  // Prepare header
  ZeroMemory(&waveHeaders[channel], sizeof(WAVEHDR));
  waveHeaders[channel].lpData = const_cast<LPSTR>(audioData);
  waveHeaders[channel].dwBufferLength = audioSize;
  waveHeaders[channel].dwFlags = 0;

  result = waveOutPrepareHeader(waveOutHandles[channel], 
                                 &waveHeaders[channel], 
                                 sizeof(WAVEHDR));
  if (result != MMSYSERR_NOERROR) {
    waveOutClose(waveOutHandles[channel]);
    waveOutHandles[channel] = nullptr;
    channelBusy[channel].store(false, std::memory_order_release);
    return;
  }

  // Play!
  result = waveOutWrite(waveOutHandles[channel], 
                        &waveHeaders[channel], 
                        sizeof(WAVEHDR));
  if (result != MMSYSERR_NOERROR) {
    waveOutUnprepareHeader(waveOutHandles[channel], 
                           &waveHeaders[channel], 
                           sizeof(WAVEHDR));
    waveOutClose(waveOutHandles[channel]);
    waveOutHandles[channel] = nullptr;
    channelBusy[channel].store(false, std::memory_order_release);
  }
}

#else
// ============================================================================
// LINUX: Fork-based async playback (unchanged)
// ============================================================================

std::string SFXManager::detectAudioPlayer() {
  if (system("which paplay > /dev/null 2>&1") == 0) {
    return "paplay";
  }
  if (system("which aplay > /dev/null 2>&1") == 0) {
    return "aplay -q";
  }
  return "";
}

void SFXManager::playAudioLinux(const char *filename) {
  if (audioPlayer.empty()) return;

  pid_t pid = fork();
  if (pid == 0) {
    freopen("/dev/null", "w", stdout);
    freopen("/dev/null", "w", stderr);
    std::string cmd = audioPlayer + " " + filename;
    execl("/bin/sh", "sh", "-c", cmd.c_str(), (char *)NULL);
    _exit(1);
  }
  if (pid > 0) {
    signal(SIGCHLD, SIG_IGN);
  }
}
#endif

// ============================================================================
// AUDIO PLAYBACK
// ============================================================================

void SFXManager::playTrue() {
  if (!sfxEnabled) return;

#ifdef _WIN32
  if (buffersLoaded.load(std::memory_order_acquire) && !trueWavData.empty()) {
    playFromPool(trueWavData);
  }
#else
  playAudioLinux("assets/true.wav");
#endif
}

void SFXManager::playFalse() {
  if (!sfxEnabled) return;

#ifdef _WIN32
  if (buffersLoaded.load(std::memory_order_acquire) && !falseWavData.empty()) {
    playFromPool(falseWavData);
  }
#else
  playAudioLinux("assets/false.wav");
#endif
}

// ============================================================================
// TOGGLE & STATUS
// ============================================================================

void SFXManager::toggle() {
  sfxEnabled = !sfxEnabled;
  SettingsManager::setSfxEnabled(sfxEnabled);
  if (sfxEnabled) {
    preload();
  }
}

bool SFXManager::isEnabled() {
  if (!settingsLoaded) {
    sfxEnabled = SettingsManager::getSfxEnabled();
    settingsLoaded = true;
  }
  return sfxEnabled;
}

// ============================================================================
// PRELOAD & CLEANUP
// ============================================================================

void SFXManager::preload() {
#ifdef _WIN32
  // Check if already loaded or loading
  if (buffersLoaded.load(std::memory_order_acquire) || isLoading.load(std::memory_order_acquire)) {
    return;
  }

  isLoading.store(true, std::memory_order_release);

  // Initialize channel states immediately
  for (int i = 0; i < AUDIO_POOL_SIZE; i++) {
    channelBusy[i].store(false, std::memory_order_relaxed);
  }
  nextChannel.store(0, std::memory_order_relaxed);

  // Load audio files in background thread
  std::thread([]() {
    std::vector<char> tData, fData;
    bool tLoaded = loadWavFile("assets\\true.wav", tData);
    bool fLoaded = loadWavFile("assets\\false.wav", fData);
    
    if (tLoaded && fLoaded) {
      trueWavData = std::move(tData);
      falseWavData = std::move(fData);
      buffersLoaded.store(true, std::memory_order_release);
    }
    isLoading.store(false, std::memory_order_release);
  }).detach();

#else
  if (!audioPlayerDetected) {
    audioPlayer = detectAudioPlayer();
    audioPlayerDetected = true;
  }
#endif
}

void SFXManager::cleanup() {
#ifdef _WIN32
  // Stop and close all wave devices
  for (int i = 0; i < AUDIO_POOL_SIZE; i++) {
    if (waveOutHandles[i]) {
      waveOutReset(waveOutHandles[i]);
      waveOutUnprepareHeader(waveOutHandles[i], &waveHeaders[i], sizeof(WAVEHDR));
      waveOutClose(waveOutHandles[i]);
      waveOutHandles[i] = nullptr;
    }
  }

  // Clear buffers
  trueWavData.clear();
  trueWavData.shrink_to_fit();
  falseWavData.clear();
  falseWavData.shrink_to_fit();
  buffersLoaded.store(false, std::memory_order_release);
#endif
}

