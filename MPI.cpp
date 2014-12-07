#include "MPI.h"

MPI* MPI::_instance = 0;

MPI::MPI()
        : _rank(-1), _size(0)
{
    _instance = this;
}

MPI* MPI::instance()
{
    return _instance;
}

void MPI::init(int* argc, char*** argv)
{
    MPI_Init(argc, argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &_size);
}

void MPI::finalize()
{
    MPI_Finalize();
}

bool MPI::isMaster() const
{
    return _rank == 0;
}