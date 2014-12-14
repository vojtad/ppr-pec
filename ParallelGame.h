#ifndef PARALLEL_GAME_H
#define PARALLEL_GAME_H

#include "Game.h"

#include <mpi.h>
#include <vector>

class ParallelGame : public Game
{
public:
    ParallelGame();
    ParallelGame(int a, int b);

    void prepare();
    void solve();

private:
    void initializeStack(int* buffer, int count);

    void sendBoard();
    void recvBoard();

    void sendWorkRequest();

    void sendWork(int target);
    void sendWork(int* stack, int stackSize, int target);
    void sendNoWork(int target);

    void sendNewBest();

    void sendToken();
    void sendToken(char tokenColor);

    void sendFinishToSlaves();

    void handleCommunication(bool block);

    void handleWorkRequest(MPI_Status& status);
    void handleWork(MPI_Status& status);
    void handleNoWork(MPI_Status& status);
    void handleNewBest(MPI_Status& status);
    void handleToken(MPI_Status& status);
    void handleFinish(MPI_Status& status);

    void finish();

private:
    virtual void onBestChanged();

private:
    vector<int> _stackBuffer;
    int _nextWorkRequestTarget;
    bool _hasToken;
    char _tokenColor;
    bool _finished;
};

#endif