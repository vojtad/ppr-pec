#ifndef SEQUENTIAL_GAME_H
#define SEQUENTIAL_GAME_H

#include "Game.h"

class SequentialGame : public Game
{
public:
    SequentialGame();
    SequentialGame(int a, int b);

    void solve();
};

#endif