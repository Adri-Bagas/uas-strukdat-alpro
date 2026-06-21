#pragma once
#include <vlcpp/vlc.hpp>
#include <string>

class MusicManager {
private:
    VLC::Instance instance;
    VLC::MediaPlayer mediaPlayer;
    std::string currentTrack;
    std::string audioFolderPath;

public:
    MusicManager();
    ~MusicManager() = default;

    // Fungsi utama 
    void playMusic(const std::string& filename);
    void stopMusic();
    void pauseMusic();
    void setVolume(int volume); // Skala 0 - 100
};