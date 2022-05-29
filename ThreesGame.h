#pragma once

#include "ThreesGrid.h"

static unsigned int __attribute__((aligned(16))) list[262144];

constexpr int GRID_X = 112;
constexpr int GRID_Y = 8;

typedef struct {
    float s, t;
    unsigned int c;
    float x, y, z;
} VERT;

class ThreesGame {
public:
    void NewGame() {
        grid.Reset();
        this->previewAmount = 0.0f;
        this->effectAmount = 0.0f;
        this->timeSinceLastMove = 0.0f;
        this->moveCommitted = false;
        this->score = this->grid.GetScore();
    }

    void Update() {
        UpdateGameplay();
        Draw();
    }

    void UpdateGameplay();

    void Draw();

    void DebugDraw();

    int GetScore() {
        return this->grid.GetScore();
    }
    
    void LoadGame(const Savedata* savedata) {
        this->grid.LoadFromSaveData(savedata);
        this->previewAmount = 0.0f;
        this->effectAmount = 0.0f;
        this->moveCommitted = false;
        this->score = this->grid.GetScore();
    }

    void SaveGame(Savedata* savedata) {
        this->grid.StoreToSavedata(savedata);
    }

private:
    ThreesGrid grid;

    Direction previewDirection;
    Direction lastMoveDirection;

    float previewAmount = 0;
    float effectAmount = 0;
    float timeSinceLastMove = 0;

    bool waitForButtonRelease = false;
    bool moveCommitted = false;

    int score;

    float currentSecondProgress = 0.0f;    

    void drawCard(int cardX, int cardY);
    
    void drawSlot(float x, float y);

    void drawRedBlueMerge(int cardX, int cardY);

    void DrawNextCard();

    void DrawPreviewCard(int index, float x, float y, float opacity);

    void ApplyMove();
};