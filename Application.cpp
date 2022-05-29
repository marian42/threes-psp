#include "Application.h"

#include <pspkernel.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspdisplay.h>
#include <cstdio>
#include <psputility.h>

#include "text.h"
#include "img/icon0.c"

#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)
#define FRAME_SIZE (BUF_WIDTH * SCR_HEIGHT * 4)
#define ZBUF_SIZE (BUF_WIDTH SCR_HEIGHT * 2)

Application Application::instance;

SceKernelUtilsMt19937Context Application::sceKernelUtilsMt19937Context;

void Application::Initialize() {
    sceGuInit();
    sceGuStart(GU_DIRECT, list);
    sceGuDrawBuffer(GU_PSM_8888,(void*)0,BUF_WIDTH);
    sceGuDispBuffer(SCR_WIDTH,SCR_HEIGHT,(void*)0x88000,BUF_WIDTH);
    sceGuDepthBuffer((void*)0x110000,BUF_WIDTH);
    sceGuOffset(2048 - (SCR_WIDTH/2),2048 - (SCR_HEIGHT/2));
    sceGuViewport(2048,2048,SCR_WIDTH,SCR_HEIGHT);
    sceGuDepthRange(0xc350,0x2710);
    sceGuScissor(0,0,SCR_WIDTH,SCR_HEIGHT);
    sceGuEnable(GU_SCISSOR_TEST);
    sceGuDisable(GU_DEPTH_TEST);
    sceGuShadeModel(GU_SMOOTH);
    sceGuEnable(GU_BLEND);
    sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
    sceGuEnable(GU_TEXTURE_2D);
    sceGuTexMode(GU_PSM_8888, 0, 0, 0);
    sceGuTexImage(0, 256, 256, 256, cards);
    sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);
    sceGuTexEnvColor(0x000000);
    sceGuClearColor(0xE0E6C7);
    sceGuTexOffset(0.0f, 0.0f);
    sceGuTexScale(1.0f / 256.0f, 1.0f / 256.0f);
    sceGuTexWrap(GU_REPEAT, GU_REPEAT);
    sceGuTexFilter(GU_NEAREST, GU_NEAREST);
    sceGuFinish();
    sceGuSync(0,0);
    sceGuDisplay(GU_TRUE);
}

void Application::Update() {
    sceGuStart(GU_DIRECT, list);
    sceGuClear(GU_COLOR_BUFFER_BIT);

    PSPInput::Update();

    switch (currentScreen) {
        case Screen::Game:
            game.Update();
            if (PSPInput::GetButtonDown(PSP_CTRL_TRIANGLE)) {
                SwitchScreen(Screen::GameComplete);
            }
            break;
        case Screen::PauseMenu:
            DoPauseMenu();
            break;
        case Screen::GameComplete:
            DoGameOverScreen();
            break;
    }

    sceGuFinish();
    sceGuSync(0, 0);

    sceDisplayWaitVblankStart();
    sceGuSwapBuffers();
}

void Application::DoPauseMenu() {
    static const char* PAUSE_MENU[] = {
        "Resume",
        "Restart",
        "Options",
        "Stats",
        "About",
        "Quit"
    };
    constexpr int PAUSE_MENU_ITEM_COUNT = 6;

    if (PSPInput::GetButtonDown(PSP_CTRL_START | PSP_CTRL_CIRCLE)) {
        SwitchScreen(Screen::Game);
    }

    if (PSPInput::GetButtonDown(PSP_CTRL_UP)) {
        menuIndex = (menuIndex + PAUSE_MENU_ITEM_COUNT - 1) % PAUSE_MENU_ITEM_COUNT;
    }
    if (PSPInput::GetButtonDown(PSP_CTRL_DOWN)) {
        menuIndex = (menuIndex + 1) % PAUSE_MENU_ITEM_COUNT;
    }
    
    if (PSPInput::GetButtonDown(PSP_CTRL_CROSS)) {
        switch (menuIndex) {
            case 0:
                SwitchScreen(Screen::Game);
                break;
            case 1:
                game.NewGame();
                SwitchScreen(Screen::Game);
                break;
            case 5:
                sceKernelExitGame();
                break;
        }
    }

    useSpritesheet();
    drawString("Threes", 240, 10, HEXCOLOR(0x000000), TextAlignment::Center);
    for (int i = 0; i < PAUSE_MENU_ITEM_COUNT; i++) {
        drawString(PAUSE_MENU[i], 240, 60 + 32 * i, (menuIndex == i) ? HEXCOLOR(0x01CCFE) : HEXCOLOR(0x75555B), TextAlignment::Center);
    }
}

void Application::DoGameOverScreen() {
    if (PSPInput::GetButtonDown(PSP_CTRL_START | PSP_CTRL_CIRCLE | PSP_CTRL_CROSS)) {
        this->game.NewGame();
        SwitchScreen(Screen::Game);
    }

    useSpritesheet();
    drawString("Game Over", 240, 20, HEXCOLOR(0x000000), TextAlignment::Center);

    drawString("Score:", 120, 90, HEXCOLOR(0x018BAA), TextAlignment::Left);
    
    int score = this->game.GetScore();

    char scoreString[10];

    sprintf(scoreString, "%d", score);
    drawString(scoreString, 360, 90, HEXCOLOR(0xFF002E), TextAlignment::Right);

    char highscoreString[10];
    sprintf(highscoreString, "%d", this->stats.highscore);
    
    drawString("Highscore:", 120, 130, HEXCOLOR(0x018BAA), TextAlignment::Left);
    drawString(highscoreString, 360, 130, score == this->stats.highscore ? HEXCOLOR(0xFF002E) : HEXCOLOR(0x7E7E7E), TextAlignment::Right);
}


