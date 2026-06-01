#pragma once

// Forward declare to prevent cyclic include
class GameEngine; 

class GameState {
protected:
    // Pointer back to main engine to access Models/Views and trigger state changes
    GameEngine* engine; 

public:
    GameState(GameEngine* eng) : engine(eng) {}
    virtual ~GameState() = default;

    virtual void handle_input(int ch) = 0;
    virtual void update() = 0;
    virtual void render() = 0;

    // Optional lifecycle hooks
    virtual void on_enter() {} 
    virtual void on_exit() {}
};