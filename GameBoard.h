#ifndef GAME_BOARD_H
#define GAME_BOARD_H

#include "Move.h"

class GameBoard
{
public:
    GameBoard();
    GameBoard(int a, int b);

    ~GameBoard();

    int upperBound() const;
    int lowerBound() const;

    void print() const;

    void load(const char* filename);
    void randomize();

    void calcLowerBound();

    bool canMoveUp(const Move& from) const;
    bool canMoveRight(const Move& from) const;
    bool canMoveDown(const Move& from) const;
    bool canMoveLeft(const Move& from) const;

    void makeMove(MoveDirection direction);

    bool isInAcceptableEndState() const;

    int* serialize(int* bufferSize) const;
    void deserialize(int* buffer);

private:
    void randomizeStep(int s);

private:
    int _columns, _rows;
    int _q, _d;

    int** _gamePlan;
    int _currentRow, _currentColumn;
};

#endif