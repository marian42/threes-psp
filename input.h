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
    
private:
    struct SceCtrlData padData;
    int buttonState = 0;
    int buttonsDown;

    static PSPInput instance;
};

PSPInput PSPInput::instance;