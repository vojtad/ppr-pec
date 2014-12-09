#include <iostream>

using namespace std;

#ifdef BUILD_PARALLEL

#include "MyMPI.h"
#include "ParallelGame.h"

int main(int argc, char** argv)
{
    MyMPI mpi;
    ParallelGame* game = 0;
    double t1, t2;

    mpi.init(&argc, &argv);

    cout << RANK << ": started." << endl;

    if (mpi.isMaster())
    {
        srand(time(0));

        game = new ParallelGame(5, 5);

        // randomize game plan and calculate lower and upper bounds
        game->randomize();

        // print initial game plan
        cout << RANK << ": initial game board" << endl;
        game->printGamePlan();

        mpi.barrier();
        t1 = mpi.time();
    }
    else
    {
        game = new ParallelGame();

        cout << "Slave barrier." << endl;

        mpi.barrier();
        t1 = mpi.time();
    }

    game->prepare();
    game->solve();

    mpi.barrier();
    t2 = mpi.time();

    if (MyMPI::instance()->isMaster())
    {
        game->printBest();

        cout << "Solution finding took " << (t2 - t1) << " seconds." << endl;
    }

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
