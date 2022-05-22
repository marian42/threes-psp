#pragma once

#include "utils.h"
#include <math.h>

constexpr int DEFAULT_DECK[] = {1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3};
constexpr int DEFAULT_DECK_SIZE = 12;
constexpr int SCORE[] = {0, 0, 0, 3, 9, 27, 81, 243, 729, 2187, 6561, 19683, 59049, 177147, 531441 };

int formatPiece(int piece) {
    if (piece <= 2) {
        return piece;
    } else {
        return 3 * (1 << (piece - 3));
    }
}

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

int getPieceColor(int piece) {
    if (piece == 1) {
        return 0xFFCC66;
    } else if (piece == 2) {
        return 0x8066FF;
    } else {
        return 0xFFFFFF;
    }
}

class ThreesGrid {
private:
    int data[16];

    int deck[20];
    int deckCardsRemaining;

    bool possibleMoves[4];

    MoveType movePreviews[4 * 16];

    int highestPiece;
public:

    void Reset() {
        this->highestPiece = 3;
        this->CreateDeck();
        for (int i = 0; i < 16; i++) {
            if (i < 9) {
                this->data[i] = this->TakeFromDeck();
            } else {
                this->data[i] = 0;
            }
        }

        shuffleArray(this->data, 16, 32);

        UpdatePreviews();
    }

    void DebugDraw(int globalX, int globalY) {
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

    int Get(int x, int y) {
        return this->data[INDEX(x, y)];
    }

    void CreateDeck() {
        this->deckCardsRemaining = DEFAULT_DECK_SIZE;
        for (int i = 0; i < this->deckCardsRemaining; i++) {
            this->deck[i] = DEFAULT_DECK[i];
        }

        if (this->highestPiece >= 7 && GetRandomBool() ) {
            if (this->highestPiece >= 8 && GetRandomBool()) {
                this->AddToDeck(this->highestPiece - GetRandomInt(2, 5));
            } else {
                this->AddToDeck(this->highestPiece - GetRandomInt(2, 4));
            }
        }

        shuffleArray(this->deck, this->deckCardsRemaining, 20);
    }

    int PeekDeck() {
        return this->deck[this->deckCardsRemaining - 1];
    }

    int TakeFromDeck() {
        int result = this->deck[this->deckCardsRemaining - 1];
        this->deckCardsRemaining--;
        if (this->deckCardsRemaining == 0) {
            this->CreateDeck();
        }
        return result;
    }

    void ApplyMove(Direction direction) {
        for (int i = 0; i < 12; i++) {
            CardMove move = MOVES[direction * 12 + i];

            int& from = this->data[move.from];

            if (from == 0) {
                continue;
            }

            int& to = this->data[move.to];

            if (to == 0) {
                to = from;
                from = 0;
            } else if (from + to == 3) {
                to = 3;
                from = 0;
            } else if (from == to && from >= 3) {
                to++;
                from = 0;
                if (to > this->highestPiece) {
                    this->highestPiece = to;
                }
            }
        }

        int insertIndices[4] = { 0, 1, 2, 3 };
        shuffleArray(insertIndices, 4, 8);

        for (int i = 0; i < 4; i++) {
            int position = INSERT_POSITIONS[4 * direction + insertIndices[i]];
            if (this->data[position] == 0) {
                this->data[position] = this->TakeFromDeck();
                break;
            }
        }

        UpdatePreviews();
    }

    int GetScore() {
        int result = 0;
        for (int i = 0; i < 16; i++) {
            result += SCORE[this->data[i]];
        }
        
        return result;
    }

    MoveType GetPreview(Direction direction, int x, int y) {
        return this->movePreviews[direction * 16 + INDEX(x, y)];
    }

    bool IsMovePossible(Direction direction) {
        return this->possibleMoves[direction];
    }

private:
    void AddToDeck(int piece) {
        this->deck[this->deckCardsRemaining] = piece;
        this->deckCardsRemaining++;
    }

    void UpdatePreviews() {
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
};