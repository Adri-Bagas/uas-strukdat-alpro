#pragma once
#include <string>
#include <map>
#include <vector>
#include "../utils/miniaudio.h"

class MusicManager {
private:
    ma_engine engine;
    ma_sound bgmSound;
    ma_sound typingSound; 
    std::string bgmFolderPath;
    std::string sfxFolderPath;
    std::string currentTrack;
    
    // Pool untuk menyimpan beberapa salinan SFX di memori agar bisa dioverlap tanpa baca disk
    std::map<std::string, std::vector<ma_sound*>> sfxPool;
    
    // Keamanan status untuk mencegah error memori
    bool isEngineInitialized;
    bool isBgmLoaded;
    
    bool isTypingLoaded;

public:
    MusicManager();
    ~MusicManager();

    void playMusic(const std::string& filename);
    void stopMusic();
    void pauseMusic();
    void setVolume(int volume); // Skala 0 - 100 (Master Volume / SFX Baseline)
    void setBgmVolume(float volume); // Skala 0.0f - 1.0f
    void setSfxVolume(float volume); // Skala 0.0f - 1.0f
    
    // Fungsi baru untuk Efek Suara instan
    void playSfx(const std::string& filename); 
    
    void startTypingSfx(const std::string& filename);
    void stopTypingSfx();
};