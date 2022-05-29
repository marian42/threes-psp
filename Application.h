#pragma once

#include "ThreesGame.h"
#include "input.h"
#include "savedata.h"

enum Screen {
    Game,
    PauseMenu,
    GameComplete,
    Settings,
    Stats,
    About
};

class Application {

public:
    void Initialize();
    
    void Run() {
        Initialize();
        Load();
        this->currentScreen = Screen::Game;
        if (this->savedata.containsInProgressGame) {
            game.LoadGame(&this->savedata);
        } else {
            game.NewGame();
        }

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

    void Save(bool saveCurrentGame);

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

    void DoStatsScreen();
    
    void DoAboutScreen();

    Savedata savedata;
};