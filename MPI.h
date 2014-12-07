#ifndef MPI_H
#define MPI_H

#include <mpi.h>

#define

class MPI
{
public:
    MPI();

    static MPI* instance();

    void init(int* argc, char*** argv);
    void finalize();

    bool isMaster() const;

    unsigned char* recvBoard();
    void sendBoard(const unsigned char* work, int length);



private:
    static MPI* _instance;

    int _rank, _size;
};

#endif