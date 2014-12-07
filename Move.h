#ifndef MOVE_H
#define MOVE_H

enum MoveDirection
{
    NoDirection = 0,
    Up,
    Right,
    Down,
    Left
};

extern const char* MoveDirectionNames[5];

enum MoveState
{
    Open,
    Closed,
    Skip
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

    MoveDirection oppositeDirection()
    {
        switch (direction)
        {
            case NoDirection:
                return NoDirection;

            case Up:
                return Down;

            case Right:
                return Left;

            case Down:
                return Up;

            case Left:
                return Right;
        }

        return NoDirection;
    }
};

#endif