#include <iostream>

#include "MPI.h"
#include "Game.h"

using namespace std;

int main(int argc, char** argv)
{
    MPI mpi;
    Game* game = 0;

    mpi.init(&argc, &argv);

    if (mpi.isMaster())
    {
        srand(time(0));

        game = new Game(5, 5);

        // print initial game plan
        game->printGamePlan();

        // randomize game plan and calculate lower and upper bounds
        game->randomize();

        game->sendBoard();
    }
    else
    {
        unsigned char* serializedBoard = mpi.recvBoard();

        game = new Game();
        game->recvBoard();
    }

    game->solve();

    mpi.finalize();

    return 0;
}