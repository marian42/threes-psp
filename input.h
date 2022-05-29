#pragma once

#include <pspctrl.h>

class PSPInput {
public:
    static void Update() {
        sceCtrlReadBufferPositive(&instance.padData,1);
        instance.buttonsDown = ~instance.buttonState & instance.padData.Buttons;
        instance.buttonState = instance.padData.Buttons;
    }

    static bool GetButtonPressed(int button) {
        return instance.buttonState & button;
    }

    static bool GetButtonDown(int button) {
        return instance.buttonsDown & button;
    }

    static float GetAnalogX() {
        return instance.padData.Lx / 128.0f - 1.0f;
    }

    static float GetAnalogY() {
        return instance.padData.Ly / 128.0f - 1.0f;
    }
    
private:
    struct SceCtrlData padData;
    int buttonState = 0;
    int buttonsDown;

    static PSPInput instance;
};