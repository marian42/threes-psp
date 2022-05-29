#pragma once

#include "ThreesGame.h"
#include "input.h"
#include "savedata.h"
#include <pspkernel.h>

enum Screen {
    Game,
    PauseMenu,
    GameComplete,
    Settings,
    Stats,
    About,
    Options
};

class Application {

public:
    void Initialize();
    
    void Run();

    void Update();

    void SwitchScreen(Screen newScreen) {
        this->currentScreen = newScreen;

        switch (newScreen) {
            case Screen::Options:
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

    GameOptions* GetOptions() {
        return &savedata.options;
    }

private:
    Screen currentScreen;
    
    ThreesGame game;

    int menuIndex = 0;

    void DoPauseMenu();

    void DoGameOverScreen();

    void DoStatsScreen();
    
    void DoAboutScreen();

    void DoOptionsScreen();

    Savedata savedata;
};