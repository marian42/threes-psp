#pragma once

enum TextAlignment {
	Left,
	Right,
	Center
};

enum Glyph {
	Cross,
	Circle,
	Square,
	Triangle,
	ToggleOff,
	ToggleOn
};


void useSpritesheet();

void drawString(const char* text, int x, int y, unsigned int color, TextAlignment alignment = TextAlignment::Left);

void drawGlyph(Glyph glyph, int x, int y);