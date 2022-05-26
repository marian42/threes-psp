#pragma once

#include "ThreesGame.h"
#include "input.h"

enum Screen {
    Game,
    PauseMenu,
    GameComplete,
    Settings
};


struct Stats {
    int highscore = 0;
};


class Application {

public:
    void Initialize();
    
    void Run() {
        Initialize();
        this->currentScreen = Screen::Game;
        game.NewGame();

        while (true) {
            Update();
        }
    }

    void Update();

    void SwitchScreen(Screen newScreen) {
        this->currentScreen = newScreen;

        switch (newScreen) {
            case Screen::PauseMenu:
                pauseMenuIndex = 0;
                break;
            case Screen::GameComplete:
                OnGameComplete();
                break;
        }
    }

    static Application instance;

    Stats stats;
private:
    Screen currentScreen;
    
    ThreesGame game;

    int pauseMenuIndex = 0;

    void DoPauseMenu();

    void DoGameOverScreen();

    void OnGameComplete();
};