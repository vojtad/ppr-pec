#ifndef PARALLEL_GAME_H
#define PARALLEL_GAME_H

#include "Game.h"
#include "GameCommunication.h"

class ParallelGame : public Game
{
public:
    ParallelGame();
    ParallelGame(int a, int b);

    void sendBoard();
    void recvBoard();

private:
    GameCommunication _comm;
};

#endif