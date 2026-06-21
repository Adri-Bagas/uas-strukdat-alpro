#include <fstream>
#include "MusicManager.hpp"
#include <iostream>

MusicManager::MusicManager() 
    : instance(VLC::Instance(0, nullptr)), 
      mediaPlayer(instance),
      audioFolderPath("data/audio/") // tempat berkas .mp3
{}

void MusicManager::playMusic(const std::string& filename) {

    if (currentTrack == filename && mediaPlayer.isPlaying()) {
        return;
    }

    currentTrack = filename;
    std::string fullPath = audioFolderPath + filename;

    try {
        // Buat objek media 
        auto media = VLC::Media(instance, fullPath, VLC::Media::FromPath);
        
        // Masukkan media ke player dan mainkan
        mediaPlayer.setMedia(media);
        mediaPlayer.play();
        
        std::cout << "[MUSIC] Playing: " << filename << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[ERROR MUSIC] Gagal memutar lagu: " << e.what() << std::endl;
    }
}

void MusicManager::stopMusic() {
    if (mediaPlayer.isPlaying()) {
        mediaPlayer.stop();
    }
}

void MusicManager::pauseMusic() {
    mediaPlayer.pause();
}

void MusicManager::setVolume(int volume) {
    // libvlc menerima volume dalam skala int (0 - 100)
    mediaPlayer.setVolume(volume);
}