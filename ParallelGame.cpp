#include "ParallelGame.h"

#include "MPI.h"

ParallelGame::ParallelGame()
        : Game(), _finished(false)
{
}

ParallelGame::ParallelGame(int a, int b)
        : Game(a, b), _finished(false)
{
}

void ParallelGame::prepare()
{
    if (MPI::instance()->isMaster())
    {
        sendBoard();

        while (openMoveCount() < MPI::instance()->size())
            step();

        for (int i = 1; i < MPI::instance()->size(); ++i)
        {
            deque<Move>::const_iterator it = _stack.begin();
            deque<Move> newStack;

            while (it->state != Open)
            {
                if (it->state == Closed)
                    newStack.push_back(*it);

                ++it;
            }

            newStack.push_back(*it);
            it->state = Skip;

            sendWork(newStack, i);
        }
    }
    else
    {
        recvBoard();
    }
}

void ParallelGame::solve()
{
    int counter = 0;

    while (!_finished)
    {
        MPI::instance()->send(p, MSG_WORK_REQUEST);
        handleCommunication(true);

        if (_finished)
            return;

        while (!_stack.empty())
        {
            step();

            if ((++counter % 100) == 0)
            {
                handleCommunication(false);

                if (_finished)
                    return;
            }
        }
    }
}

void ParallelGame::sendBoard()
{
    int buffer* = _board.serialize();

    for (int i = 1; i < MPI::instance()->size(); ++i)
    {
        MPI::instance()->send(buffer, _board.count(), MPI_INT, i, MSG_BOARD);
    }
}

void ParallelGame::recvBoard()
{
    int count = 0;
    int* buffer;
    MPI_Status status;

    MPI::instance()->waitForMessage(status, MSG_GAME_BOARD);
    MPI::instance()->getCount(status, MPI_INT, &count);

    buffer = new int[count];

    MPI::instance()->recv(status, buffer, count, MPI_INT);

    _board.deserialize(buffer);
}

void ParallelGame::sendWork(const deque<Move>& stack, int target)
{

}

void ParallelGame::handleCommunication(bool block)
{
    MPI_Status status;

    if (block)
    {
        MPI::instance()->waitForMessage(status);
    }
    else
    {
        if (!MPI::instance()->anyMessagePending(status))
            return;
    }

    switch (status.MPI_TAG)
    {
        case MSG_GAME_BOARD:
            // should never happen
            break;
        case MSG_WORK_REQUEST:
            handleWorkRequest();
            break;
        case MSG_WORK_SENT:
            handleWork();
            break;
        case MSG_WORK_NOWORK:
            // no need to do anything
            break;
        case MSG_NEW_BEST:
            handleNewBest();
            break;
        case MSG_TOKEN:
            handleToken();
            break;
        case MSG_FINISH:
            handleFinish();
            break;
    }
}

void ParallelGame::handleWorkRequest()
{

}

void ParallelGame::handleWork()
{

}

void ParallelGame::handleNewBest()
{

}

void ParallelGame::handleToken()
{

}

void ParallelGame::handleFinish()
{
    _finished = true;
}
