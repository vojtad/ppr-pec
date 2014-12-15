#include <iostream>

using namespace std;

#ifdef BUILD_PARALLEL

#include "MyMPI.h"
#include "ParallelGame.h"

#define D(x) //x

int main(int argc, char** argv)
{
    MyMPI mpi;
    ParallelGame* game = 0;
    double t1, t2;

    mpi.init(&argc, &argv);

    D(cerr << RANK << ": started." << endl);

    if (mpi.isMaster())
    {
        if (argc == 2)
        {
            game = new ParallelGame();
            game->load(argv[1]);
        }
        else if (argc == 4)
        {
            game = new ParallelGame(atoi(argv[1]), atoi(argv[2]));
            game->randomize(atoi(argv[3])); // randomize game plan and calculate lower and upper bounds
        }
        else
        {
            game = new ParallelGame(5, 5);
            game->randomize(); // randomize game plan and calculate lower and upper bounds
        }

        // print initial game plan
        D(cerr << RANK << ": initial game board" << endl);
        D(game->printGamePlan());

        mpi.barrier();
        t1 = mpi.time();
    }
    else
    {
        game = new ParallelGame();

        D(cerr << "Slave barrier." << endl);

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

        cout << "Time: " << (t2 - t1) << endl;
        cout << MyMPI::instance()->size() << (t2 - t1) << endl;
    }

    mpi.finalize();

    return 0;
}

#else

#include "SequentialGame.h"
#include <sys/time.h>

int main(int argc, char** argv)
{
    struct timeval tv;

    SequentialGame* game = 0;

    if (argc == 2)
        {
            game = new SequentialGame();
            game->load(argv[1]);
        }
        else if (argc == 4)
        {
            game = new SequentialGame(atoi(argv[1]), atoi(argv[2]));
            game->randomize(atoi(argv[3])); // randomize game plan and calculate lower and upper bounds
        }
        else
        {
            game = new SequentialGame(5, 5);
            game->randomize(); // randomize game plan and calculate lower and upper bounds
        }

    gettimeofday(&tv, 0);
    double t1 = tv.tv_sec + tv.tv_usec / 1000000.0;
    game->solve();

    game->printBest();

    gettimeofday(&tv, 0);
    double t2 = tv.tv_sec + tv.tv_usec / 1000000.0;

    cout << "Time: " << (t2 - t1) << endl;

    return 0;
}

#endif
