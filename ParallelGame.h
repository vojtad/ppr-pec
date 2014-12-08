#ifndef PARALLEL_GAME_H
#define PARALLEL_GAME_H

#include "Game.h"
#include "ParallelGameCommunication.h"

class ParallelGame : public Game
{
public:
    ParallelGame();
    ParallelGame(int a, int b);

    void load(const char* fileName);
    void prepare();
    void solve();

private:
    void sendBoard();
    void recvBoard();

    void sendWork(const deque<Move>& stack, int target);

    void handleCommunication(bool block);

    void handleWorkRequest();
    void handleWork();
    void handleNewBest();
    void handleToken();
    void handleFinish();

private:
    bool _finished;
};

#endif