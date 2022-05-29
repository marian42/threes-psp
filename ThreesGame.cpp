#include "ThreesGame.h"
#include "Application.h"

#include <pspkernel.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspdisplay.h>
#include <pspdebug.h>
#include "ThreesGrid.h"
#include "text.h"
#include "utils.h"
#include "input.h"
#include <cstdio>

#include <math.h>

void ThreesGame::UpdateGameplay() {
    bool movementButtonPressed = false;
    Direction directionPressed;

    if (PSPInput::GetButtonPressed(PSP_CTRL_UP)) {
        directionPressed = Direction::UP;
        movementButtonPressed = true;
    } else if (PSPInput::GetButtonPressed(PSP_CTRL_RIGHT)) {
        directionPressed = Direction::RIGHT;
        movementButtonPressed = true;
    } else if (PSPInput::GetButtonPressed(PSP_CTRL_DOWN)) {
        directionPressed = Direction::DOWN;
        movementButtonPressed = true;
    } else if (PSPInput::GetButtonPressed(PSP_CTRL_LEFT)) {            
        directionPressed = Direction::LEFT;
        movementButtonPressed = true;
    }

    if (PSPInput::GetButtonDown(PSP_CTRL_CROSS | PSP_CTRL_START) && grid.IsGameOver()) {
        Application::instance.Save(false);
        Application::instance.SwitchScreen(Screen::GameComplete);
        return;
    }
    if (PSPInput::GetButtonDown(PSP_CTRL_START)) {
        Application::instance.SwitchScreen(Screen::PauseMenu);
    }

    if (this->waitForButtonRelease) {
        movementButtonPressed = false;
        if (!PSPInput::GetButtonPressed(PSP_CTRL_DOWN | PSP_CTRL_LEFT | PSP_CTRL_UP | PSP_CTRL_RIGHT)) {
            this->waitForButtonRelease = false;
        }
    }

    if (!movementButtonPressed) {
        this->previewAmount -= 0.1;
    } else if (this->previewDirection == directionPressed) {
        this->previewAmount += 0.08;
        if (this->previewAmount > 1.0 && this->grid.IsMovePossible(this->previewDirection)) {
            this->grid.ApplyMove(this->previewDirection);
            this->lastMoveDirection = this->previewDirection;
            this->previewAmount = 0;
            this->waitForButtonRelease = true;
            this->timeSinceLastMove = 0.0f;
            this->effectAmount = 0.0f;
        }
    } else if (this->previewAmount <= 0) {
        this->previewDirection = directionPressed;
    } else {
        this->previewAmount -= 0.2;
    }

    if (this->previewAmount < 0) {
        this->previewAmount = 0;
    } else if (this->previewAmount > 1.0) {
        this->previewAmount = 1.0;
    }

    if (this->effectAmount < 1.0f) {
        this->effectAmount += this->previewAmount > 0.1f ? 0.2f : 0.07f;
        if (this->effectAmount > 1.0f) {
            this->effectAmount = 1.0f;
        }
    }

    this->timeSinceLastMove += 1.0f / 60.0f;

    this->currentSecondProgress += 1.0f / 60.0f;
    if (this->currentSecondProgress > 1.0f) {
        Application::instance.GetStatistics()->secondsPlayed++;
        this->currentSecondProgress -= 1.0f;
    }
}

