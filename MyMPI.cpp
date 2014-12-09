#include "MyMPI.h"

MyMPI* MyMPI::_instance = 0;

MyMPI::MyMPI()
        : _rank(-1), _size(0)
{
    _instance = this;
}

MyMPI* MyMPI::instance()
{
    return _instance;
}

int MyMPI::rank() const
{
    return _rank;
}

int MyMPI::size() const
{
    return _size;
}

void MyMPI::init(int* argc, char*** argv)
{
    MPI_Init(argc, argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &_size);
}

void MyMPI::finalize()
{
    MPI_Finalize();
}

bool MyMPI::isMaster() const
{
    return _rank == 0;
}

void MyMPI::barrier()
{
    MPI_Barrier(MPI_COMM_WORLD);
}

double MyMPI::time()
{
    return MPI_Wtime();
}

bool MyMPI::anyMessagePending(MPI_Status& status)
{
    int flag = 0;
    MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
    return flag != 0;
}

void MyMPI::waitForMessage(MPI_Status& status)
{
    waitForMessage(status, MPI_ANY_TAG);
}

void MyMPI::waitForMessage(MPI_Status& status, int tag)
{
    MPI_Probe(MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status);
}

void MyMPI::getCount(MPI_Status& status, MPI_Datatype datatype, int* count)
{
    MPI_Get_count(&status, datatype, count);
}

void MyMPI::send(int target, int tag)
{
    send(0, 0, MPI_INT, target, tag);
}

void MyMPI::send(void* buffer, int count, MPI_Datatype datatype, int target, int tag)
{
    MPI_Send(buffer, count, datatype, target, tag, MPI_COMM_WORLD);
}

void MyMPI::recv(const MPI_Status& status, void* buffer, int count, MPI_Datatype datatype)
{
    MPI_Recv(buffer, count, datatype, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}