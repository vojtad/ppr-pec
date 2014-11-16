#include <deque>
#include <iostream>

using namespace std;

#define D(x) //x

enum MoveDirection
{
    NoDirection = 0,
    Up,
    Right,
    Down,
    Left
};

const char* MoveDirectionNames[] = {
        "NoDirection", "Up", "Right", "Down", "Left"
};

enum MoveState
{
    Open,
    Closed
};

struct Move
{
    int depth;

    MoveDirection direction;
    MoveState state;

    Move(const Move& m)
            : depth(m.depth), direction(m.direction), state(m.state)
    {
    }

    Move(int depth, MoveDirection direction)
            : depth(depth), direction(direction), state(Open)
    {
    }
};

class Game
{
    public:
    Game(int a, int b)
        : _columns(a), _rows(b), _q(0), _d(0), _gamePlan(0), _currentRow(b - 1), _currentColumn(a - 1),
          _bestMoveCount(-1), _bestMoves(0)
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

    ~Game()
    {
        for (int r = 0; r < _rows; ++r)
        {
            delete [] _gamePlan[r];
            _gamePlan[r] = 0;
        }

        delete [] _gamePlan;
    }

    int upperBound() const
    {
        return _q;
    }

    int lowerBound() const
    {
        return _d;
    }

    void printGamePlan() const
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
    }

    void printBest() const
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

    void randomize()
    {
        _q = 1 + rand() % (_rows * _columns - 1);

        _bestMoveCount = -1;
        delete [] _bestMoves;
        _bestMoves = new MoveDirection[_q];

        randomizeStep(1);

        calcLowerBound();
    }

    void walkthru()
    {
        _stack.push_back(Move(0, NoDirection));

        while (!_stack.empty())
        {
            Move& current = _stack.back();

            switch (current.state)
            {
                case Open:
                    walkthruOpen(current);
                    break;
                case Closed:
                    walkthruClosed(current);
                    break;
            }
        }
    }

    private:
    void randomizeStep(int s)
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

    void calcLowerBound()
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
                cout << _d << " " << r << ", " << c << ": " << _gamePlan[r][c] << " - " << v << endl;
            }
        }
    }

    bool canMoveUp(const Move& from) const
    {
        return from.direction != Down && _currentRow > 0;
    }

    bool canMoveRight(const Move& from) const
    {
        return from.direction != Left && _currentColumn < (_columns - 1);
    }

    bool canMoveDown(const Move& from) const
    {
        return from.direction != Up && _currentRow < (_rows - 1);
    }

    bool canMoveLeft(const Move& from) const
    {
        return from.direction != Right && _currentColumn > 0;
    }

    void walkthruOpen(Move& current)
    {
        int oldRow = _currentRow, oldColumn = _currentColumn;

        switch (current.direction)
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

        D(cout << "walkthruOpen(" << current.depth << ", " << MoveDirectionNames[current.direction] << "): " << _currentRow << ", " << _currentColumn << endl);

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
        else if (current.depth < _q && (_bestMoveCount == -1 || current.depth < _bestMoveCount))
        {
            if (canMoveUp(current))
            {
                D(cout << "    add up" << endl);
                _stack.push_back(Move(current.depth + 1, Up));
            }

            if (canMoveRight(current))
            {
                D(cout << "    add right" << endl);
                _stack.push_back(Move(current.depth + 1, Right));
            }

            if (canMoveDown(current))
            {
                D(cout << "    add down" << endl);
                _stack.push_back(Move(current.depth + 1, Down));
            }

            if (canMoveLeft(current))
            {
                D(cout << "    add left" << endl);
                _stack.push_back(Move(current.depth + 1, Left));
            }
        }
        else
            walkthruClosed(current);
    }

    void walkthruClosed(Move& current)
    {
        int oldRow = _currentRow, oldColumn = _currentColumn;

        switch (current.direction)
        {
            case Up:
                ++_currentRow;
                break;
            case Right:
                --_currentColumn;
                break;
            case Down:
                --_currentRow;
                break;
            case Left:
                ++_currentColumn;
                break;
            case NoDirection:
                break;
        }

        _gamePlan[oldRow][oldColumn] = _gamePlan[_currentRow][_currentColumn];
        _gamePlan[_currentRow][_currentColumn] = 0;

        D(cout << "walkthruClosed(" << current.depth << ", " << MoveDirectionNames[current.direction] << "): " << _currentRow << ", " << _currentColumn << endl);

        _stack.pop_back();
    }

    bool isInAcceptableEndState(const Move& current) const
    {
        if (current.depth < _d)
            return false;

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

    private:
    int _columns, _rows;
    int _q, _d;

    int** _gamePlan;
    int _currentRow, _currentColumn;

    int _bestMoveCount;
    MoveDirection* _bestMoves;

    deque<Move> _stack;
};

int main()
{
    srand(time(0));

    // initialize game
    // first parameter of Game constructor is column count and second one is row count
    Game game(5, 5);

    // print initial game plan
    game.printGamePlan();

    // randomize game plan and calculate lower and upper bounds
    game.randomize();

    cout << endl;

    // print randomized game plan
    game.printGamePlan();

    // print calculated lower and upper bounds for bb-dfs
    cout << "q = " << game.upperBound() << endl << "d = " << game.lowerBound() << endl;

    // perform bb-dfs and find the best solution
    game.walkthru();

    // print best solution
    game.printBest();

    return 0;
}