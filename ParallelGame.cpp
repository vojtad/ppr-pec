#include "ParallelGame.h"

ParallelGame::ParallelGame()
        : Game()
{
}

ParallelGame::ParallelGame(int a, int b)
        : Game(a, b)
{
}

void ParallelGame::sendBoard()
{
    _comm.sendBoard(_board);
}

void ParallelGame::recvBoard()
{
    _comm.recvBoard(_board);
}