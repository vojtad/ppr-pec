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

        int* stackBuffer = new int[_stack.size()];

        for (int target = 1; target < MyMPI::instance()->size(); ++target)
        {
            int i = 0, currentDepth = 0;
            deque<Move>::iterator it = _stack.begin();

            while (it->state != Open)
            {
                if (currentDepth != it->depth)
                {
                    currentDepth = it->depth;
                    stackBuffer[i++] = 1000;
                }

                if (it->state == Closed)
                    stackBuffer[i++] = it->direction;

                ++it;
            }

            if (currentDepth != it->depth)
            {
                currentDepth = it->depth;
                stackBuffer[i++] = 1000;
            }

            stackBuffer[i++] = -it->direction;

            Move& move = *it;

            move.state = Skip;

            sendWork(stackBuffer, i, target);
        }

        delete [] stackBuffer;
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

        _tokenColor = 'W';

        if (_hasToken)
            sendToken();

        cout << RANK << ": requesting work from " << _nextWorkRequestTarget << endl;
        MyMPI::instance()->send(_nextWorkRequestTarget, MsgWorkRequest);

        while (_stack.empty() && !_finished)
            handleCommunication(true);
    }
}

void ParallelGame::initializeStack(int* buffer)
{
    int i = 0, depth = 0;
    for (; buffer[i] >= 0; ++i)
    {
        if (buffer[i] == 1000)
            ++depth;
        else
        {
            MoveDirection direction = MoveDirection(buffer[i]);

            _board.makeMove(direction);
            _stack.push_back(Move(depth, direction, Closed));
        }
    }

    _stack.push_back(Move(depth, MoveDirection(-buffer[i]), Open));
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
    int* buffer;
    MPI_Status status;

    MyMPI::instance()->waitForMessage(status, MsgGameBoard);
    MyMPI::instance()->getCount(status, MPI_INT, &count);

    buffer = new int[count];

    MyMPI::instance()->recv(status, buffer, count, MPI_INT);

    _board.deserialize(buffer);

    delete [] buffer;

    _bestMoves = new int[_board.upperBound()];

    _board.print();
}

void ParallelGame::sendWork(int* stack, int stackSize, int target)
{
    cout << RANK << ": ParallelGame::sendWork: " << stackSize << ", ";
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
    _hasToken = false;

    int target = (MyMPI::instance()->rank() + 1) % MyMPI::instance()->size();
    cout << RANK << ": ParallelGame::sendToken: '" << _tokenColor << "' to " << target << endl;
    MyMPI::instance()->send(&_tokenColor, 1, MPI_CHAR, target, MsgToken);
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
    cout << RANK << ": ParallelGame::handleCommunication: " << (block ? "sync" : "async") << endl;

    MPI_Status status;

    if (block)
    {
        MyMPI::instance()->waitForMessage(status);
    }
    else
    {
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

    sendNoWork(status.MPI_SOURCE);
}

void ParallelGame::handleWork(MPI_Status& status)
{
    cout << RANK << ": ParallelGame::handleWork" << endl;

    int count = 0;
    MyMPI::instance()->getCount(status, MPI_INT, &count);

    int* buffer = new int[count];
    MyMPI::instance()->recv(status, buffer, count, MPI_INT);

    cout << RANK << ": ParallelGame::sendWork: " << count << ", ";
    for (int i = 0; i < count; ++i)
    {
        if (buffer[i] == 1000)
            cout << " ";
        else if (buffer[i] < 0)
            cout << "(" << MoveDirectionNames[-buffer[i]] << ")";
        else
            cout << MoveDirectionNames[buffer[i]];
    }
    cout << endl;

    initializeStack(buffer);

    delete [] buffer;
}

void ParallelGame::handleNoWork(MPI_Status& status)
{
    cout << RANK << ": ParallelGame::handleNoWork" << endl;

    MyMPI::instance()->recv(status, 0, 0, MPI_CHAR);

    do
    {
        _nextWorkRequestTarget = (_nextWorkRequestTarget + 1) % MyMPI::instance()->size();
    } while (_nextWorkRequestTarget == MyMPI::instance()->rank());
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

    if (newTokenColor == 'B')
        _tokenColor = 'B';

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
        if (newTokenColor == 'B')
            _tokenColor = 'B';

        if (_tokenColor == 'B' || _stack.empty())
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