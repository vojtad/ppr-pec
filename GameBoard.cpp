#include "GameBoard.h"

#include <iostream>

using namespace std;

GameBoard::GameBoard()
        : _columns(0), _rows(0), _q(0), _d(0), _gamePlan(0), _currentRow(0), _currentColumn(0)
{
}

GameBoard::GameBoard(int a, int b)
        : _columns(a), _rows(b), _q(0), _d(0), _gamePlan(0), _currentRow(b - 1), _currentColumn(a - 1)
{
    _gamePlan = new int* [_rows];
    for (int r = 0; r < _rows; ++r)
    {
        _gamePlan[r] = new int[_columns];

        for (int c = 0; c < _columns; ++c)
            _gamePlan[r][c] = (r * _columns) + c + 1;
    }

    _gamePlan[_currentRow][_currentColumn] = 0;
}

GameBoard::~GameBoard()
{
    for (int r = 0; r < _rows; ++r)
    {
        delete[] _gamePlan[r];
        _gamePlan[r] = 0;
    }

    delete[] _gamePlan;
}

int GameBoard::upperBound() const
{
    return _q;
}

int GameBoard::lowerBound() const
{
    return _d;
}

void GameBoard::print() const
{
    for (int r = 0; r < _rows; ++r)
    {
        for (int c = 0; c < _columns; ++c)
        {
            cout.width(4);
            cout.fill(' ');
            cout << left << _gamePlan[r][c];
        }

        cout << endl;
    }

    cout << "Lower bound: " << _d << endl;
    cout << "Upper bound: " << _q << endl;
}

void GameBoard::randomize()
{
    _q = 1 + rand() % (_rows * _columns - 1);

    randomizeStep(1);

    calcLowerBound();
}

void GameBoard::calcLowerBound()
{
    _d = 0;
    for (int r = 0; r < _rows; ++r)
    {
        for (int c = 0; c < _columns; ++c)
        {
            if (_gamePlan[r][c] == 0)
                continue;

            int v = (r * _columns) + c + 1;
            if (_gamePlan[r][c] == v)
                continue;

            _d += abs(r - ((_gamePlan[r][c] - 1) / _columns)) + abs(c - ((_gamePlan[r][c] - 1) % _columns));
        }
    }
}

bool GameBoard::canMoveUp(const Move& from) const
{
    return from.direction != Down && _currentRow > 0;
}

bool GameBoard::canMoveRight(const Move& from) const
{
    return from.direction != Left && _currentColumn < (_columns - 1);
}

bool GameBoard::canMoveDown(const Move& from) const
{
    return from.direction != Up && _currentRow < (_rows - 1);
}

bool GameBoard::canMoveLeft(const Move& from) const
{
    return from.direction != Right && _currentColumn > 0;
}

void GameBoard::makeMove(MoveDirection direction)
{
    int oldRow = _currentRow, oldColumn = _currentColumn;

    switch (direction)
    {
        case Up:
            --_currentRow;
            break;
        case Right:
            ++_currentColumn;
            break;
        case Down:
            ++_currentRow;
            break;
        case Left:
            --_currentColumn;
            break;
        case NoDirection:
            break;
    }

    _gamePlan[oldRow][oldColumn] = _gamePlan[_currentRow][_currentColumn];
    _gamePlan[_currentRow][_currentColumn] = 0;
}

bool GameBoard::isInAcceptableEndState() const
{
    if (_currentRow != (_rows - 1) || _currentColumn != (_columns - 1))
        return false;

    for (int r = 0; r < _rows; ++r)
    {
        for (int c = 0; c < _columns; ++c)
        {
            if (r == (_rows - 1) && c == (_columns - 1))
                break;

            int v = (r * _columns) + c + 1;
            if (_gamePlan[r][c] != v)
                return false;
        }
    }

    return true;
}

void GameBoard::randomizeStep(int s)
{
    int dR = 0, dC = 0;
    int d = rand() % 4;

    if (d == 0)
        dR = _currentRow > 0 ? -1 : 1;
    else if (d == 1)
        dC = _currentColumn < (_columns - 1) ? 1 : -1;
    else if (d == 2)
        dR = _currentRow < (_rows - 1) ? 1 : -1;
    else
        dC = _currentColumn > 0 ? -1 : 1;

    int nR = _currentRow + dR, nC = _currentColumn + dC;

    _gamePlan[_currentRow][_currentColumn] = _gamePlan[nR][nC];
    _currentRow = nR;
    _currentColumn = nC;
    _gamePlan[_currentRow][_currentColumn] = 0;

    if (s < _q)
        randomizeStep(s + 1);
}