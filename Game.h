#ifndef GAME_H
#define GAME_H

#include <deque>

#include "Move.h"

using namespace std;

class Game
{
public:
    Game(int a, int b);
    ~Game();

    int upperBound() const;
    int lowerBound() const;

    void printGamePlan() const;
    void printBest() const;

    void randomize();

    void walkthru();

private:
    void randomizeStep(int s);
    void calcLowerBound();

    bool canMoveUp(const Move& from) const;
    bool canMoveRight(const Move& from) const;
    bool canMoveDown(const Move& from) const;
    bool canMoveLeft(const Move& from) const;

    void walkthruOpen(Move& current);
    void walkthruClosed(Move& current);

    bool isInAcceptableEndState(const Move& current) const;

private:
    int _columns, _rows;
    int _q, _d;

    int** _gamePlan;
    int _currentRow, _currentColumn;

    int _bestMoveCount;
    MoveDirection* _bestMoves;

    deque<Move> _stack;
};

#endif