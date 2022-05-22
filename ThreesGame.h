#pragma once

#include <pspkernel.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspdisplay.h>
#include <pspdebug.h>
#include "ThreesGrid.h"
#include <pspctrl.h>

#include <math.h>

#include "img/cards.c"

#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)
#define FRAME_SIZE (BUF_WIDTH * SCR_HEIGHT * 4)

static unsigned int __attribute__((aligned(16))) list[262144];

constexpr int GRID_X = 112;
constexpr int GRID_Y = 8;

class ThreesGame {
public:
    void Run() {
        Initialize();
        NewGame();

        while (true) {
            Update();
        }
    }

    void NewGame() {
        grid.Reset();
        this->previewAmount = 0.0f;
    }

    void Update() {
        UpdateGameplay();
        Draw();
    }

    void Initialize() {
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
        sceGuTexImage(0, 512, 512, 512, cards);
        sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);
        sceGuTexEnvColor(0x000000);
        sceGuClearColor(0xE0E6C7);
        sceGuTexOffset(0.0f, 0.0f);
        sceGuTexScale(1.0f / 512.0f, 1.0f / 512.0f);
        sceGuTexWrap(GU_REPEAT, GU_REPEAT);
        sceGuTexFilter(GU_LINEAR, GU_LINEAR);
        sceGuFinish();
        sceGuSync(0,0);
        sceGuDisplay(GU_TRUE);
    }

    void UpdateGameplay() {
        sceCtrlReadBufferPositive(&padData,1);
        int buttonsDown = ~buttonState & padData.Buttons;

        bool movementButtonPressed = false;
        Direction directionPressed;

        if (padData.Buttons & PSP_CTRL_UP) {
            directionPressed = Direction::UP;
            movementButtonPressed = true;
        } else if (padData.Buttons & PSP_CTRL_RIGHT) {
            directionPressed = Direction::RIGHT;
            movementButtonPressed = true;
        } else if (padData.Buttons & PSP_CTRL_DOWN) {
            directionPressed = Direction::DOWN;
            movementButtonPressed = true;
        } else if (padData.Buttons & PSP_CTRL_LEFT) {            
            directionPressed = Direction::LEFT;
            movementButtonPressed = true;
        } else if (buttonsDown & PSP_CTRL_START) {
            grid.Reset();
        }

        if (this->waitForButtonRelease) {
            movementButtonPressed = false;
            if ((padData.Buttons & (PSP_CTRL_DOWN | PSP_CTRL_LEFT | PSP_CTRL_UP | PSP_CTRL_RIGHT)) == 0) {
                this->waitForButtonRelease = false;
            }
        }

        if (!movementButtonPressed) {
            this->previewAmount -= 0.1;
        } else if (this->previewDirection == directionPressed) {
            this->previewAmount += 0.08;
            if (this->previewAmount > 1.0 && this->grid.IsMovePossible(this->previewDirection)) {
                this->grid.ApplyMove(this->previewDirection);
                this->lastMoveDirection = this->previewDirection;
                this->previewAmount = 0;
                this->waitForButtonRelease = true;
            }
        } else if (this->previewAmount <= 0) {
            this->previewDirection = directionPressed;
        } else {
            this->previewAmount -= 0.2;
        }

        if (this->previewAmount < 0) {
            this->previewAmount = 0;
        } else if (this->previewAmount > 1.0) {
            this->previewAmount = 1.0;
        }

        buttonState = padData.Buttons;
    }

    void Draw() {
        sceGuStart(GU_DIRECT, list);
		sceGuClear(GU_COLOR_BUFFER_BIT);

        for (int x = 0; x < 4; x++) {
            for (int y = 0; y < 4; y++) {
                drawSlot(GRID_X + x * 64, GRID_Y + y * 64);
            }
        }

        for (int x = 0; x < 4; x++) {
            for (int y = 0; y < 4; y++) {
                if (this->grid.Get(x, y) != 0 && this->grid.GetPreview(this->previewDirection, x, y) == MoveType::ReceiveMerge) {
                    drawCard(x, y);
                }
            }
        }

        for (int x = 0; x < 4; x++) {
            for (int y = 0; y < 4; y++) {
                if (this->grid.Get(x, y) != 0 && this->grid.GetPreview(this->previewDirection, x, y) != MoveType::ReceiveMerge) {
                    drawCard(x, y);
                }
            }
        }

		sceGuFinish();
		sceGuSync(0, 0);

		sceDisplayWaitVblankStart();
		sceGuSwapBuffers();
    }

    void drawCard(int cardX, int cardY) {
        typedef struct {
            float s, t;
            unsigned int c;
            float x, y, z;
        } VERT;

        float moveX = this->previewDirection == Direction::RIGHT ? 1 : (this->previewDirection == Direction::LEFT ? -1 : 0);
        float moveY = this->previewDirection == Direction::DOWN ? 1 : (this->previewDirection == Direction::UP ? -1 : 0);

        MoveType movementType = this->grid.GetPreview(this->previewDirection, cardX, cardY);

        float x = cardX;
        float y = cardY;
        float sizeX = 1.0f;
        float sizeY = 1.0f;

        if (movementType == MoveType::Move || movementType == MoveType::MoveMerge) {
            x += moveX * this->previewAmount;
            y += moveY * this->previewAmount;
        }
        
        VERT* v = (VERT*)sceGuGetMemory(sizeof(VERT) * 2);

        int index = this->grid.Get(cardX, cardY);
        int tx = (index % 4) * 128;
        int ty = (index / 4) * 128;

        VERT* v0 = &v[0];
        VERT* v1 = &v[1];
        
        v0->s = (float)(tx);
        v0->t = (float)(ty);
        v0->c = 0xFFFFFFFF;
        v0->x = GRID_X + x * 64.0f;
        v0->y = GRID_Y + y * 64.0f;
        v0->z = 0.0f;

        v1->s = (float)(tx + 128);
        v1->t = (float)(ty + 128);
        v1->c = 0xFFFFFFFF;
        v1->x = GRID_X + (x + sizeX) * 64.0f;
        v1->y = GRID_Y + (y + sizeY) * 64.0f;
        v1->z = 0.0f;

        sceGumDrawArray(GU_SPRITES, 
            GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D,
            2, 0, v
        );
    }

    
    void drawSlot(float x, float y) {
        typedef struct {
            float s, t;
            unsigned int c;
            float x, y, z;
        } VERT;

        VERT* v = (VERT*)sceGuGetMemory(sizeof(VERT) * 2);

        VERT* v0 = &v[0];
        VERT* v1 = &v[1];
        
        v0->s = 0.0f;
        v0->t = 0.0f;
        v0->c = 0xFFFFFFFF;
        v0->x = x;
        v0->y = y;
        v0->z = 0.0f;

        v1->s = 128.0f;
        v1->t = 128.0f;
        v1->c = 0xFFFFFFFF;
        v1->x = x + 64.0f;
        v1->y = y + 64.0f;
        v1->z = 0.0f;

        sceGumDrawArray(GU_SPRITES, 
            GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D,
            2, 0, v
        );
    }

    void DebugDraw() {
        pspDebugScreenInit();
        
        grid.DebugDraw(8, 7);
    }

private:
    ThreesGrid grid;

    struct SceCtrlData padData;
    int buttonState = 0;

    Direction previewDirection;
    Direction lastMoveDirection;

    float previewAmount = 0;

    bool waitForButtonRelease = false;
};