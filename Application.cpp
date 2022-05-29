#include "Application.h"

#include <pspkernel.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspdisplay.h>
#include <cstdio>
#include <cstring>
#include <psputility.h>

#include "text.h"
#include "utils.h"
#include "img/icon0.c"
#include "img/cards.c"

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
        case Screen::Stats:
            DoStatsScreen();
            break;
        case Screen::About:
            DoAboutScreen();
            break;
        case Screen::GameComplete:
            DoGameOverScreen();
            break;
        case Screen::Options:
            DoOptionsScreen();
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
        "Save",
        "Save and quit"
    };
    constexpr int PAUSE_MENU_ITEM_COUNT = 7;

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
            case 2:
                SwitchScreen(Screen::Options);
                break;
            case 3:
                SwitchScreen(Screen::Stats);
                break;
            case 4:
                SwitchScreen(Screen::About);
                break;
            case 5:
                Save(true);
                break;
            case 6:
                Save(true);
                sceKernelExitGame();
                break;
        }
    }

    useSpritesheet();
    drawString("Threes", 240, 10, HEXCOLOR(0x000000), TextAlignment::Center);
    for (int i = 0; i < PAUSE_MENU_ITEM_COUNT; i++) {
        drawString(PAUSE_MENU[i], 240, 46 + 26 * i, (menuIndex == i) ? HEXCOLOR(0x01CCFE) : HEXCOLOR(0x75555B), TextAlignment::Center);
    }

    drawGlyph(Glyph::Cross, 340, 240);
    drawString("Select", 376, 240, HEXCOLOR(0x7E7E7E), TextAlignment::Left);
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
    sprintf(highscoreString, "%d", this->GetStatistics()->highscore);
    
    drawString("Highscore:", 120, 130, HEXCOLOR(0x018BAA), TextAlignment::Left);
    drawString(highscoreString, 360, 130, score == this->GetStatistics()->highscore ? HEXCOLOR(0xFF002E) : HEXCOLOR(0x7E7E7E), TextAlignment::Right);

    drawGlyph(Glyph::Cross, 330, 240);
    drawString("New Game", 366, 240, HEXCOLOR(0x7E7E7E), TextAlignment::Left);
}


void Application::DoAboutScreen() {
    if (PSPInput::GetButtonDown(PSP_CTRL_START | PSP_CTRL_CIRCLE | PSP_CTRL_CROSS)) {
        SwitchScreen(Screen::PauseMenu);
    }

    useSpritesheet();
    drawString("About", 240, 20, HEXCOLOR(0x000000), TextAlignment::Center);

    drawString("This is an unofficial PSP implementation", 20, 60, HEXCOLOR(0x000000));
    drawString("of the Threes game. Find it at threesgame.com.", 20, 90, HEXCOLOR(0x000000));
    drawString("The source code for this project is", 20, 120, HEXCOLOR(0x000000));
    drawString("available at github.com/marian42/threes-psp.", 20, 150, HEXCOLOR(0x000000));

    drawGlyph(Glyph::Circle, 330, 240);
    drawString("Back", 366, 240, HEXCOLOR(0x7E7E7E), TextAlignment::Left);
}

void Application::DoOptionsScreen() {
    if (PSPInput::GetButtonDown(PSP_CTRL_CIRCLE)) {
        Save(true);
        SwitchScreen(Screen::PauseMenu);
    }

    constexpr int OPTIONS_MENU_ITEM_COUNT = 3;

    if (PSPInput::GetButtonDown(PSP_CTRL_UP)) {
        menuIndex = (menuIndex + OPTIONS_MENU_ITEM_COUNT - 1) % OPTIONS_MENU_ITEM_COUNT;
    }

    if (PSPInput::GetButtonDown(PSP_CTRL_DOWN)) {
        menuIndex = (menuIndex + 1) % OPTIONS_MENU_ITEM_COUNT;
    }

    bool* currentOption;
    switch (menuIndex) {
        case 0: currentOption = &savedata.options.holdToMove;
        break;

        case 1: currentOption = &savedata.options.enableAnalogStickPreview;
        break;

        case 2: currentOption = &savedata.options.showScore;
        break;
    }

    if (PSPInput::GetButtonDown(PSP_CTRL_CROSS)) {
        *currentOption = !(*currentOption);
    }

    if (PSPInput::GetButtonDown(PSP_CTRL_LEFT)) {
        *currentOption = false;
    }

    if (PSPInput::GetButtonDown(PSP_CTRL_RIGHT)) {
        *currentOption = true;
    }

    useSpritesheet();
    drawString("Options", 240, 20, HEXCOLOR(0x000000), TextAlignment::Center);

    drawString("Hold to move", 40, 60, (menuIndex == 0) ? HEXCOLOR(0x01CCFE) : HEXCOLOR(0x75555B));
    drawGlyph(savedata.options.holdToMove ? Glyph::ToggleOn : ToggleOff, 360, 60);

    drawString("Enable analog stick preview", 40, 90, (menuIndex == 1) ? HEXCOLOR(0x01CCFE) : HEXCOLOR(0x75555B));
    drawGlyph(savedata.options.enableAnalogStickPreview ? Glyph::ToggleOn : ToggleOff, 360, 90);

    drawString("Show score during gameplay", 40, 120, (menuIndex == 2) ? HEXCOLOR(0x01CCFE) : HEXCOLOR(0x75555B));
    drawGlyph(savedata.options.showScore ? Glyph::ToggleOn : ToggleOff, 360, 120);
        
    drawGlyph(Glyph::Cross, 210, 240);
    drawString("Toggle", 246, 240, HEXCOLOR(0x7E7E7E), TextAlignment::Left);
    drawGlyph(Glyph::Circle, 330, 240);
    drawString("Back", 366, 240, HEXCOLOR(0x7E7E7E), TextAlignment::Left);
}

