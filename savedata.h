#pragma once

struct Statistics {
    int highscore = 0;
    int gamesPlayed = 0;
    int moves = 0;
    int secondsPlayed = 0;
    int highestPiece = 0;
};

struct GameOptions {
    bool holdToMove = true;
    bool showScore = false;
};

struct Savedata {
    Statistics stats;

    bool containsInProgressGame = false;
    int data[16];
    int deck[14];
    int deckCardsRemaining;
    int highestPossibleCard;
    int lowerstPossibleCard;

    GameOptions options;
};