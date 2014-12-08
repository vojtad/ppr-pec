#include <iostream>

using namespace std;

#ifdef BUILD_PARALLEL

#include "MPI.h"
#include "ParallelGame.h"

int main(int argc, char** argv)
{
    MPI mpi;
    ParallelGame* game = 0;
    double t1, t2;

    mpi.init(&argc, &argv);

    if (mpi.isMaster())
    {
        srand(time(0));

        game = new ParallelGame(5, 5);

        // print initial game plan
        game->printGamePlan();

        // randomize game plan and calculate lower and upper bounds
        game->randomize();

        mpi.barrier();
        t1 = mpi.time();
    }
    else
    {
        game = new ParallelGame();

        mpi.barrier();
        t1 = mpi.time();
    }

    game->prepare();
    game->solve();

    mpi.barrier();
    t2 = mpi.time();

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
    game.printGamePlan();

    cout << endl;

    game.solve();

    game.printBest();

    return 0;
}

#endif
