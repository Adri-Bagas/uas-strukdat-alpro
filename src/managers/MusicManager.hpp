#pragma once
#include <string>
#include "../utils/miniaudio.h"

class MusicManager {
private:
    ma_engine engine;
    ma_sound bgmSound;
    std::string currentTrack;
    std::string audioFolderPath;
    
    // Keamanan status untuk mencegah error memori
    bool isEngineInitialized;
    bool isBgmLoaded;
    
    ma_sound typingSound;
    bool isTypingLoaded;

public:
    MusicManager();
    ~MusicManager();

    void playMusic(const std::string& filename);
    void stopMusic();
    void pauseMusic();
    void setVolume(int volume); // Skala 0 - 100
    
    // Fungsi baru untuk Efek Suara instan
    void playSfx(const std::string& filename); 
    
    void startTypingSfx(const std::string& filename);
    void stopTypingSfx();
};