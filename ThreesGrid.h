#pragma once

struct Savedata;

enum Direction {
    RIGHT,
    DOWN,
    LEFT,
    UP
};

enum MoveType {
    Squeeze,
    Stay,
    Move,
    MoveMerge,
    ReceiveMerge
};

#define INDEX(x, y) x * 4 + y

enum CardEffect {
    None,
    Merged,
    SlideIn,
    NewCard
};

class ThreesGrid {
private:
    int data[16];

    int deck[14];
    int deckCardsRemaining;

    int highestPossibleCard;
    int lowerstPossibleCard;

    bool possibleMoves[4];

    MoveType movePreviews[4 * 16];

    int score = 0;
    int highestPiece;

    CardEffect effects[16];
public:

    void Reset();

    void LoadFromSaveData(const Savedata* savedata);

    void DebugDraw(int globalX, int globalY);

    int Get(int x, int y) {
        return this->data[INDEX(x, y)];
    }

    void CreateDeck();

    int PeekDeck() {
        return this->deck[this->deckCardsRemaining - 1];
    }

    int TakeFromDeck();

    void ApplyMove(Direction direction);

    MoveType GetPreview(Direction direction, int x, int y) {
        return this->movePreviews[direction * 16 + INDEX(x, y)];
    }

    bool IsMovePossible(Direction direction) {
        return this->possibleMoves[direction];
    }

    CardEffect GetEffect(int x, int y) {
        return this->effects[INDEX(x, y)];
    }
    
    bool IsGameOver() {
        return !this->possibleMoves[0] && !this->possibleMoves[1] && !this->possibleMoves[2] && !this->possibleMoves[3];    
    }

    int GetScore() {
        return this->score;
    }

    int GetHighestPossibleCard() {
        return this->highestPossibleCard;
    }

    int GetLowestPossibleCard() {
        return this->lowerstPossibleCard;
    }

    void StoreToSavedata(Savedata* savedata);

private:
    void AddToDeck(int piece) {
        this->deck[this->deckCardsRemaining] = piece;
        this->deckCardsRemaining++;
    }

    void UpdatePreviews();

    int CalculateScore();

    void UpdateStats();
};