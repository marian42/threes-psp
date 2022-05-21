#pragma once

#include <pspkernel.h>
#include <pspdebug.h>
#include "ThreesGrid.h"
#include <pspctrl.h>


class ThreesGame {
public:
    void Run() {

        ThreesGrid grid;
        grid.Reset();

        struct SceCtrlData padData;
        int buttonState = 0;

        while (true) {
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

            pspDebugScreenInit();
            
            grid.DebugDraw(8, 7);
        }
    }
};