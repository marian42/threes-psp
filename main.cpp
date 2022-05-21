#include <pspkernel.h>
#include <pspdebug.h>
#include "ThreesGame.h"
#include "utils.h"
 
PSP_MODULE_INFO("Tutorial", 0, 1, 0);



int exit_callback(int arg1, int arg2, void* common){
	sceKernelExitGame();
	return 0;
}
 
int CallbackThread(SceSize args, void* argp) {
	int cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);
	sceKernelSleepThreadCB();
 
	return 0;
}
 
int SetupCallbacks(void) {
	int thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
	if (thid >= 0) {
		sceKernelStartThread(thid, 0, 0);
	}
	return thid;
}
 
int main() {
	SetupCallbacks();
	initializeRNG();
	
	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

	ThreesGame game;
    game.Run();

	return 0;
}