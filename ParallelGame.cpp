#include "ParallelGame.h"

#include <cctype>

#include "MyMPI.h"

#define D(x) //x

enum Messages
{
    MsgGameBoard = 1000,
    MsgWorkRequest,
    MsgWork,
    MsgWorkNoWork,
    MsgNewBest,
    MsgToken,
    MsgFinish
};

ParallelGame::ParallelGame()
        : Game(), _tokenColor('W'), _finished(false)
{
    _hasToken = MyMPI::instance()->isMaster();
    _nextWorkRequestTarget = (MyMPI::instance()->rank() + 1) % MyMPI::instance()->size();
}

ParallelGame::ParallelGame(int a, int b)
        : Game(a, b), _tokenColor('W'), _finished(false)
{
    _hasToken = MyMPI::instance()->isMaster();
    _nextWorkRequestTarget = (MyMPI::instance()->rank() + 1) % MyMPI::instance()->size();
}

void ParallelGame::prepare()
{
    D(cerr << RANK << ": ParallelGame::prepare" << endl);

    if (MyMPI::instance()->isMaster())
    {
        sendBoard();

        _stack.push_back(Move(0, NoDirection));

        while (_openMoveCount < MyMPI::instance()->size() && !_stack.empty())
            step();

        if (_stack.empty())
        {
            sendFinishToSlaves();
            finish();
            return;
        }

        for (int target = 1; target < MyMPI::instance()->size(); ++target)
        {
            int currentDepth = 0;
            deque<Move>::iterator it = _stack.begin();

            _stackBuffer.clear();

            while (it->state != Open)
            {
                if (currentDepth != it->depth)
                {
                    currentDepth = it->depth;
                    _stackBuffer.push_back(1000);
                }

                if (it->state == Closed)
                    _stackBuffer.push_back(it->direction);

                ++it;
            }

            if (currentDepth != it->depth)
            {
                currentDepth = it->depth;
                _stackBuffer.push_back(1000);
            }

            _stackBuffer.push_back(-it->direction);

            Move& move = *it;

            move.state = Skip;
            --_openMoveCount;

            sendWork(_stackBuffer.data(), _stackBuffer.size(), target);
        }
    }
    else
    {
        recvBoard();
    }
}

void ParallelGame::solve()
{
    D(cerr << RANK << ": ParallelGame::solve" << endl);

    if (!MyMPI::instance()->isMaster())
    {
        while (_stack.empty() && !_finished)
            handleCommunication(true);
    }

    int counter = 0;

    while (!_finished)
    {
        D(cerr << RANK << ": active" << endl);

        while (!_stack.empty())
        {
            step();

            if ((++counter % 100) == 0)
                handleCommunication(false);
        }

        if (_finished || MyMPI::instance()->size() == 1)
            return;

        D(cerr << RANK << ": idle" << endl);

        if (MyMPI::instance()->isMaster())
        {
            _tokenColor = 'W';
        }

        if (_hasToken)
            sendToken();

        sendWorkRequest();

        while (_stack.empty() && !_finished)
            handleCommunication(true);
    }
}

void ParallelGame::initializeStack(int* buffer, int count)
{
    D(cerr << RANK << ": ParallelGame::initializeStack: start, stack size = " << count << endl);
    D(_board.print());

    _openMoveCount = 0;

    int depth = 0;
    for (int i = 0; i < count; ++i)
    {
        if (buffer[i] == 1000)
            ++depth;
        else if (buffer[i] < 0)
        {
            MoveDirection direction = MoveDirection(-buffer[i]);

            _stack.push_back(Move(depth, direction, Open));
            ++_openMoveCount;
        }
        else
        {
            MoveDirection direction = MoveDirection(buffer[i]);

            _board.makeMove(direction);
            _stack.push_back(Move(depth, direction, Closed));
        }
    }

    D(cerr << RANK << ": ParallelGame::initializeStack: done, open move count = " << _openMoveCount << endl);
}

void ParallelGame::sendBoard()
{
    D(cerr << RANK << ": ParallelGame::sendBoard" << endl);

    int bufferSize = 0;
    int* buffer = _board.serialize(&bufferSize);

    for (int i = 1; i < MyMPI::instance()->size(); ++i)
    {
        MyMPI::instance()->send(buffer, bufferSize, MPI_INT, i, MsgGameBoard);
    }

    delete [] buffer;
}

