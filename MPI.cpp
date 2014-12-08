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

void MPI::barrier()
{
    MPI_Barrier();
}

double MPI::time()
{
    return MPI_Wtime();
}

bool MPI::anyMessagePending(MPI_Status& status)
{
    int flag = 0;
    MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
    return flag != 0;
}

void MPI::waitForMessage(MPI_Status& status)
{
    waitForMessage(status, MPI_ANY_TAG);
}

void MPI::waitForMessage(MPI_Status& status, int tag)
{
    MPI_Probe(MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status);
}

void MPI::getCount(MPI_Status& status, MPI_Datatype datatype, int* count)
{
    MPI_Get_count(&status, datatype, count);
}

void MPI::send(int target, int tag)
{
    send(0, 0, MPI_INT, target, tag);
}

void MPI::send(void* buffer, int* count, MPI_Datatype datatype, int target, int tag)
{
    MPI_Send(buffer, count, datatype, target, tag, MPI_COMM_WORLD);
}

void MPI::recv(const MPI_Status& status, void* buffer, int* count, MPI_Datatype datatype)
{
    MPI_Recv(buffer, count, datatype, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}