#include "SequentialGame.h"

SequentialGame::SequentialGame(int a, int b)
        : Game(a, b)
{
}

void SequentialGame::solve()
{
    _stack.push_back(Move(0, NoDirection));

    while (!_stack.empty())
    {
        Move& current = _stack.back();

        switch (current.state)
        {
            case Open:
                handleOpenMove(current);
                break;
            case Closed:
                handleClosedMove(current);
                break;
            case Skip:
                handleSkipMove(current);
                break;
        }
    }
}