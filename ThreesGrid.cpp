#include "ThreesGrid.h"
#include "Application.h"
#include <cstring>

void ThreesGrid::UpdateStats() {
    int oldScore = this->score;
    this->score = this->CalculateScore();
    
    Statistics* stats = Application::instance.GetStatistics();
    stats->moves++;
    if (this->score > stats->highscore) {
        stats->highscore = this->score;
    }
    if (this->highestPiece > stats->highestPiece) {
        stats->highestPiece = this->highestPiece;
    }
    if (oldScore < 60 && this->score >= 60) {
        stats->gamesPlayed++;
    }
}

void ThreesGrid::StoreToSavedata(Savedata* savedata) {
    savedata->containsInProgressGame = true;
    memcpy(savedata->data, this->data, 16 * 4);
    memcpy(savedata->deck, this->deck, 14 * 4);
    savedata->deckCardsRemaining = this->deckCardsRemaining;
    savedata->highestPossibleCard = this->highestPossibleCard;
    savedata->lowerstPossibleCard = this->lowerstPossibleCard;
}

void ThreesGrid::LoadFromSaveData(const Savedata* savedata) {
    memcpy(this->data, savedata->data, 16 * 4);
    memcpy(this->deck, savedata->deck, 14 * 4);
    this->deckCardsRemaining = savedata->deckCardsRemaining;
    this->highestPossibleCard = savedata->highestPossibleCard;
    this->lowerstPossibleCard = savedata->lowerstPossibleCard;

    this->highestPiece = 0;
    for (int i = 0; i < 16; i++) {
        if (this->data[i] > this->highestPiece) {
            this->highestPiece = this->data[i];
        }
    }

    this->score = this->GetScore();
    
    for (int i = 0; i < 16; i++) {
        this->effects[i] = this->data[i] == 0 ? CardEffect::None : CardEffect::NewCard;
    }
    
    UpdatePreviews();
}