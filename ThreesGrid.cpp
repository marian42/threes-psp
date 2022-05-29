#include "ThreesGrid.h"
#include "Application.h"

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