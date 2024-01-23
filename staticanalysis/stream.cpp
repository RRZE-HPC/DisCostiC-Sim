#include <iostream>
#include <mpi.h>
#include <time.h>

static double
getTimeStamp()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1.e-9;
}

static double
getTimeResolution()
{
    struct timespec ts;
    clock_getres(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1.e-9;
}

int main(int argc, char **argv)
{
    int rank, systemsize, nTimeStep;
    long int N = 1000000000, Nlocal;
    double *a, *b, *c;
    double s = 55;
    double S, E, kernelS, kernelT, buff, res;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &systemsize);

    if (argc > 1)
    {
        N = atol(argv[1]);
    }

    if (argc > 2)
    {
        nTimeStep = atoi(argv[2]);
    }

    Nlocal = N / systemsize;

    size_t memA = posix_memalign((void **)&a, 64, Nlocal * sizeof(double)); //!< dynamic allocation of the array a
    size_t memB = posix_memalign((void **)&b, 64, Nlocal * sizeof(double)); //!< dynamic allocation of the array b
    size_t memC = posix_memalign((void **)&c, 64, Nlocal * sizeof(double)); //!< dynamic allocation of the array c

    if (memA != 0)
    {
        throw std::runtime_error(" error: allocation of array A FAIL");
        MPI_Abort(MPI_COMM_WORLD, 2);
    }
    if (memB != 0)
    {
        throw std::runtime_error(" error: allocation of array B FAIL");
        MPI_Abort(MPI_COMM_WORLD, 2);
    }
    if (memC != 0)
    {
        throw std::runtime_error(" error: allocation of array C FAIL");
        MPI_Abort(MPI_COMM_WORLD, 2);
    }

    for (long int i = 0; i < Nlocal; ++i)
    {
        a[i] = 1.0 * rank;
        b[i] = 2.0 * rank;
        c[i] = 3.0 * rank;
    }

    if (rank == 0)
    {
        std::cout << "A allocated : " << (double)Nlocal / 1024 / 1024 / 1024 << " GB" << std::endl;
        std::cout << "B allocated : " << (double)Nlocal / 1024 / 1024 / 1024 << " GB" << std::endl;
        std::cout << "C allocated : " << (double)Nlocal / 1024 / 1024 / 1024 << " GB" << std::endl;
    }

    MPI_Barrier(MPI_COMM_WORLD);

    S = getTimeStamp();

    for (int timeStep = 0; timeStep < nTimeStep; ++timeStep)
    {

        kernelS = getTimeStamp();

        for (long int i = 0; i < Nlocal; ++i)
            a[i] = b[i] + s * c[i];

        kernelT = getTimeStamp() - kernelS;

        if (a[Nlocal / 2] < 0)
            Nlocal = -1;

        MPI_Request requests[4] = {MPI_REQUEST_NULL, MPI_REQUEST_NULL, MPI_REQUEST_NULL, MPI_REQUEST_NULL};
        res = kernelT;
        
        /* exchange ghost cells with top neighbor */
        if (rank + 1 < systemsize)
        {
            int top = rank + 1;
            MPI_Isend(&res, 1, MPI_DOUBLE, top, 1, MPI_COMM_WORLD, &requests[0]);
            MPI_Irecv(&res, 1, MPI_DOUBLE, top, 2, MPI_COMM_WORLD, &requests[1]);
        }

        /* exchange ghost cells with bottom neighbor */
        if (rank > 0)
        {
            int bottom = rank - 1;
            MPI_Isend(&res, 1, MPI_DOUBLE, bottom, 2, MPI_COMM_WORLD, &requests[2]);
            MPI_Irecv(&res, 1, MPI_DOUBLE, bottom, 1, MPI_COMM_WORLD, &requests[3]);
        }

        MPI_Waitall(4, requests, MPI_STATUSES_IGNORE);
    }

    E = getTimeStamp();

    if (rank == 0)
    {
        std::cout << "Solution took : " << E - S << std::endl;
        std::cout << "Overall collected time : " << res << std::endl;
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
