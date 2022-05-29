#include "utils.h"
#include "Application.h"
#include <pspkernel.h>
#include <pspgu.h>
#include <time.h>
#include <math.h>
#include "img/cards.c"

void initializeRNG() {
 	sceKernelUtilsMt19937Init(&Application::sceKernelUtilsMt19937Context, time(NULL));
}

void useCardsTexture() {
    sceGuTexImage(0, 256, 256, 256, cards);
}

bool GetRandomBool() {
    uint randUint = sceKernelUtilsMt19937UInt(&Application::sceKernelUtilsMt19937Context);
    return randUint < 2147483648;
}

int GetRandomInt(int maxExclusive) {
    uint randUint = sceKernelUtilsMt19937UInt(&Application::sceKernelUtilsMt19937Context);
    return randUint % maxExclusive;
}

int GetRandomInt(int minInclusive, int maxExclusive) {
    uint randUint = sceKernelUtilsMt19937UInt(&Application::sceKernelUtilsMt19937Context);
    return randUint % (maxExclusive - minInclusive) + minInclusive;
}

void shuffleArray(int* array, int arraySize, int numberOfSwaps) {    
    for (int i = 0; i < numberOfSwaps; i++) {
        int index1 = GetRandomInt(arraySize);
        int index2 = GetRandomInt(arraySize);
        int swap = array[index1];
        array[index1] = array[index2];
        array[index2] = swap;
    }
}

float ease(float value) {
    return 0.5f - 0.5f * cos(value * 3.14159f);
}