void ParallelGame::recvBoard()
{
    D(cerr << RANK << ": ParallelGame::recvBoard" << endl);

    int count = 0;
    MPI_Status status;

    MyMPI::instance()->waitForMessage(status, MsgGameBoard);
    MyMPI::instance()->getCount(status, MPI_INT, &count);

    if (_stackBuffer.size() < count)
        _stackBuffer.resize(count);

    MyMPI::instance()->recv(status, _stackBuffer.data(), count, MPI_INT);

    _board.deserialize(_stackBuffer.data());

    _bestMoves = new int[_board.upperBound()];

    D(_board.print());
}

void ParallelGame::sendWorkRequest()
{
    D(cerr << RANK << ": ParallelGame::sendWorkRequest: " << _nextWorkRequestTarget << endl);

    MyMPI::instance()->send(_nextWorkRequestTarget, MsgWorkRequest);
}

void ParallelGame::sendWork(int target)
{
    D(cerr << RANK << ": ParallelGame::sendWork: " << target << ", " << _openMoveCount << endl);

    if (_openMoveCount <= 1)
    {
        sendNoWork(target);
        return;
    }

    _stackBuffer.clear();

    int currentDepth = 0;
    deque<Move>::iterator it = _stack.begin();

    while (it->state != Open)
    {
        if (currentDepth != it->depth)
        {
            currentDepth = it->depth;
            _stackBuffer.push_back(1000);
        }

        if (it->state == Closed)
            _stackBuffer.push_back(it->direction);

        ++it;
    }

    if (currentDepth != it->depth)
    {
        currentDepth = it->depth;
        _stackBuffer.push_back(1000);
    }

    _stackBuffer.push_back(-it->direction);

    Move& move = *it;

    move.state = Skip;
    --_openMoveCount;

    sendWork(_stackBuffer.data(), _stackBuffer.size(), target);
}

void ParallelGame::sendWork(int* stack, int stackSize, int target)
{
    D(cerr << RANK << ": ParallelGame::sendWork: " << stackSize << ", " << target << ", ");
    D(for (int i = 0; i < stackSize; ++i))
    D({)
        D(if (stack[i] == 1000))
            D(cerr << " ");
        D(else if (stack[i] < 0))
            D(cerr << "(" << MoveDirectionNames[-stack[i]] << ")");
        D(else)
            D(cerr << MoveDirectionNames[stack[i]]);
    D(})
    D(cerr << endl);

    if (MyMPI::instance()->rank() > target)
        _tokenColor = 'B';

    MyMPI::instance()->send(stack, stackSize, MPI_INT, target, MsgWork);
}

void ParallelGame::sendNoWork(int target)
{
    D(cerr << RANK << ": ParallelGame::sendNoWork: " << target << endl);

    MyMPI::instance()->send(target, MsgWorkNoWork);
}

void ParallelGame::sendNewBest()
{
    D(cerr << RANK << ": ParallelGame::sendNewBest: new move count " << _bestMoveCount << endl);

    for (int i = 0; i < MyMPI::instance()->size(); ++i)
    {
        if (i == MyMPI::instance()->rank())
            continue;

        MyMPI::instance()->send(_bestMoves, _bestMoveCount, MPI_INT, i, MsgNewBest);
    }
}

void ParallelGame::sendToken()
{
    sendToken(_tokenColor);

    _tokenColor = 'W';
}

void ParallelGame::sendToken(char tokenColor)
{
    _hasToken = false;

    int target = (MyMPI::instance()->rank() + 1) % MyMPI::instance()->size();
    D(cerr << RANK << ": ParallelGame::sendToken: '" << tokenColor << "' to " << target << endl);
    MyMPI::instance()->send(&tokenColor, 1, MPI_CHAR, target, MsgToken);
}

void ParallelGame::sendFinishToSlaves()
{
    D(cerr << RANK << ": ParallelGame::sendFinishToSlaves" << endl);

    for (int i = 1; i < MyMPI::instance()->size(); ++i)
    {
        MyMPI::instance()->send(i, MsgFinish);
    }
}

