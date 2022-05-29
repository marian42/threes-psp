#pragma once

#include <pspkernel.h>
#include <pspgu.h>
#include <time.h>
#include "img/cards.c"
#include <math.h>

#define HEXCOLOR(c) (0xFF000000 | (c & 0x000000FF) << 16 | (c & 0x00FF0000) >> 16 | c & 0x0000FF00)

void initializeRNG();

void useCardsTexture();

bool GetRandomBool();

int GetRandomInt(int maxExclusive);

int GetRandomInt(int minInclusive, int maxExclusive);

void shuffleArray(int* array, int arraySize, int numberOfSwaps);

static float min(float a, float b) {
    return a < b ? a : b;
}

static float clamp(float value) {
    return value > 1.0f ? 1.0f : (value < 0.0f ? 0.0f : value);
}

static float ease(float value) {
    return 0.5f - 0.5f * cos(value * 3.14159f);
}