#include "ThreesGrid.h"
#include "Application.h"
#include <cstring>
#include "utils.h"
#include <math.h>
#include "savedata.h"

constexpr int DEFAULT_DECK[] = {1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3};
constexpr int DEFAULT_DECK_SIZE = 12;
constexpr int SCORE[] = {0, 0, 0, 3, 9, 27, 81, 243, 729, 2187, 6561, 19683, 59049, 177147, 531441 };

struct CardMove {
    int from;
    int to;

    constexpr CardMove(int from, int to) : from(from), to(to) { }
    constexpr CardMove() : from(0), to(0) { }
};

constexpr const CardMove MOVES[] = {
    // RIGHT
    CardMove(INDEX(2, 0), INDEX(3, 0)),
    CardMove(INDEX(1, 0), INDEX(2, 0)),
    CardMove(INDEX(0, 0), INDEX(1, 0)),
    CardMove(INDEX(2, 1), INDEX(3, 1)),
    CardMove(INDEX(1, 1), INDEX(2, 1)),
    CardMove(INDEX(0, 1), INDEX(1, 1)),
    CardMove(INDEX(2, 2), INDEX(3, 2)),
    CardMove(INDEX(1, 2), INDEX(2, 2)),
    CardMove(INDEX(0, 2), INDEX(1, 2)),
    CardMove(INDEX(2, 3), INDEX(3, 3)),
    CardMove(INDEX(1, 3), INDEX(2, 3)),
    CardMove(INDEX(0, 3), INDEX(1, 3)),

    // DOWN
    CardMove(INDEX(0, 2), INDEX(0, 3)),
    CardMove(INDEX(0, 1), INDEX(0, 2)),
    CardMove(INDEX(0, 0), INDEX(0, 1)),
    CardMove(INDEX(1, 2), INDEX(1, 3)),
    CardMove(INDEX(1, 1), INDEX(1, 2)),
    CardMove(INDEX(1, 0), INDEX(1, 1)),
    CardMove(INDEX(2, 2), INDEX(2, 3)),
    CardMove(INDEX(2, 1), INDEX(2, 2)),
    CardMove(INDEX(2, 0), INDEX(2, 1)),
    CardMove(INDEX(3, 2), INDEX(3, 3)),
    CardMove(INDEX(3, 1), INDEX(3, 2)),
    CardMove(INDEX(3, 0), INDEX(3, 1)),

    // LEFT
    CardMove(INDEX(1, 0), INDEX(0, 0)),
    CardMove(INDEX(2, 0), INDEX(1, 0)),
    CardMove(INDEX(3, 0), INDEX(2, 0)),
    CardMove(INDEX(1, 1), INDEX(0, 1)),
    CardMove(INDEX(2, 1), INDEX(1, 1)),
    CardMove(INDEX(3, 1), INDEX(2, 1)),
    CardMove(INDEX(1, 2), INDEX(0, 2)),
    CardMove(INDEX(2, 2), INDEX(1, 2)),
    CardMove(INDEX(3, 2), INDEX(2, 2)),
    CardMove(INDEX(1, 3), INDEX(0, 3)),
    CardMove(INDEX(2, 3), INDEX(1, 3)),
    CardMove(INDEX(3, 3), INDEX(2, 3)),

    // UP
    CardMove(INDEX(0, 1), INDEX(0, 0)),
    CardMove(INDEX(0, 2), INDEX(0, 1)),
    CardMove(INDEX(0, 3), INDEX(0, 2)),
    CardMove(INDEX(1, 1), INDEX(1, 0)),
    CardMove(INDEX(1, 2), INDEX(1, 1)),
    CardMove(INDEX(1, 3), INDEX(1, 2)),
    CardMove(INDEX(2, 1), INDEX(2, 0)),
    CardMove(INDEX(2, 2), INDEX(2, 1)),
    CardMove(INDEX(2, 3), INDEX(2, 2)),
    CardMove(INDEX(3, 1), INDEX(3, 0)),
    CardMove(INDEX(3, 2), INDEX(3, 1)),
    CardMove(INDEX(3, 3), INDEX(3, 2)),
};

constexpr int INSERT_POSITIONS[] = {
    // RIGHT
    INDEX(0, 0),
    INDEX(0, 1),
    INDEX(0, 2),
    INDEX(0, 3),

    // DOWN
    INDEX(0, 0),
    INDEX(1, 0),
    INDEX(2, 0),
    INDEX(3, 0),

    // LEFT
    INDEX(3, 0),
    INDEX(3, 1),
    INDEX(3, 2),
    INDEX(3, 3),

    // UP
    INDEX(0, 3),
    INDEX(1, 3),
    INDEX(2, 3),
    INDEX(3, 3),
};


void ThreesGrid::Reset() {
    this->highestPiece = 3;
    this->score = 0;
    this->CreateDeck();
    for (int i = 0; i < 16; i++) {
        if (i < 9) {
            this->data[i] = this->TakeFromDeck();
        } else {
            this->data[i] = 0;            
        }
    }

    shuffleArray(this->data, 16, 32);

    for (int i = 0; i < 16; i++) {
        this->effects[i] = this->data[i] == 0 ? CardEffect::None : CardEffect::NewCard;
    }

    UpdatePreviews();
}

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

int getPieceColor(int piece) {
    if (piece == 1) {
        return 0xFFCC66;
    } else if (piece == 2) {
        return 0x8066FF;
    } else {
        return 0xFFFFFF;
    }
}

