// Ini WAJIB ada agar kode internal miniaudio tereksekusi
#define MINIAUDIO_IMPLEMENTATION
#include "../utils/miniaudio.h"

#include "MusicManager.hpp"
#include <fstream>
#include <string>
#include <stdlib.h>

// KOTAK HITAM KITA: Akan mencatat diam-diam apa yang terjadi di dalam Miniaudio
void debug_audio(const std::string& msg) {
    std::ofstream dbg("debug_miniaudio.txt", std::ios::app);
    dbg << msg << "\n";
    dbg.close();
}

MusicManager::MusicManager() 
    : bgmFolderPath("data/audio/bgm/"), sfxFolderPath("data/audio/sfx/"), isEngineInitialized(false), isBgmLoaded(false), isTypingLoaded(false) 
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
        
        for (auto& pair : sfxPool) {
            for (ma_sound* snd : pair.second) {
                ma_sound_uninit(snd);
                delete snd;
            }
        }
        sfxPool.clear();
        
        ma_engine_uninit(&engine);
    }
}

void MusicManager::playMusic(const std::string& filename) {
    if (!isEngineInitialized) return;

    if (currentTrack == filename && isBgmLoaded && ma_sound_is_playing(&bgmSound)) return;

    stopMusic(); 

    currentTrack = filename;
    std::string fullPath = bgmFolderPath + filename;
    debug_audio("Mencoba memutar file: " + fullPath);

    // Menggunakan 0 (decoding to RAM) alih-alih MA_SOUND_FLAG_STREAM untuk mencegah lagu tersendat di tengah-tengah
    ma_result result = ma_sound_init_from_file(&engine, fullPath.c_str(), 0, NULL, NULL, &bgmSound);
    if (result == MA_SUCCESS) {
        isBgmLoaded = true;
        ma_sound_set_volume(&bgmSound, 0.1f); // <-- Mengatur volume BGM di angka 10
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

void MusicManager::setBgmVolume(float volume) {
    if (!isEngineInitialized || !isBgmLoaded) return;
    ma_sound_set_volume(&bgmSound, volume);
}

void MusicManager::setSfxVolume(float volume) {
    if (!isEngineInitialized) return;
    ma_engine_set_volume(&engine, volume);
}

// --- FUNGSI BARU UNTUK EFEK SUARA (SFX) ---
void MusicManager::playSfx(const std::string& filename) {
    if (!isEngineInitialized) return;

    std::string fullPath = sfxFolderPath + filename;
    
    ma_sound* availableSound = nullptr;
    
    // Cari sound yang sedang tidak dimainkan di dalam pool
    if (sfxPool.find(filename) != sfxPool.end()) {
        for (ma_sound* snd : sfxPool[filename]) {
            if (!ma_sound_is_playing(snd)) {
                availableSound = snd;
                break;
            }
        }
    }
    
    // Jika tidak ada sound yang tersedia, atau pool belum ada, kita buat instance baru
    if (!availableSound) {
        // Batasi pool maksimal 5 overlapping sound per SFX untuk hemat memori
        if (sfxPool[filename].size() >= 5) {
            // Gunakan paksa sound pertama jika sudah penuh (paling lama)
            availableSound = sfxPool[filename][0];
        } else {
            ma_sound* newSound = new ma_sound;
            ma_result initResult = ma_sound_init_from_file(&engine, fullPath.c_str(), 0, NULL, NULL, newSound);
            if (initResult == MA_SUCCESS) {
                sfxPool[filename].push_back(newSound);
                availableSound = newSound;
                debug_audio("INFO: Menambahkan salinan baru ke SFX Pool untuk " + filename + ". Total: " + std::to_string(sfxPool[filename].size()));
            } else {
                debug_audio("ERROR: Gagal load SFX untuk pool: " + fullPath);
                delete newSound;
                return;
            }
        }
    }
    
    if (availableSound) {
        ma_sound_seek_to_pcm_frame(availableSound, 0);
        ma_result result = ma_sound_start(availableSound);
        if (result != MA_SUCCESS) {
            debug_audio("DEBUG: Gagal memutar SFX. Kode Error: " + std::to_string(result) + " Path: " + fullPath);
        }
    }
}

void MusicManager::startTypingSfx(const std::string& filename) {
    if (!isEngineInitialized) return;
    
    // Jika belum dimuat ke memori, muat sekali saja
    if (!isTypingLoaded) {
        std::string fullPath = sfxFolderPath + filename;
        ma_result result = ma_sound_init_from_file(&engine, fullPath.c_str(), 0, NULL, NULL, &typingSound);
        if (result == MA_SUCCESS) {
            isTypingLoaded = true;
            ma_sound_set_looping(&typingSound, MA_TRUE);
        } else {
            debug_audio("ERROR: Typing SFX gagal dimuat. Error: " + std::to_string(result));
            return;
        }
    }
    
    // Putar ulang jika sedang tidak berputar
    if (isTypingLoaded && !ma_sound_is_playing(&typingSound)) {
        ma_sound_start(&typingSound);
        debug_audio("SUCCESS: Typing SFX started.");
    }
}

void MusicManager::stopTypingSfx() {
    if (!isEngineInitialized || !isTypingLoaded) return;
    if (ma_sound_is_playing(&typingSound)) {
        ma_sound_stop(&typingSound);
        debug_audio("SUCCESS: Typing SFX stopped.");
    }
}