void drawCard(int card, int x, int y) {    
    VERT* v = (VERT*)sceGuGetMemory(sizeof(VERT) * 2);

    int tx = (card % 4) * 64;
    int ty = (card / 4) * 64;

    VERT* v0 = &v[0];
    VERT* v1 = &v[1];
    
    v0->s = (float)(tx);
    v0->t = (float)(ty);
    v0->c = 0xFFFFFFFF;
    v0->x = x;
    v0->y = y;
    v0->z = 0.0f;

    v1->s = (float)(tx + 64);
    v1->t = (float)(ty + 64);
    v1->c = 0xFFFFFFFF;
    v1->x = x + 64.0f;
    v1->y = y + 64.0f;
    v1->z = 0.0f;

    sceGumDrawArray(GU_SPRITES, GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D,  2, 0, v );
}

void Application::DoStatsScreen() {
    if (PSPInput::GetButtonDown(PSP_CTRL_START | PSP_CTRL_CIRCLE | PSP_CTRL_CROSS)) {
        SwitchScreen(Screen::PauseMenu);
    }

    useSpritesheet();

    char valueString[10];

    constexpr int LEFT = 40;
    constexpr int RIGHT = 480 - LEFT;
    constexpr int SPACING = 26;

    drawString("Statistics", 240, 20, HEXCOLOR(0x000000), TextAlignment::Center);

    drawString("Highscore:", LEFT, 46 + 0 * SPACING, HEXCOLOR(0x018BAA), TextAlignment::Left);
    sprintf(valueString, "%d", this->GetStatistics()->highscore);
    drawString(valueString, RIGHT, 46 + 0 * SPACING, HEXCOLOR(0xFF002E), TextAlignment::Right);
   
    drawString("Games played:", LEFT, 46 + 1 * SPACING, HEXCOLOR(0x018BAA), TextAlignment::Left);
    sprintf(valueString, "%d", this->GetStatistics()->gamesPlayed);
    drawString(valueString, RIGHT, 46 + 1 * SPACING, HEXCOLOR(0xFF002E), TextAlignment::Right);
   
    drawString("Total moves:", LEFT, 46 + 2 * SPACING, HEXCOLOR(0x018BAA), TextAlignment::Left);
    sprintf(valueString, "%d", this->GetStatistics()->moves);
    drawString(valueString, RIGHT, 46 + 2 * SPACING, HEXCOLOR(0xFF002E), TextAlignment::Right);

    drawString("Time played:", LEFT, 46 + 3 * SPACING, HEXCOLOR(0x018BAA), TextAlignment::Left);
    int hours = this->GetStatistics()->secondsPlayed / 3600;
    int minutes = (this->GetStatistics()->secondsPlayed / 60) % 60;
    if (hours == 0)
    {
        sprintf(valueString, "%d minutes", minutes);
    } else if (hours == 1) {
        sprintf(valueString, "1 hour, %d minutes", minutes);
    } else {
        sprintf(valueString, "%d hours, %d minutes", hours, minutes);
    }
    drawString(valueString, RIGHT, 46 + 3 * SPACING, HEXCOLOR(0xFF002E), TextAlignment::Right);

    drawString("Pieces unlocked:", LEFT, 46 + 4 * SPACING, HEXCOLOR(0x018BAA), TextAlignment::Left);

    drawGlyph(Glyph::Circle, 340, 240);
    drawString("Back", 376, 240, HEXCOLOR(0x7E7E7E), TextAlignment::Left);

    useCardsTexture();
    for (int i = 3; i <= this->GetStatistics()->highestPiece; i++) {
        drawCard(i, RIGHT - this->GetStatistics()->highestPiece * 64 - 60 + i * 64, 46 + 5 * SPACING);
    }
}

char nameOptions[][20] = { "0000","0001","0002", "0003", "0004", "" };
constexpr int SAVEDATASIZE = sizeof(Savedata);

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
    memcpy(dialog.dataBuf, Application::instance.GetSavedata(), SAVEDATASIZE);
	dialog.dataBufSize = SAVEDATASIZE;
	dialog.dataSize = SAVEDATASIZE;

	if (mode == PSP_UTILITY_SAVEDATA_LISTSAVE || mode == PSP_UTILITY_SAVEDATA_AUTOSAVE)
	{
        strcpy(dialog.sfoParam.title, "Threes Savegame");

        sprintf(dialog.sfoParam.savedataTitle, "Highscore: %d", Application::instance.GetStatistics()->highscore);
        sprintf(dialog.sfoParam.detail, "%d games played", Application::instance.GetStatistics()->gamesPlayed);
        
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

    if (mode == PSP_UTILITY_SAVEDATA_AUTOLOAD || PSP_UTILITY_SAVEDATA_LISTLOAD) {        
        memcpy(Application::instance.GetSavedata(), dialog.dataBuf, SAVEDATASIZE);
    }

    return dialog.base.result == 0;
}

void Application::Load() {
    RunSaveDataUtility(PSP_UTILITY_SAVEDATA_AUTOLOAD);
}

void Application::Save(bool saveCurrentGame) {
    if (saveCurrentGame) {
        this->game.SaveGame(&this->savedata);
    }
    this->savedata.containsInProgressGame = saveCurrentGame;

    RunSaveDataUtility(PSP_UTILITY_SAVEDATA_AUTOSAVE);
}