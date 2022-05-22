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

class ThreesGame {
public:
    void Run() {
        Initialize();
        grid.Reset();

        while (true) {
            Update();
        }
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

        if (buttonsDown & PSP_CTRL_UP) {
            grid.ApplyMove(Direction::UP);
        } else if (buttonsDown & PSP_CTRL_RIGHT) {
            grid.ApplyMove(Direction::RIGHT);
        } else if (buttonsDown & PSP_CTRL_DOWN) {
            grid.ApplyMove(Direction::DOWN);
        } else if (buttonsDown & PSP_CTRL_LEFT) {
            grid.ApplyMove(Direction::LEFT);
        } else if (buttonsDown & PSP_CTRL_START) {
            grid.Reset();
        }

        buttonState = padData.Buttons;
    }

    void Draw() {
        sceGuStart(GU_DIRECT, list);
		sceGuClear(GU_COLOR_BUFFER_BIT);

        for (int x = 0; x < 4; x++) {
            for (int y = 0; y < 4; y++) {
                drawCard(grid.Get(x, y), 112 + x * 64, 8 + y * 64);
            }
        }

		sceGuFinish();
		sceGuSync(0, 0);

		sceDisplayWaitVblankStart();
		sceGuSwapBuffers();
    }

    void drawCard(int index, int x, int y) {
        typedef struct {
            float s, t;
            unsigned int c;
            float x, y, z;
        } VERT;

        VERT* v = (VERT*)sceGuGetMemory(sizeof(VERT) * 2);

        int tx = (index % 4) * 128;
        int ty = (index / 4) * 128;

        VERT* v0 = &v[0];
        VERT* v1 = &v[1];
        
        v0->s = (float)(tx);
        v0->t = (float)(ty);
        v0->c = 0xFFFFFFFF;
        v0->x = (float)(x);
        v0->y = (float)(y);
        v0->z = 0.0f;

        v1->s = (float)(tx + 128);
        v1->t = (float)(ty + 128);
        v1->c = 0xFFFFFFFF;
        v1->x = (float)(x + 64);
        v1->y = (float)(y + 64);
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
};