void ThreesGame::Draw() {
    useCardsTexture();

    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 4; y++) {
            drawSlot(GRID_X + x * 64, GRID_Y + y * 64);
        }
    }

    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 4; y++) {
            if (this->grid.Get(x, y) != 0 && this->grid.GetPreview(this->previewDirection, x, y) == MoveType::ReceiveMerge) {
                drawCard(x, y);
            }
        }
    }

    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 4; y++) {
            if (this->grid.Get(x, y) != 0 && this->grid.GetPreview(this->previewDirection, x, y) != MoveType::ReceiveMerge) {
                drawCard(x, y);
            }
        }
    }

    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 4; y++) {
            if (this->grid.Get(x, y) < 3 && this->grid.GetPreview(this->previewDirection, x, y) == MoveType::ReceiveMerge) {
                drawRedBlueMerge(x, y);
            }
        }
    }

    if (!this->grid.IsGameOver()) {
        DrawNextCard();
    }

    int actualScore = this->grid.GetScore();

    if (actualScore > this->score) {
        this->score += (actualScore - this->score) / 16 + 1;
    }

    int highscore = Application::instance.GetStatistics()->highscore;
    if (highscore == this->grid.GetScore()) {
        highscore = this->score;
    }

    if (Application::instance.GetOptions()->showScore) {
        char scoreString[10];
        sprintf(scoreString, "%d", this->score);

        useSpritesheet();
        drawString(scoreString, 470, 8, 0xFF7E8271, TextAlignment::Right);

        sprintf(scoreString, "%d", highscore);
        drawString(scoreString, 470, 32, 0x777E8271, TextAlignment::Right);
    }

    if (this->grid.IsGameOver()) {
        int offset = (int)((1.0f - ease(clamp((this->timeSinceLastMove - 1.5f) / 0.7f))) * 40.0f);
        drawGlyph(Glyph::Cross, 369, 240 + offset);
        drawString("Continue", 396, 240 + offset, HEXCOLOR(0x7E7E7E), TextAlignment::Left);
    }
}

void ThreesGame::drawCard(int cardX, int cardY) {
    float moveX = this->previewDirection == Direction::RIGHT ? 1 : (this->previewDirection == Direction::LEFT ? -1 : 0);
    float moveY = this->previewDirection == Direction::DOWN ? 1 : (this->previewDirection == Direction::UP ? -1 : 0);

    MoveType movementType = this->grid.GetPreview(this->previewDirection, cardX, cardY);
    CardEffect effect = this->grid.GetEffect(cardX, cardY);

    float x = cardX;
    float y = cardY;
    float sizeX = 1.0f;
    float sizeY = 1.0f;

    if (movementType == MoveType::Move || movementType == MoveType::MoveMerge) {
        x += moveX * this->previewAmount;
        y += moveY * this->previewAmount;
    }

    if (effect == CardEffect::SlideIn) {
        float f = (this->effectAmount + 0.4f) / 1.4f;
        f = (1.0 - f * f);
        x -= f * 2.0f * moveX;
        y -= f * 2.0f * moveY;
    } else if (effect == CardEffect::Merged) {
        float size = abs(cos(this->effectAmount * 3.14159));
        sizeX *= size;
        x += (1.0f - size) * 0.5f;
    } else if (effect == CardEffect::NewCard) {
        float progress = clamp((this->timeSinceLastMove - ((cardX + cardY) * 0.05f)) / 0.3f);
        float size = sin(progress * 3.141459) * 0.45 + progress;
        sizeX *= size;
        x += (1.0f - size) * 0.5f;
        sizeY *= size;
        y += (1.0f - size) * 0.5f;
    }
    
    VERT* v = (VERT*)sceGuGetMemory(sizeof(VERT) * 2);

    int index = this->grid.Get(cardX, cardY);
    if (effect == CardEffect::Merged && this->effectAmount < 0.5f) {
        index--;
        if (index == 2) {
            index = 15;
        }
    }

    int tx = (index % 4) * 64;
    int ty = (index / 4) * 64;

    VERT* v0 = &v[0];
    VERT* v1 = &v[1];
    
    v0->s = (float)(tx);
    v0->t = (float)(ty);
    v0->c = 0xFFFFFFFF;
    v0->x = GRID_X + x * 64.0f;
    v0->y = GRID_Y + y * 64.0f;
    v0->z = 0.0f;

    v1->s = (float)(tx + 64);
    v1->t = (float)(ty + 64);
    v1->c = 0xFFFFFFFF;
    v1->x = GRID_X + (x + sizeX) * 64.0f;
    v1->y = GRID_Y + (y + sizeY) * 64.0f;
    v1->z = 0.0f;

    sceGumDrawArray(GU_SPRITES, 
        GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D,
        2, 0, v
    );
}


