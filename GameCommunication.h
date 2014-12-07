#ifndef GAME_COMMUNICATION_H
#define GAME_COMMUNICATION_H

#include "GameBoard.h"

class GameCommunication
{
public:
    void sendBoard(const GameBoard& board);
    void recvBoard(GameBoard& board);
};

#endif