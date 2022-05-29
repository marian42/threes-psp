#pragma once

#include "ThreesGame.h"
#include "input.h"

enum Screen {
    Game,
    PauseMenu,
    GameComplete,
    Settings
};


struct Statistics {
    int highscore = 0;
    int gamesPlayed = 0;
    int moves = 0;
    int secondsPlayed = 0;
    int highestPiece = 0;
};

struct Savedata {
    Statistics stats;
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
        }
    }

    static Application instance;

    static SceKernelUtilsMt19937Context sceKernelUtilsMt19937Context;

    void Save();

    void Load();

    Statistics* GetStatistics() {
        return &savedata.stats;
    }

    Savedata* GetSavedata() {
        return &savedata;
    }

private:
    Screen currentScreen;
    
    ThreesGame game;

    int menuIndex = 0;

    void DoPauseMenu();

    void DoGameOverScreen();

    Savedata savedata;
};