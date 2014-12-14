#include "ParallelGame.h"

#include <cctype>

#include "MyMPI.h"

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
    cout << RANK << ": ParallelGame::prepare" << endl;

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
    cout << RANK << ": ParallelGame::solve" << endl;

    if (!MyMPI::instance()->isMaster())
    {
        while (_stack.empty() && !_finished)
            handleCommunication(true);
    }

    int counter = 0;

    while (!_finished)
    {
        cout << RANK << ": active" << endl;

        while (!_stack.empty())
        {
            step();

            if ((++counter % 100) == 0)
                handleCommunication(false);
        }

        if (_finished || MyMPI::instance()->size() == 1)
            return;

        cout << RANK << ": idle" << endl;

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
    cout << RANK << ": ParallelGame::initializeStack: start, stack size = " << count << endl;

    _board.print();

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

    cout << RANK << ": ParallelGame::initializeStack: done, open move count = " << _openMoveCount << endl;
}

void ParallelGame::sendBoard()
{
    cout << RANK << ": ParallelGame::sendBoard" << endl;

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
    cout << RANK << ": ParallelGame::recvBoard" << endl;

    int count = 0;
    MPI_Status status;

    MyMPI::instance()->waitForMessage(status, MsgGameBoard);
    MyMPI::instance()->getCount(status, MPI_INT, &count);

    if (_stackBuffer.size() < count)
        _stackBuffer.resize(count);

    MyMPI::instance()->recv(status, _stackBuffer.data(), count, MPI_INT);

    _board.deserialize(_stackBuffer.data());

    _bestMoves = new int[_board.upperBound()];

    _board.print();
}

void ParallelGame::sendWorkRequest()
{
    cout << RANK << ": ParallelGame::sendWorkRequest: " << _nextWorkRequestTarget << endl;

    MyMPI::instance()->send(_nextWorkRequestTarget, MsgWorkRequest);
}

void ParallelGame::sendWork(int target)
{
    cout << RANK << ": ParallelGame::sendWork: " << target << ", " << _openMoveCount << endl;

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
    cout << RANK << ": ParallelGame::sendWork: " << stackSize << ", " << target << ", ";
    for (int i = 0; i < stackSize; ++i)
    {
        if (stack[i] == 1000)
            cout << " ";
        else if (stack[i] < 0)
            cout << "(" << MoveDirectionNames[-stack[i]] << ")";
        else
            cout << MoveDirectionNames[stack[i]];
    }
    cout << endl;

    if (MyMPI::instance()->rank() > target)
        _tokenColor = 'B';

    MyMPI::instance()->send(stack, stackSize, MPI_INT, target, MsgWork);
}

void ParallelGame::sendNoWork(int target)
{
    cout << RANK << ": ParallelGame::sendNoWork: " << target << endl;

    MyMPI::instance()->send(target, MsgWorkNoWork);
}

void ParallelGame::sendNewBest()
{
    cout << RANK << ": ParallelGame::sendNewBest: new move count " << _bestMoveCount << endl;

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
    cout << RANK << ": ParallelGame::sendToken: '" << tokenColor << "' to " << target << endl;
    MyMPI::instance()->send(&tokenColor, 1, MPI_CHAR, target, MsgToken);
}

void ParallelGame::sendFinishToSlaves()
{
    cout << RANK << ": ParallelGame::sendFinishToSlaves" << endl;

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
        cout << RANK << ": ParallelGame::handleCommunication: sync" << endl;
        MyMPI::instance()->waitForMessage(status);
    }
    else
    {
        //cout << RANK << ": ParallelGame::handleCommunication: async" << endl;
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
    cout << RANK << ": ParallelGame::handleWorkRequest: " << status.MPI_SOURCE << endl;

    MyMPI::instance()->recv(status, 0, 0, MPI_CHAR);

    sendWork(status.MPI_SOURCE);
}

void ParallelGame::handleWork(MPI_Status& status)
{
    int count = 0;
    MyMPI::instance()->getCount(status, MPI_INT, &count);

    cout << RANK << ": ParallelGame::handleWork: " << count << endl;

    if (_stackBuffer.size() < count)
        _stackBuffer.resize(count);

    MyMPI::instance()->recv(status, _stackBuffer.data(), count, MPI_INT);

    cout << RANK << ": ParallelGame::handleWork: " << count << ", ";
    for (int i = 0; i < count; ++i)
    {
        if (_stackBuffer[i] == 1000)
            cout << " ";
        else if (_stackBuffer[i] < 0)
            cout << "(" << MoveDirectionNames[-_stackBuffer[i]] << ")";
        else
            cout << MoveDirectionNames[_stackBuffer[i]];
    }
    cout << endl;

    initializeStack(_stackBuffer.data(), count);
}

void ParallelGame::handleNoWork(MPI_Status& status)
{
    cout << RANK << ": ParallelGame::handleNoWork" << endl;

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

    cout << RANK << ": ParallelGame::handleNewBest: new best move count " << _bestMoveCount << " from " << status.MPI_SOURCE << endl;

    MyMPI::instance()->recv(status, _bestMoves, _bestMoveCount, MPI_INT);
}

void ParallelGame::handleToken(MPI_Status& status)
{
    _hasToken = true;

    char newTokenColor = 'B';
    MyMPI::instance()->recv(status, &newTokenColor, 1, MPI_CHAR);

    cout << RANK << ": ParallelGame::handleToken: new color '" << newTokenColor << "', old color = '" << _tokenColor << "'" << endl;

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
    cout << RANK << ": ParallelGame::handleFinish" << endl;

    MyMPI::instance()->recv(status, 0, 0, MPI_CHAR);

    if (MyMPI::instance()->isMaster())
        sendFinishToSlaves();

    finish();
}

void ParallelGame::finish()
{
    cout << RANK << ": ParallelGame::finish" << endl;

    _finished = true;
}

void ParallelGame::onBestChanged()
{
    sendNewBest();
}