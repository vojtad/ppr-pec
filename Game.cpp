#include <iostream>

#include "Game.h"

#define D(x) //x

Game::Game()
        : _bestMoveCount(-1), _bestMoves(0), _openMoveCount(0)
{
}

Game::Game(int a, int b)
        : _board(a, b), _bestMoveCount(-1), _bestMoves(0), _openMoveCount(0)
{
}

Game::~Game()
{
    delete [] _bestMoves;
}

void Game::printGamePlan() const
{
    _board.print();
}

void Game::printBest() const
{
    if (_bestMoveCount == -1)
    {
        cout << "No solution found." << endl;
        return;
    }

    cout << _bestMoveCount << " moves" << endl;

    for (int i = 0; i < _bestMoveCount; ++i)
        cout << MoveDirectionNames[_bestMoves[i]];
    cout << endl;
}

void Game::load(const char* filename)
{
    D(cerr << "Game::load: " << filename << endl);

    _board.load(filename);

    _bestMoveCount = -1;
    delete[] _bestMoves;
    _bestMoves = new int[_board.upperBound()];
}

void Game::randomize()
{
    srand(time(0));

    _board.randomize();

    _bestMoveCount = -1;
    delete[] _bestMoves;
    _bestMoves = new int[_board.upperBound()];
}

void Game::randomize(int q)
{
    srand(time(0));

    _board.randomize(q);

    _bestMoveCount = -1;
    delete[] _bestMoves;
    _bestMoves = new int[_board.upperBound()];
}

void Game::step()
{
    bool pop = false;

    {
        Move& current = _stack.back();

        switch (current.state)
        {
            case Open:
                handleOpenMove(current);
                break;
            case Closed:
                handleClosedMove(current);
                pop = true;
                break;
            case Skip:
                handleSkipMove(current);
                pop = true;
                break;
        }
    }

    if (pop)
        _stack.pop_back();
}

void Game::handleOpenMove(Move& current)
{
    D(cerr << "handleOpenMove(" << current.depth << ", " << MoveDirectionNames[current.direction] << ")" << endl);
    D(_board.print());

    _board.makeMove(current.direction);

    D(_board.print());
    D(cerr << endl);

    current.state = Closed;
    --_openMoveCount;

    if (isInAcceptableEndState(current))
    {
        D(cerr << "Found solution with " << current.depth << " moves." << endl);

        if (current.depth < _bestMoveCount || _bestMoveCount == -1)
        {
            int i = 0;
            for (deque<Move>::const_iterator it = _stack.begin(); it != _stack.end(); ++it)
            {
                const Move& move = *it;

                if (move.state != Closed || move.direction == NoDirection)
                    continue;

                _bestMoves[i++] = move.direction;
            }

            _bestMoveCount = current.depth;

            onBestChanged();
        }
    }
    else if (current.depth < _board.upperBound() && (_bestMoveCount == -1 || current.depth < (_bestMoveCount - 1)))
    {
        if (_board.canMoveUp(current))
        {
            D(cerr << "    add up" << endl);
            _stack.push_back(Move(current.depth + 1, Up));
            ++_openMoveCount;
        }

        if (_board.canMoveRight(current))
        {
            D(cerr << "    add right" << endl);
            _stack.push_back(Move(current.depth + 1, Right));
            ++_openMoveCount;
        }

        if (_board.canMoveDown(current))
        {
            D(cerr << "    add down" << endl);
            _stack.push_back(Move(current.depth + 1, Down));
            ++_openMoveCount;
        }

        if (_board.canMoveLeft(current))
        {
            D(cerr << "    add left" << endl);
            _stack.push_back(Move(current.depth + 1, Left));
            ++_openMoveCount;
        }
    }
}

void Game::handleClosedMove(Move& current)
{
    D(cerr << "handleClosedMove(" << current.depth << ", " << MoveDirectionNames[current.direction] << ")" << endl);
    D(_board.print());

    _board.makeMove(current.oppositeDirection());

    D(_board.print());
    D(cerr << endl);


    //_stack.pop_back();
}

void Game::handleSkipMove(Move& current)
{
    D(cerr << "Game::handleClosedMove(" << current.depth << ", " << MoveDirectionNames[current.direction] << ")" << endl);

    //_stack.pop_back();
}

bool Game::isInAcceptableEndState(const Move& current) const
{
    if (current.depth < _board.lowerBound())
        return false;

    return _board.isInAcceptableEndState();
}

void Game::onBestChanged()
{

}