void ThreesGame::drawSlot(float x, float y) {
    VERT* v = (VERT*)sceGuGetMemory(sizeof(VERT) * 2);

    VERT* v0 = &v[0];
    VERT* v1 = &v[1];
    
    v0->s = 0.0f;
    v0->t = 0.0f;
    v0->c = 0xFFFFFFFF;
    v0->x = x;
    v0->y = y;
    v0->z = 0.0f;

    v1->s = 64.0f;
    v1->t = 64.0f;
    v1->c = 0xFFFFFFFF;
    v1->x = x + 64.0f;
    v1->y = y + 64.0f;
    v1->z = 0.0f;

    sceGumDrawArray(GU_SPRITES, 
        GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D,
        2, 0, v
    );
}

void ThreesGame::drawRedBlueMerge(int cardX, int cardY) {
    VERT* v = (VERT*)sceGuGetMemory(sizeof(VERT) * 4);

    VERT* v0 = &v[0];
    VERT* v1 = &v[1];
    VERT* v2 = &v[2];
    VERT* v3 = &v[3];

    float moveX = this->previewDirection == Direction::RIGHT ? 1 : (this->previewDirection == Direction::LEFT ? -1 : 0);
    float moveY = this->previewDirection == Direction::DOWN ? 1 : (this->previewDirection == Direction::UP ? -1 : 0);

    v0->c = 0xFFFFFFFF;
    v1->c = 0xFFFFFFFF;
    v2->c = 0xFFFFFFFF;
    v3->c = 0xFFFFFFFF;
    v0->z = 0.0f;
    v1->z = 0.0f;
    v2->z = 0.0f;
    v3->z = 0.0f;

    switch (this->previewDirection) {
        case Direction::RIGHT:
            v0->s = 192.0f;
            v0->t = 192.0f;
            v0->x = GRID_X + cardX * 64.0f;
            v0->y = GRID_Y + cardY * 64.0f;

            v1->s = 192.0f + this->previewAmount * 32.0f;
            v1->t = 256.0f;
            v1->x = GRID_X + cardX * 64.0f + 32.0f * this->previewAmount;
            v1->y = GRID_Y + cardY * 64.0f + 64.0f;

            v2->s = 256.0f - this->previewAmount * 32.0f;
            v2->t = 192.0f;
            v2->x = GRID_X + cardX * 64.0f + 32.0f * this->previewAmount;
            v2->y = GRID_Y + cardY * 64.0f;

            v3->s = 256.0f;
            v3->t = 256.0f;
            v3->x = GRID_X + cardX * 64.0f + 64.0f * this->previewAmount;
            v3->y = GRID_Y + cardY * 64.0f + 64.0f;
            break;
        
        case Direction::LEFT:
            v0->s = 256.0f - this->previewAmount * 32.0f;
            v0->t = 192.0f;
            v0->x = GRID_X + cardX * 64.0f + 64.0f - 32.0f * this->previewAmount;
            v0->y = GRID_Y + cardY * 64.0f;

            v1->s = 256.0f;
            v1->t = 256.0f;
            v1->x = GRID_X + cardX * 64.0f + 64.0f;
            v1->y = GRID_Y + cardY * 64.0f + 64.0f;

            v2->s = 192.0f;
            v2->t = 192.0f;
            v2->x = GRID_X + cardX * 64.0f + 64.0f - 64.0f * this->previewAmount;
            v2->y = GRID_Y + cardY * 64.0f;

            v3->s = 192.0f + this->previewAmount * 32.0f;
            v3->t = 256.0f;
            v3->x = GRID_X + cardX * 64.0f + 64.0f - 32.0f * this->previewAmount;
            v3->y = GRID_Y + cardY * 64.0f + 64.0f;
            break;

        case Direction::DOWN:
            v0->s = 192.0f;
            v0->t = 192.0f;
            v0->x = GRID_X + cardX * 64.0f;
            v0->y = GRID_Y + cardY * 64.0f;

            v1->s = 256.0f;
            v1->t = 192.0f + this->previewAmount * 32.0f;
            v1->x = GRID_X + cardX * 64.0f + 64.0f;
            v1->y = GRID_Y + cardY * 64.0f + 32.0f * this->previewAmount;

            v2->s = 192.0f;
            v2->t = 256.0f - this->previewAmount * 32.0f;
            v2->x = GRID_X + cardX * 64.0f;
            v2->y = GRID_Y + cardY * 64.0f + 32.0f * this->previewAmount;

            v3->s = 256.0f;
            v3->t = 256.0f;
            v3->x = GRID_X + cardX * 64.0f + 64.0f;
            v3->y = GRID_Y + cardY * 64.0f + 64.0f * this->previewAmount;
            break;
        
        case Direction::UP:
            v0->s = 192.0f;
            v0->t = 256.0f - this->previewAmount * 32.0f;
            v0->x = GRID_X + cardX * 64.0f;
            v0->y = GRID_Y + cardY * 64.0f + 64.0f - 32.0f * this->previewAmount;

            v1->s = 256.0f;
            v1->t = 256.0f;
            v1->x = GRID_X + cardX * 64.0f + 64.0f;
            v1->y = GRID_Y + cardY * 64.0f + 64.0f;

            v2->s = 192.0f;
            v2->t = 192.0f;
            v2->x = GRID_X + cardX * 64.0f;
            v2->y = GRID_Y + cardY * 64.0f + 64.0f - 64.0f * this->previewAmount;

            v3->s = 256.0f;
            v3->t = 192.0f + this->previewAmount * 32.0f;
            v3->x = GRID_X + cardX * 64.0f + 64.0f;
            v3->y = GRID_Y + cardY * 64.0f + 64.0f - 32.0f * this->previewAmount;
            break;
    }

    sceGumDrawArray(GU_SPRITES, 
        GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D,
        4, 0, v
    );
}

