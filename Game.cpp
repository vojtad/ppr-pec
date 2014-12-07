#include <iostream>

#include "Game.h"

#define D(x) //x

Game::Game()
        : _bestMoveCount(-1), _bestMoves(0)
{
}

Game::Game(int a, int b)
        : _board(a, b), _bestMoveCount(-1), _bestMoves(0)
{
}

Game::~Game()
{
}

void Game::printGamePlan() const
{
    _board.print();
}

void Game::printBest() const
{
    if (_bestMoveCount == -1)
    {
        cout << "No best." << endl;
        return;
    }

    cout << "Best move count is " << _bestMoveCount << "." << endl;

    cout << "    ";
    for (int i = 0; i < _bestMoveCount; ++i)
        cout << MoveDirectionNames[_bestMoves[i]] << ", ";
    cout << endl;
}

void Game::randomize()
{
    _board.randomize();

    _bestMoveCount = -1;
    delete[] _bestMoves;
    _bestMoves = new MoveDirection[_board.upperBound()];
}

void Game::step()
{
    Move& current = _stack.back();

    switch (current.state)
    {
        case Open:
            handleOpenMove(current);
            break;
        case Closed:
            handleClosedMove(current);
            break;
        case Skip:
            handleSkipMove(current);
            break;
    }
}

void Game::handleOpenMove(Move& current)
{
    _board.makeMove(current.direction);

    D(cout << "handleOpenMove(" << current.depth << ", " << MoveDirectionNames[current.direction] << "): " << _currentRow << ", " << _currentColumn << endl);

    current.state = Closed;

    if (isInAcceptableEndState(current))
    {
        cout << "Found solution with " << current.depth << " moves." << endl;

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
        }
    }
    else if (current.depth < _board.upperBound() && (_bestMoveCount == -1 || current.depth < _bestMoveCount))
    {
        if (_board.canMoveUp(current))
        {
            D(cout << "    add up" << endl);
            _stack.push_back(Move(current.depth + 1, Up));
        }

        if (_board.canMoveRight(current))
        {
            D(cout << "    add right" << endl);
            _stack.push_back(Move(current.depth + 1, Right));
        }

        if (_board.canMoveDown(current))
        {
            D(cout << "    add down" << endl);
            _stack.push_back(Move(current.depth + 1, Down));
        }

        if (_board.canMoveLeft(current))
        {
            D(cout << "    add left" << endl);
            _stack.push_back(Move(current.depth + 1, Left));
        }
    }
    else
        handleClosedMove(current);
}

void Game::handleClosedMove(Move& current)
{
    _board.makeMove(current.oppositeDirection());

    D(cout << "handleClosedMove(" << current.depth << ", " << MoveDirectionNames[current.direction] << "): " << _currentRow << ", " << _currentColumn << endl);

    _stack.pop_back();
}

void Game::handleSkipMove(Move& current)
{
    _stack.pop_back();
}

bool Game::isInAcceptableEndState(const Move& current) const
{
    if (current.depth < _board.lowerBound())
        return false;

    return _board.isInAcceptableEndState();
}