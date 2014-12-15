#include "SequentialGame.h"

SequentialGame::SequentialGame()
        : Game()
{
}

SequentialGame::SequentialGame(int a, int b)
        : Game(a, b)
{
}

void SequentialGame::solve()
{
    _stack.push_back(Move(0, NoDirection));

    while (!_stack.empty())
    {
        step();
    }
}