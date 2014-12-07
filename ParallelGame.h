#ifndef PARALLEL_GAME_H
#define PARALLEL_GAME_H

#include "Game.h"
#include "ParallelGameCommunication.h"

class ParallelGame : public Game
{
public:
    ParallelGame();
    ParallelGame(int a, int b);

    void sendBoard();
    void recvBoard();

private:
    ParallelGameCommunication _comm;
};

#endif