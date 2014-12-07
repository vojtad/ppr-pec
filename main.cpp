#include <iostream>

using namespace std;

#define PARALLEL

#ifdef PARALLEL

#include "MPI.h"
#include "ParallelGame.h"

int main(int argc, char** argv)
{
    MPI mpi;
    ParallelGame* game = 0;

    mpi.init(&argc, &argv);

    if (mpi.isMaster())
    {
        srand(time(0));

        game = new ParallelGame(5, 5);

        // print initial game plan
        game->printGamePlan();

        // randomize game plan and calculate lower and upper bounds
        game->randomize();

        game->sendBoard();
    }
    else
    {
        unsigned char* serializedBoard = mpi.recvBoard();

        game = new ParallelGame();
        game->recvBoard();
    }

    game->solve();

    mpi.finalize();

    return 0;
}

#else

#include "SequentialGame.h"

int main()
{
    srand(time(0));

    SequentialGame game(5, 5);

    game.printGamePlan();
    game.randomize();

    game.solve();

    game.printBest();

    return 0;
}

#endif
