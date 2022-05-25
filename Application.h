#pragma once

#include "ThreesGame.h"
#include "input.h"

enum Screen {
    Game,
    PauseMenu,
    GameComplete,
    Settings
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
    }


    static Application instance;
private:
    Screen currentScreen;
    
    ThreesGame game;

    int pauseMenuIndex = 0;

    void DoPauseMenu();

    void DoGameOverScreen();
};