#ifndef GAME_H
#define GAME_H

#include <deque>

#include "Move.h"
#include "GameBoard.h"

using namespace std;

class Game
{
public:
    Game();
    Game(int a, int b);

    ~Game();

    void printGamePlan() const;
    void printBest() const;

    void randomize();

protected:
    void step();

    void handleOpenMove(Move& current);
    void handleClosedMove(Move& current);
    void handleSkipMove(Move& current);

    bool isInAcceptableEndState(const Move& current) const;

protected:
    GameBoard _board;

    int _bestMoveCount;
    MoveDirection* _bestMoves;

    deque<Move> _stack;
};

#endif