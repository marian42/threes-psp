#include "Application.h"

#include <pspkernel.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspdisplay.h>
#include <cstdio>

#include "text.h"

#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)
#define FRAME_SIZE (BUF_WIDTH * SCR_HEIGHT * 4)
#define ZBUF_SIZE (BUF_WIDTH SCR_HEIGHT * 2)

Application Application::instance;

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

            if (PSPInput::GetButtonDown(PSP_CTRL_START)) {
                SwitchScreen(Screen::PauseMenu);
                pauseMenuIndex = 0;
            }
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
        "About"
    };
    constexpr int PAUSE_MENU_ITEM_COUNT = 5;

    if (PSPInput::GetButtonDown(PSP_CTRL_START | PSP_CTRL_CIRCLE)) {
        SwitchScreen(Screen::Game);
    }

    if (PSPInput::GetButtonDown(PSP_CTRL_UP)) {
        pauseMenuIndex = (pauseMenuIndex + PAUSE_MENU_ITEM_COUNT - 1) % PAUSE_MENU_ITEM_COUNT;
    }
    if (PSPInput::GetButtonDown(PSP_CTRL_DOWN)) {
        pauseMenuIndex = (pauseMenuIndex + 1) % PAUSE_MENU_ITEM_COUNT;
    }
    
    if (PSPInput::GetButtonDown(PSP_CTRL_CROSS)) {
        switch (pauseMenuIndex) {
            case 0:
                SwitchScreen(Screen::Game);
                break;
            case 1:
                game.NewGame();
                SwitchScreen(Screen::Game);
                break;
        }
    }

    useSpritesheet();
    drawString("Threes", 240, 10, HEXCOLOR(0x000000), TextAlignment::Center);
    for (int i = 0; i < PAUSE_MENU_ITEM_COUNT; i++) {
        drawString(PAUSE_MENU[i], 240, 60 + 32 * i, (pauseMenuIndex == i) ? HEXCOLOR(0x01CCFE) : HEXCOLOR(0x75555B), TextAlignment::Center);
    }
}

void Application::DoGameOverScreen() {
    if (PSPInput::GetButtonDown(PSP_CTRL_START | PSP_CTRL_CIRCLE | PSP_CTRL_CROSS)) {
        this->game.NewGame();
        SwitchScreen(Screen::Game);
    }

    useSpritesheet();
    drawString("Game over", 240, 10, HEXCOLOR(0x000000), TextAlignment::Center);

    drawString("Score:", 120, 90, HEXCOLOR(0x018BAA), TextAlignment::Left);
    
    char scoreString[10];

    sprintf(scoreString, "%d", this->game.GetScore());
    drawString(scoreString, 360, 90, HEXCOLOR(0xFF002E), TextAlignment::Right);

    
    drawString("Highscore:", 120, 130, HEXCOLOR(0x018BAA), TextAlignment::Left);
    drawString(scoreString, 360, 130, HEXCOLOR(0xFF002E), TextAlignment::Right);
}
