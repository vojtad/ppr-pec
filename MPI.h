#ifndef MPI_H
#define MPI_H

#include <mpi.h>

class MPI
{
public:
    MPI();

    static MPI* instance();

    void init(int* argc, char*** argv);
    void finalize();

    bool isMaster() const;

    void barrier();
    double time();

    bool anyMessagePending(MPI_Status& status);

    void waitForMessage(MPI_Status& status);
    void waitForMessage(MPI_Status& status, int tag);

    void getCount(MPI_Status& status, MPI_Datatype datatype, int* count);

    void send(int target, int tag);
    void send(void* buffer, int* count, MPI_Datatype datatype, int target, int tag);
    void recv(const MPI_Status& status, void* buffer, int* count, MPI_Datatype datatype);

private:
    static MPI* _instance;

    int _rank, _size;
};

#endif