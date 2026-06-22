// Ini WAJIB ada agar kode internal miniaudio tereksekusi
#define MINIAUDIO_IMPLEMENTATION
#include "../utils/miniaudio.h"

#include "MusicManager.hpp"
#include <fstream>
#include <string>

// KOTAK HITAM KITA: Akan mencatat diam-diam apa yang terjadi di dalam Miniaudio
void debug_audio(const std::string& msg) {
    std::ofstream dbg("debug_miniaudio.txt", std::ios::app);
    dbg << msg << "\n";
    dbg.close();
}

MusicManager::MusicManager() 
    : audioFolderPath("data/audio/"), isEngineInitialized(false), isBgmLoaded(false), isTypingLoaded(false) 
{
    debug_audio("\n=== STARTUP ENGINE ===");
    
    // Menyalakan mesin audio
    ma_result result = ma_engine_init(NULL, &engine);
    if (result == MA_SUCCESS) {
        isEngineInitialized = true;
        // Kita paksa Miniaudio mengaku dia pakai driver apa
        const char* backend_name = ma_get_backend_name(engine.pDevice->pContext->backend);
        debug_audio(std::string("SUCCESS: Engine menyala. Driver yang dipakai: ") + backend_name);
    } else {
        debug_audio("FATAL: Gagal menyalakan Engine. Kode error: " + std::to_string(result));
    }
}

MusicManager::~MusicManager() {
    if (isEngineInitialized) {
        if (isBgmLoaded) ma_sound_uninit(&bgmSound);
        if (isTypingLoaded) ma_sound_uninit(&typingSound);
        ma_engine_uninit(&engine);
    }
}

void MusicManager::playMusic(const std::string& filename) {
    if (!isEngineInitialized) return;

    if (currentTrack == filename && isBgmLoaded && ma_sound_is_playing(&bgmSound)) return;

    stopMusic(); 

    currentTrack = filename;
    std::string fullPath = audioFolderPath + filename;
    debug_audio("Mencoba memutar file: " + fullPath);

    ma_result result = ma_sound_init_from_file(&engine, fullPath.c_str(), MA_SOUND_FLAG_STREAM, NULL, NULL, &bgmSound);
    if (result == MA_SUCCESS) {
        isBgmLoaded = true;
        ma_sound_set_looping(&bgmSound, MA_TRUE);
        ma_sound_start(&bgmSound);
        debug_audio("SUCCESS: Lagu mulai diputar ke speaker!");
    } else {
        debug_audio("ERROR: File MP3 gagal dimuat. Kode error: " + std::to_string(result));
    }
}

void MusicManager::stopMusic() {
    if (!isEngineInitialized || !isBgmLoaded) return;
    ma_sound_stop(&bgmSound);
    ma_sound_uninit(&bgmSound);
    isBgmLoaded = false;
    currentTrack = "";
}

void MusicManager::pauseMusic() {
    if (!isEngineInitialized || !isBgmLoaded) return;
    ma_sound_stop(&bgmSound);
}

void MusicManager::setVolume(int volume) {
    if (!isEngineInitialized) return;
    float vol = static_cast<float>(volume) / 100.0f;
    ma_engine_set_volume(&engine, vol);
}

// --- FUNGSI BARU UNTUK EFEK SUARA (SFX) ---
void MusicManager::playSfx(const std::string& filename) {
    if (!isEngineInitialized) return;

    std::string fullPath = audioFolderPath + filename;
    
    // Uji coba apakah file bisa dibaca dengan cara lain
    ma_result result = ma_engine_play_sound(&engine, fullPath.c_str(), NULL);
    
    if (result != MA_SUCCESS) {
        debug_audio("DEBUG: Gagal memutar SFX. Kode Error: " + std::to_string(result) + " Path: " + fullPath);
    } else {
        debug_audio("SUCCESS: SFX berhasil ditembakkan: " + fullPath);
    }
}

void MusicManager::startTypingSfx(const std::string& filename) {
    if (!isEngineInitialized) return;
    if (isTypingLoaded && ma_sound_is_playing(&typingSound)) return;

    stopTypingSfx();

    std::string fullPath = audioFolderPath + filename;
    ma_result result = ma_sound_init_from_file(&engine, fullPath.c_str(), 0, NULL, NULL, &typingSound);
    if (result == MA_SUCCESS) {
        isTypingLoaded = true;
        ma_sound_set_looping(&typingSound, MA_TRUE);
        ma_sound_start(&typingSound);
        debug_audio("SUCCESS: Typing SFX started: " + fullPath);
    } else {
        debug_audio("ERROR: Typing SFX gagal dimuat. Error: " + std::to_string(result));
    }
}

void MusicManager::stopTypingSfx() {
    if (!isEngineInitialized || !isTypingLoaded) return;
    ma_sound_stop(&typingSound);
    ma_sound_uninit(&typingSound);
    isTypingLoaded = false;
    debug_audio("SUCCESS: Typing SFX stopped.");
}