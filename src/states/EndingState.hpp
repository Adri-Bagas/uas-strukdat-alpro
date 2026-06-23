#pragma once
#include "GameState.hpp"
#include <string>
#include <vector>

class EndingState : public GameState {
private:
    std::string ending_id;
    std::vector<std::string> ending_text;

public:
    EndingState(GameEngine* eng, const std::string& ending_id);
    ~EndingState() override = default;

    void handle_input(int ch) override;
    void update() override;
    void render() override;
};
