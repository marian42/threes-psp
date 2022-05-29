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
    int gamesPlayed = 0;
};

class Application {

public:
    void Initialize();
    
    void Run() {
        Initialize();
        Load();
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
                menuIndex = 0;
                break;
            case Screen::GameComplete:
                OnGameComplete();
                break;
        }
    }

    static Application instance;

    static SceKernelUtilsMt19937Context sceKernelUtilsMt19937Context;

    Stats stats;

    void Save();

    void Load();

private:
    Screen currentScreen;
    
    ThreesGame game;

    int menuIndex = 0;

    void DoPauseMenu();

    void DoGameOverScreen();

    void OnGameComplete();
};