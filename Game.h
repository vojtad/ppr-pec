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

    void load(const char* filename);
    void randomize();

protected:
    void step();

    void handleOpenMove(Move& current);
    void handleClosedMove(Move& current);
    void handleSkipMove(Move& current);

    bool isInAcceptableEndState(const Move& current) const;

private:
    virtual void onBestChanged();

protected:
    GameBoard _board;

    int _bestMoveCount;
    int* _bestMoves;

    int _openMoveCount;
    deque<Move> _stack;
};

#endif