int formatPiece(int piece) {
    if (piece <= 2) {
        return piece;
    } else {
        return 3 * (1 << (piece - 3));
    }
}

void ThreesGrid::DebugDraw(int globalX, int globalY) {
    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 4; y++) {
            pspDebugScreenSetXY(globalX + x * 8, globalY + y * 6);
            int value = this->Get(x, y);

            if (value == 0) {
                continue;
            }
            
            if (value > 4 && value == this->highestPiece) {
                pspDebugScreenSetTextColor(0x66D9FF);
            } else {
                pspDebugScreenSetTextColor(getPieceColor(value));
            }

            pspDebugScreenPrintf("%d", formatPiece(value));
        }
    }

    
    pspDebugScreenSetXY(globalX + 6 * 8, globalY);
    pspDebugScreenSetTextColor(getPieceColor(PeekDeck()));
    pspDebugScreenPrintf("%d", formatPiece(PeekDeck()));

    pspDebugScreenSetXY(globalX + 6 * 8, globalY + 18);
    pspDebugScreenSetTextColor(0xFFFFFF);
    pspDebugScreenPrintf("%d", this->GetScore());
}

void ThreesGrid::CreateDeck() {
    this->deckCardsRemaining = DEFAULT_DECK_SIZE;
    for (int i = 0; i < this->deckCardsRemaining; i++) {
        this->deck[i] = DEFAULT_DECK[i];
    }

    if (this->highestPiece >= 7 && GetRandomBool() ) {
        if (this->highestPiece >= 8 && GetRandomBool()) {
            this->lowerstPossibleCard = this->highestPiece - 4;
            this->highestPossibleCard = this->highestPiece - 2;
        } else {
            this->lowerstPossibleCard = this->highestPiece - 3;
            this->highestPossibleCard = this->highestPiece - 2;
        }
        this->AddToDeck(GetRandomInt(this->lowerstPossibleCard, this->highestPossibleCard + 1));
    }

    shuffleArray(this->deck, this->deckCardsRemaining, 20);
}

int ThreesGrid::TakeFromDeck() {
    int result = this->deck[this->deckCardsRemaining - 1];
    this->deckCardsRemaining--;
    if (this->deckCardsRemaining == 0) {
        this->CreateDeck();
    }
    return result;
}

void ThreesGrid::ApplyMove(Direction direction) {
    for (int i = 0; i < 16; i++) {
        this->effects[i] = CardEffect::None;
    }

    bool horizontalMovement = direction == Direction::LEFT || direction == Direction::RIGHT;
    bool insertAllowed[4] = {false, false, false, false};

    for (int i = 0; i < 12; i++) {
        CardMove move = MOVES[direction * 12 + i];

        int& from = this->data[move.from];

        if (from == 0) {
            continue;
        }

        int& to = this->data[move.to];
        bool hasMoved = true;

        if (to == 0) {
            to = from;
            from = 0;
        } else if (from + to == 3) {
            to = 3;
            from = 0;
            this->effects[move.to] = CardEffect::Merged;
        } else if (from == to && from >= 3) {
            to++;
            from = 0;
            if (to > this->highestPiece) {
                this->highestPiece = to;
            }
            this->effects[move.to] = CardEffect::Merged;
        } else {
            hasMoved = false;
        }

        if (hasMoved) {                
            insertAllowed[horizontalMovement ? move.from % 4 : move.from / 4] = true;
        }
    }

    int insertIndices[4] = { 0, 1, 2, 3 };
    shuffleArray(insertIndices, 4, 8);

    for (int i = 0; i < 4; i++) {
        int position = INSERT_POSITIONS[4 * direction + insertIndices[i]];
        if (this->data[position] == 0 && insertAllowed[horizontalMovement ? position % 4 : position / 4]) {
            this->data[position] = this->TakeFromDeck();
            this->effects[position] = CardEffect::SlideIn;
            break;
        }
    }

    this->UpdatePreviews();
    this->UpdateStats();
}

void ThreesGrid::UpdatePreviews() {
    for (int i = 0; i < 4 * 16; i++) {
        this->movePreviews[i] = MoveType::Squeeze;
    }

    for (int i = 0; i < 4; i++) {
        this->possibleMoves[i] = false;
    }

    for (int direction = 0; direction < 4; direction++) {
        for (int i = 0; i < 12; i++) {
            CardMove move = MOVES[direction * 12 + i];
            int fromIndex = direction * 16 + move.from;
            int toIndex = direction * 16 + move.to;

            int from = this->data[move.from];

            if (from == 0) {
                continue;
            }

            int to = this->data[move.to];

            if (to == 0 || this->movePreviews[toIndex] == MoveType::Move || this->movePreviews[toIndex] == MoveType::MoveMerge) {
                this->movePreviews[fromIndex] = MoveType::Move;
                this->possibleMoves[direction] = true;
            } else if ((from + to == 3 || (from == to && from >= 3))
                && this->movePreviews[toIndex] != MoveType::Move
                && this->movePreviews[toIndex] != MoveType::MoveMerge) {
                this->movePreviews[fromIndex] = MoveType::MoveMerge;
                this->movePreviews[toIndex] = MoveType::ReceiveMerge;
                this->possibleMoves[direction] = true;
            }
        }
    }
}

int ThreesGrid::CalculateScore() {
    int result = 0;
    for (int i = 0; i < 16; i++) {
        result += SCORE[this->data[i]];
    }
    
    return result;
}