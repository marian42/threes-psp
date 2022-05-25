#pragma once

#include <cstring>
#include "img/spritesheet.c"
#include <pspkernel.h>
#include <pspgu.h>
#include <pspgum.h>

static int charPosition[] = {0, 6, 12, 19, 33, 45, 60, 74, 78, 84, 92, 101, 113, 118, 128, 131, 141, 152, 163, 174, 185, 196, 207, 218, 229, 240, 0, 5, 9, 19, 29, 37, 47, 64, 79, 93, 106, 120, 132, 144, 158, 173, 177, 189, 203, 215, 232, 0, 14, 27, 41, 54, 66, 78, 91, 104, 123, 135, 146, 159, 163, 171, 179, 188, 198, 205, 217, 228, 238, 0, 11, 18, 30, 42, 45, 53, 64, 70, 88, 100, 112, 123, 135, 144, 154, 162, 173, 183, 200, 211, 222, 232, 241, 245};
static int charWidth[] = {6, 6, 7, 14, 12, 15, 14, 4, 6, 8, 9, 12, 5, 10, 3, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 5, 4, 10, 10, 8, 10, 17, 15, 14, 13, 14, 12, 12, 14, 15, 4, 12, 14, 12, 17, 14, 14, 13, 14, 13, 12, 12, 13, 13, 19, 12, 11, 13, 4, 8, 8, 9, 10, 7, 11, 11, 10, 10, 11, 7, 12, 12, 3, 8, 11, 6, 18, 12, 12, 11, 12, 9, 10, 8, 11, 10, 17, 11, 11, 10, 9, 4, 9};


enum TextAlignment {
	Left,
	Right,
	Center
};


static void useSpritesheet() {
	sceGuTexImage(0, 256, 256, 256, spritesheet);
}

static void drawString(const char* text, int x, int y, unsigned int color, TextAlignment alignment = TextAlignment::Left) {
	int len = (int)strlen(text);

	constexpr int CHAR_HEIGHT = 26;
	
	typedef struct {
		float s, t;
		unsigned int c;
		float x, y, z;
	} VERT;

	VERT* v = (VERT*)sceGuGetMemory(sizeof(VERT) * 2 * len);

	if (alignment != TextAlignment::Left) {
		int totalWidth = 0;
		for (int i = 0; i < len; i++) {
			unsigned char c = (unsigned char)text[i];
			if(c < 32 || c >= 125) {
				c = 0;
			}
			totalWidth += charWidth[c - 32];
		}

		if (alignment == TextAlignment::Right) {
			x -= totalWidth;
		} else {
			x -= totalWidth / 2;
		}
	}

	for (int i = 0; i < len; i++) {
		unsigned char c = (unsigned char)text[i];
		if (c < 32 || c >= 125) {
			c = 0;
		}

		int tx = charPosition[c - 32];
		int ty = c < 58 ? 0 : (c < 79 ? 1 : (c < 101 ? 2 : 3)) * CHAR_HEIGHT;
		int width = charWidth[c - 32];

		VERT* v0 = &v[i*2+0];
		VERT* v1 = &v[i*2+1];
		
		v0->s = (float)(tx);
		v0->t = (float)(ty);
		v0->c = color;
		v0->x = (float)(x);
		v0->y = (float)(y);
		v0->z = 0.0f;

		v1->s = (float)(tx + width);
		v1->t = (float)(ty + CHAR_HEIGHT);
		v1->c = color;
		v1->x = (float)(x + width);
		v1->y = (float)(y + CHAR_HEIGHT);
		v1->z = 0.0f;

		x += width;
	}

	sceGumDrawArray(GU_SPRITES, 
		GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D,
		len * 2, 0, v
	);
}