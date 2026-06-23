#pragma once
#include "GameState.hpp"
#include <string>
#include <vector>

class GameOverState : public GameState {
private:
    std::string title;
    std::string subtitle;
    int selection_index = 0;
    std::vector<std::string> options = {
        "Kembali ke Menu Utama",
        "Keluar Game"
    };

public:
    GameOverState(GameEngine* eng, const std::string& title = "GAME OVER", const std::string& subtitle = "Perjalananmu telah berakhir.");
    ~GameOverState() override = default;

    void handle_input(int ch) override;
    void update() override;
    void render() override;
};
