#pragma once

#include <pspkernel.h>
#include <pspgu.h>
#include <time.h>
#include "img/cards.c"

#define HEXCOLOR(c) (0xFF000000 | (c & 0x000000FF) << 16 | (c & 0x00FF0000) >> 16 | c & 0x0000FF00)

static SceKernelUtilsMt19937Context sceKernelUtilsMt19937Context;

static void initializeRNG() {
 	sceKernelUtilsMt19937Init(&sceKernelUtilsMt19937Context, time(NULL));
}

static void useCardsTexture() {
    sceGuTexImage(0, 256, 256, 256, cards);
}

static bool GetRandomBool() {
    uint randUint = sceKernelUtilsMt19937UInt(&sceKernelUtilsMt19937Context);
    return randUint < 2147483648;
}

static int GetRandomInt(int maxExclusive) {
    uint randUint = sceKernelUtilsMt19937UInt(&sceKernelUtilsMt19937Context);
    return randUint % maxExclusive;
}

static int GetRandomInt(int minInclusive, int maxExclusive) {
    uint randUint = sceKernelUtilsMt19937UInt(&sceKernelUtilsMt19937Context);
    return randUint % (maxExclusive - minInclusive) + minInclusive;
}

static void shuffleArray(int* array, int arraySize, int numberOfSwaps) {    
    for (int i = 0; i < numberOfSwaps; i++) {
        int index1 = GetRandomInt(arraySize);
        int index2 = GetRandomInt(arraySize);
        int swap = array[index1];
        array[index1] = array[index2];
        array[index2] = swap;
    }
}

static float min(float a, float b) {
    return a < b ? a : b;
}
 