void ThreesGame::DrawNextCard() {
    float opacity = this->effectAmount;

    if (this->previewAmount > 0.0f && this->grid.IsMovePossible(this->previewDirection)) {
        opacity = min(opacity, 1.0f - this->previewAmount);
    }

    float left = 8 - 64 * (1.0f - this->effectAmount);

    if (this->grid.PeekDeck() < 4) {
        this->DrawPreviewCard(this->grid.PeekDeck(), left, 120, opacity);
    } else {
        int numberOfOptions = this->grid.GetHighestPossibleCard() - this->grid.GetLowestPossibleCard() + 1;
        for (int i = 0; i < numberOfOptions; i++) {                
            this->DrawPreviewCard(this->grid.GetLowestPossibleCard() + i, left, 136 - numberOfOptions * 16 + i * 32, opacity);
        }
    }
}

void ThreesGame::DrawPreviewCard(int index, float x, float y, float opacity) {            
    VERT* v = (VERT*)sceGuGetMemory(sizeof(VERT) * 2);

    char opacityByte = (char)(opacity * 255);
    int color = 0x00FFFFFF | (opacityByte << 24);

    int tx = (index % 4) * 64;
    int ty = (index / 4) * 64;

    VERT* v0 = &v[0];
    VERT* v1 = &v[1];
    
    v0->s = (float)(tx);
    v0->t = (float)(ty);
    v0->c = color;
    v0->x = x;
    v0->y = y;
    v0->z = 0.0f;

    v1->s = (float)(tx + 64);
    v1->t = (float)(ty + 64);
    v1->c = color;
    v1->x = x + 32.0f;
    v1->y = y + 32.0f;
    v1->z = 0.0f;

    sceGumDrawArray(GU_SPRITES, 
        GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D,
        2, 0, v
    );
}

void ThreesGame::DebugDraw() {
    pspDebugScreenInit();   
    grid.DebugDraw(8, 7);
}