void ParallelGame::handleCommunication(bool block)
{
    MPI_Status status;

    if (block)
    {
        D(cerr << RANK << ": ParallelGame::handleCommunication: sync" << endl);
        MyMPI::instance()->waitForMessage(status);
    }
    else
    {
        //cerr << RANK << ": ParallelGame::handleCommunication: async" << endl;
        if (!MyMPI::instance()->anyMessagePending(status))
            return;
    }

    switch (status.MPI_TAG)
    {
        case MsgGameBoard:
            // should never happen
            break;
        case MsgWorkRequest:
            handleWorkRequest(status);
            break;
        case MsgWork:
            handleWork(status);
            break;
        case MsgWorkNoWork:
            handleNoWork(status);
            break;
        case MsgNewBest:
            handleNewBest(status);
            break;
        case MsgToken:
            handleToken(status);
            break;
        case MsgFinish:
            handleFinish(status);
            break;
    }
}

void ParallelGame::handleWorkRequest(MPI_Status& status)
{
    D(cerr << RANK << ": ParallelGame::handleWorkRequest: " << status.MPI_SOURCE << endl);

    MyMPI::instance()->recv(status, 0, 0, MPI_CHAR);

    sendWork(status.MPI_SOURCE);
}

void ParallelGame::handleWork(MPI_Status& status)
{
    int count = 0;
    MyMPI::instance()->getCount(status, MPI_INT, &count);

    D(cerr << RANK << ": ParallelGame::handleWork: " << count << endl);

    if (_stackBuffer.size() < count)
        _stackBuffer.resize(count);

    MyMPI::instance()->recv(status, _stackBuffer.data(), count, MPI_INT);

    D(cerr << RANK << ": ParallelGame::handleWork: " << count << ", ");
    D(for (int i = 0; i < count; ++i))
    D({)
        D(if (_stackBuffer[i] == 1000))
            D(cerr << " ");
        D(else if (_stackBuffer[i] < 0))
            D(cerr << "(" << MoveDirectionNames[-_stackBuffer[i]] << ")");
        D(else)
            D(cerr << MoveDirectionNames[_stackBuffer[i]]);
    D(})
    D(cerr << endl);

    initializeStack(_stackBuffer.data(), count);
}

void ParallelGame::handleNoWork(MPI_Status& status)
{
    D(cerr << RANK << ": ParallelGame::handleNoWork" << endl);

    MyMPI::instance()->recv(status, 0, 0, MPI_CHAR);

    do
    {
        _nextWorkRequestTarget = (_nextWorkRequestTarget + 1) % MyMPI::instance()->size();
    } while (_nextWorkRequestTarget == MyMPI::instance()->rank());

    sendWorkRequest();
}

void ParallelGame::handleNewBest(MPI_Status& status)
{
    MyMPI::instance()->getCount(status, MPI_INT, &_bestMoveCount);

    D(cerr << RANK << ": ParallelGame::handleNewBest: new best move count " << _bestMoveCount << " from " << status.MPI_SOURCE << endl);

    MyMPI::instance()->recv(status, _bestMoves, _bestMoveCount, MPI_INT);
}

void ParallelGame::handleToken(MPI_Status& status)
{
    _hasToken = true;

    char newTokenColor = 'B';
    MyMPI::instance()->recv(status, &newTokenColor, 1, MPI_CHAR);

    D(cerr << RANK << ": ParallelGame::handleToken: new color '" << newTokenColor << "', old color = '" << _tokenColor << "'" << endl);

    if (MyMPI::instance()->isMaster())
    {
        if (newTokenColor == 'W')
        {
            sendFinishToSlaves();

            finish();
        }
        else if (_stack.empty())
            sendToken();
    }
    else
    {
        //if (newTokenColor == 'B')
        //    _tokenColor = 'B';

        if (_stack.empty())
            sendToken();
    }
}

void ParallelGame::handleFinish(MPI_Status& status)
{
    D(cerr << RANK << ": ParallelGame::handleFinish" << endl);

    MyMPI::instance()->recv(status, 0, 0, MPI_CHAR);

    if (MyMPI::instance()->isMaster())
        sendFinishToSlaves();

    finish();
}

void ParallelGame::finish()
{
    D(cerr << RANK << ": ParallelGame::finish" << endl);

    _finished = true;
}

void ParallelGame::onBestChanged()
{
    sendNewBest();
}