void Application::OnGameComplete() {
    if (this->game.GetScore() > this->stats.highscore) {
        this->stats.highscore = this->game.GetScore();
    }
}

char nameOptions[][20] = { "0000","0001","0002", "0003", "0004", "" };
constexpr int SAVEDATASIZE = 4;

bool RunSaveDataUtility(PspUtilitySavedataMode mode) {
    SceUtilitySavedataParam dialog;
    PspUtilitySavedataListSaveNewData newData;

	memset(&dialog, 0, sizeof(SceUtilitySavedataParam));
	dialog.base.size = sizeof(SceUtilitySavedataParam);

	dialog.base.language = PSP_SYSTEMPARAM_LANGUAGE_ENGLISH;
	dialog.base.buttonSwap = PSP_UTILITY_ACCEPT_CROSS;
	dialog.base.graphicsThread = 0x11;
	dialog.base.accessThread = 0x13;
	dialog.base.fontThread = 0x12;
	dialog.base.soundThread = 0x10;

	dialog.mode = mode;
	dialog.overwrite = 1;

	strcpy(dialog.gameName, "THREES111");
	strcpy(dialog.saveName, "0000");

	dialog.saveNameList = nameOptions;

	strcpy(dialog.fileName, "DATA.BIN");

	dialog.dataBuf = malloc(SAVEDATASIZE);
    *((int*)dialog.dataBuf) = Application::instance.stats.highscore;
	dialog.dataBufSize = SAVEDATASIZE;
	dialog.dataSize = SAVEDATASIZE;
    
	if (mode == PSP_UTILITY_SAVEDATA_LISTSAVE || mode == PSP_UTILITY_SAVEDATA_AUTOSAVE)
	{
        strcpy(dialog.sfoParam.title, "Threes Savegame");

        sprintf(dialog.sfoParam.savedataTitle, "Highscore: %d", Application::instance.stats.highscore);
        sprintf(dialog.sfoParam.detail, "%d games played", Application::instance.stats.gamesPlayed);
        
        dialog.sfoParam.parentalLevel = 1;
        
        dialog.icon1FileData.buf = NULL;
        dialog.icon1FileData.bufSize = 0;
        dialog.icon1FileData.size = 0;

        dialog.pic1FileData.buf = NULL;
        dialog.pic1FileData.bufSize = 0;
        dialog.pic1FileData.size = 0;

        dialog.icon0FileData.buf = icon0;
        dialog.icon0FileData.bufSize = size_icon0;
        dialog.icon0FileData.size = size_icon0;
        
        dialog.snd0FileData.buf = NULL;
        dialog.snd0FileData.bufSize = 0;
        dialog.snd0FileData.size = 0;

        dialog.newData = &newData;
	}

    sceGuFinish();
    sceGuSync(0, 0);
    sceDisplayWaitVblankStart();
    sceGuSwapBuffers();
	
    sceUtilitySavedataInitStart(&dialog);

    bool success = false;
    bool done = false;

    while (true) {        
        sceGuStart(GU_DIRECT, list);
        sceGuClear(GU_COLOR_BUFFER_BIT);
        
        useSpritesheet();

        if (mode == PSP_UTILITY_SAVEDATA_LISTSAVE || mode == PSP_UTILITY_SAVEDATA_AUTOSAVE) {
            drawString("Saving progress...", 240, 120, HEXCOLOR(0x01CCFE), TextAlignment::Center);
        } else {
            drawString("Loading progress...", 240, 120, HEXCOLOR(0x01CCFE), TextAlignment::Center);
        }

        sceGuFinish();
        sceGuSync(0, 0);

        switch(sceUtilitySavedataGetStatus()) {

        case PSP_UTILITY_DIALOG_VISIBLE:
            sceUtilitySavedataUpdate(1);
            break;

        case PSP_UTILITY_DIALOG_QUIT:
            sceUtilitySavedataShutdownStart();
            success = true;
            break;
            
        case PSP_UTILITY_DIALOG_FINISHED:
            done = true;
            break;
                
        case PSP_UTILITY_DIALOG_NONE:
            done = true;
            break;
        }

        if (done) {
            break;
        }

        sceDisplayWaitVblankStart();
        sceGuSwapBuffers();
    }

    if (mode ==  PSP_UTILITY_SAVEDATA_LISTLOAD || mode == PSP_UTILITY_SAVEDATA_AUTOLOAD) {
        Application::instance.test = *((int*)dialog.dataBuf);        
    }

    return dialog.base.result == 0;
}

void Application::Load() {
    RunSaveDataUtility(PSP_UTILITY_SAVEDATA_AUTOLOAD);
}

void Application::Save() {    
    RunSaveDataUtility(PSP_UTILITY_SAVEDATA_AUTOLOAD);
}