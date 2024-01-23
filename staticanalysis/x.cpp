///  \file
///
///  MAIN_CPP (TEST_BENCHMARK)
///
///  C/C++ implementation of parallel workloads in MPI environment for messages exchange
///	 with (blocking or non-blocking) pairwise and collective operations on Cartesian grid topology
///
///  Prerequisites:
///     1- MPI environment to compile sources
///     2- likwid environment for performance measurement (optional)
///     3- ITAC environment for timeline visualization (optional)
///
///  \Author: Ayesha Afzal <ayesha.afzal@fau.de>
///  \Copyright © 2019 HPC, FAU Erlangen-Nuremberg. All rights reserved.
///

#include <vector>
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <algorithm> //!< e.g., std::all_of
#include <numeric>   //!< e.g., std::accumulate
#include <mpi.h>
#include <unistd.h>
#include <iomanip>
#include <fstream>
#include <thread>

#ifdef USE_HYBRID_OPENMP
#include <omp.h>
#endif

#ifdef USE_LIKWID
#include <likwid.h> //!< time measurements METHOD 1
#else
#include <chrono> //!< time measurements METHOD 2
using namespace std::chrono;
#endif

#include "helper.hpp"       //!< helper functions like printing ND vectors, error check for all MPI call routines, etc.,
#include "timeRankOP.hpp"   //!< time rank output file
#include "configParser.hpp" //!< configuration file parser

#ifdef SINGLE
#define MPI_REAL_TYPE MPI_FLOAT
#define realT float
#ifdef USE_TRIAD
#define triadT float
#endif

#else
#define MPI_REAL_TYPE MPI_DOUBLE
#define realT double
#ifdef USE_TRIAD
#define triadT double
#endif
#endif

#define timeT double

int main(int argc, char *argv[])
{
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                         Initailize environments                                      //
//////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *\brief initialize likwid environment
 */
#ifdef USE_LIKWID
    LIKWID_MARKER_INIT;
#endif

    /**
     *\brief initialize MPI environment
     */
    mpiErrorCheck(MPI_Init(&argc, &argv));
    int rank, nTasks;
    mpiErrorCheck(MPI_Comm_rank(MPI_COMM_WORLD, &rank));   //!< get processors id (rank)
    mpiErrorCheck(MPI_Comm_size(MPI_COMM_WORLD, &nTasks)); //!< get total number of processors (processors size)

/**
 *\brief OPENMP environment
 */
#ifdef USE_HYBRID_OPENMP
    int tid, nthreads;
#pragma omp parallel default(shared) private(tid, nthreads)
    {
        nthreads = omp_get_num_threads();
        tid = omp_get_thread_num();
        std::cout << "Hybrid OpenMP+MPI: Hello from thread " << tid << " out of " << nthreads << " from process " << rank << " out of " << nTasks << std::endl;
    }
    nthreads = 0;
#pragma omp parallel
#pragma omp atomic
    nthreads++;
    std::cout << "Number of Threads counted =" << nthreads << std::endl;
#endif

    /**
     *\brief initialize user interface (configuration input) environment
     */
    userInterface::configParser CFG("config.cfg");

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                     Reading configuration file                                       //
    //////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
     *\brief it parses and print user-defined parameters or args of configuration file (config.cfg).
     */
    bool exists = CFG.getKey("filename"); //!< check if a user-specified file exists
    if (rank == 0)
        std::cout << "\nUser-specified device: " << std::boolalpha << exists << std::endl;
    // const char* filename ="test.bin";

    std::size_t ppn = CFG.getValue<std::size_t>("ppn");    //!< get the value of a user-specified processes count per node
    std::size_t nDims = CFG.getValue<std::size_t>("Dims"); //!< get the value of a user-specified Cartesian grid dimension

    std::vector<int> dims(nDims);        //!< the number of nodes in each direction
    dims[0] = CFG.getValue<int>("dimX"); //!< get the value of a user-specified number of nodes/domains in x dimension
    if (nDims == 2)
        dims[1] = CFG.getValue<int>("dimY"); //!< get the value of a user-specified number of nodes/domains in y dimension
    if (nDims == 3)
    {
        dims[1] = CFG.getValue<int>("dimY"); //!< get the value of a user-specified number of nodes/domains in y dimension
        dims[2] = CFG.getValue<int>("dimZ"); //!< get the value of a user-specified number of nodes/domains in z dimension
    }

    int const nDirs = CFG.getValue<int>("numDirShifts"); //!< get the value of a user-specified number of direction of shifts in each dimension
    int ShiftStep = CFG.getValue<int>("ShiftStep");      //!< get the value of a user-specified step size of shifts in each dimension
    std::vector<int> periods(nDims);                     //!< specify which Cartesian directions are periodic (periods =0 for no periods) or non-periodic (periods =1 for wrap-around)
    periods[0] = CFG.getValue<int>("periodicX");         //!< get the value of a user-specified periodicity in x direction of Cartesian grid dimension
    if (nDims == 2)
        periods[1] = CFG.getValue<int>("periodicY"); //!< get the value of a user-specified periodicity in y direction of Cartesian grid dimension
    if (nDims == 3)
    {
        periods[1] = CFG.getValue<int>("periodicY"); //!< get the value of a user-specified periodicity in y direction of Cartesian grid dimension
        periods[2] = CFG.getValue<int>("periodicZ"); //!< get the value of a user-specified periodicity in z direction Cartesian grid dimension
    }

    if (rank == 0)
    {
        std::cout << "Cartesian grid dimensionality: " << nDims << "D" << std::endl;
        std::cout << "Periodicity of Cartesian directions: ";
        print_1D(periods);
    }

    realT msgSizeBytes = CFG.getValue<realT>("msgSizeBytes"); //!< get the value of a user-specified message size in bytes
    int length = msgSizeBytes / sizeof(realT);
    std::size_t nTimeStep = CFG.getValue<std::size_t>("numTimeStep");                     //!< get the value of a user-specified outer iteration count
    std::size_t collectiveOPTimeStep = CFG.getValue<std::size_t>("collectiveOPTimeStep"); //!< get the value of a user-specified Outer iteration count where collective communication happens
    std::size_t nCppTimeRep = CFG.getValue<std::size_t>("timingRep");                     //!< get the value of a user-specified timing repetitions
    std::size_t vtkStep = CFG.getValue<std::size_t>("vtk_step");

    if (rank == 0)
        std::cout << "Message size: " << length << " doubles or " << length * sizeof(realT) << " bytes" << std::endl;

#ifdef USE_PISOLVER
    const long nSteps = CFG.getValue<realT>("numSteps"); //!< get the value of a user-specified number of steps for pi calculation
    if (rank == 0)
    {
        std::cout << "Number of steps for pi calculation: " << nSteps << std::endl;
        std::cout << "Number of outer loop iterations: " << nTimeStep << std::endl;
    }
#endif

#ifdef USE_TRIAD
    std::size_t arraySize = CFG.getValue<std::size_t>("arraySize");           //!< get the value of a user-specified number of elements per array
    std::size_t arrayAlignment = CFG.getValue<std::size_t>("arrayAlignment"); //!< get the value of a user-specified scalar coefficient value
    std::size_t scalar = CFG.getValue<std::size_t>("scalar");                 //!< get the value of a user-specified array alignment value
    if (rank == 0)
    {
        std::cout << "Total aggregate array size: " << arraySize << std::endl;
        std::cout << "Total aggregate memory per array in MB: " << sizeof(triadT) * ((double)arraySize / 1024.0 / 1024.0) << std::endl;
        std::cout << "Required total aggregate memory in MB: " << (3.0 * sizeof(triadT)) * ((double)arraySize / 1024.0 / 1024.) << std::endl;
    }
#endif

#ifdef USE_IDLEPERIOD
    std::size_t extraIt = CFG.getValue<std::size_t>("extraIt");                         //!< get the value of a user-specified number of extra iterations for a processor
    std::size_t extraTriadIt = CFG.getValue<std::size_t>("extraTRIADIt");               //!< get the value of a user-specified number of extra iterations for a processor
    std::size_t IdlePeriodTimeStep = CFG.getValue<std::size_t>("idlePeriodTimeStep");   //!< get the value of a user-specified time step number for extra iterations
    std::size_t IdlePeriodTimeStep1 = CFG.getValue<std::size_t>("idlePeriodTimeStep1"); //!< get the value of a user-specified time step number for extra iterations
    std::size_t IdlePeriodTimeStep2 = CFG.getValue<std::size_t>("idlePeriodTimeStep2"); //!< get the value of a user-specified time step number for extra iterations
    std::size_t IdlePeriodRank = CFG.getValue<std::size_t>("idlePeriodRank");           //!< get the value of a user-specified number of extra iterations for a processor
#endif

#ifdef USE_EXTRAWORK
    std::size_t extraItTimeStep = CFG.getValue<std::size_t>("extraItTimeStep"); //!< get the value of a user-specified time step number for extra iterations
#endif

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    //               Setup of Cartesian grid topology using virtual topology routines                       //
    //////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
     *\brief virtual topology describes a mapping of MPI processes into a geometric "shape".
     * this MPI function (MPI_Dims_create) creates an optimal and balanced distribution of processes in the Cartesian grid per coordinate direction,
     * depending on the number of processes in the group to be balanced and optional constraints that can be specified by the user.
     * If the total size of the Cartesian grid is smaller than the size of the group of comm, then some processes are returned MPI_COMM_NULL, in analogy to MPI_COMM_SPLIT.
     * \param nTasks the number of nodes in a grid (integer)
     * \param nDims the number of Cartesian dimensions (integer)
     * \param dims the number of nodes in each direction. A value of 0 indicates that MPI_Dims_create should fill in a suitable value. (integer vector of size nDims)
     */

    if (std::any_of(dims.begin(), dims.end(), [](int i)
                    { return i    ==    0; }))
    {
        mpiErrorCheck(MPI_Dims_create(nTasks, nDims, &dims[0]));
    }

    if (rank == 0)
    {
        std::size_t nDDDims = nDims - std::count(dims.begin(), dims.end(), 1);
        std::cout << "Domain decomposition dimensionality: " << nDDDims << "D" << std::endl;

        std::cout << "Cartesian grid topology: ";
        print_1D(dims);

/**
 * \brief debug use only
 *  it is erroneous if it specifies a grid that is larger than the group size. (no. of processes per node must >= 1)
 *	vector sizes of grid dimensionality and periodicity should be same
 */
#ifdef USE_DEBUG
        if (std::accumulate(dims.begin(), dims.end(), 1, std::multiplies<int>()) > nTasks)
            throw std::runtime_error(" error: not enough nodes in original communicator");

        if (nDDDims > nDims)
            throw std::runtime_error(" error: not enough grid dimensionality for decomposition");

        if (dims.size() != periods.size())
            throw std::runtime_error(" error: size mismatch for dims and periods");
#endif
    }

    /**
     * \brief It defines a new communicator to which chosen topology is attached and keeps track of the topology information.
     * It creates a dimX * dimsY * dimsZ Cartesian topology from nTasks processors.
     * \param MPI_COMM_WORLD input communicator (handler)
     * \param nDims the number of Cartesian dimensions (integer)
     * \param dims the number of nodes in each direction (integer vector of size nDims)
     * \param periods grid may be periodic (true) or not (false) in each dimension (logical vector of size nDims)
     * \param reorder ranking may be reordered (true) or not (false) (logical flag)
     * \param comm_cart communicator with new Cartesian topology (handle)
     */

    MPI_Comm comm_cart = MPI_COMM_NULL; //!< new Cartesian communicator
    std::size_t reorder = 1;            //!< rank reorder is allowed (rank in MPI_COMM_WORLD and comm_cart communicators may differ)
    mpiErrorCheck(MPI_Cart_create(MPI_COMM_WORLD, nDims, &dims[0], &periods[0], reorder, &comm_cart));

    /// if the node is inside the Cartesian grid topology
    if (comm_cart != MPI_COMM_NULL)
    {
        /**
         \brief rank of current node in new communicator (rank differ from old communicator, if reorder allowed)
        */
        int gridrank = MPI_PROC_NULL;
        mpiErrorCheck(MPI_Comm_rank(comm_cart, &gridrank));

        /**
         \brief the number of nodes in current new communicator
        */
        int gridNTasks = 0;
        mpiErrorCheck(MPI_Comm_size(comm_cart, &gridNTasks));

        /**
         * \brief Cartesian coodinates of nodes in current new communicator
         * \param comm_cart communicator with Cartesian structure (handle)
         * \param gridrank rank of a process within group of comm (integer)
         * \param nDims length of vector coords in the calling program (integer)
         * \return Cartesian coodinates of specified process/node in current new communicator (integer vector of size nDims)
         */
        std::vector<int> coords(nDims);
        mpiErrorCheck(MPI_Cart_coords(comm_cart, gridrank, nDims, &coords[0]));

/**
 * \brief debug use only
 * grid rank should be in the grid
 * vector sizes of grid dimensionality and coordinates should be same
 * coordinates check for non-periodic directions
 */
#ifdef USE_DEBUG
        if (gridrank == 0)
        {
            if (gridrank < 0 || gridrank > gridNTasks)
                throw std::runtime_error(" error: gridrank is not in the grid");

            if (coords.size() != dims.size())
                throw std::runtime_error(" error: size mismatch for dims and coords");

            bool test = true;
            for (std::size_t i = 0; test && i < coords.size(); ++i)
                test = periods[i] || (coords[i] > -1 && coords[i] < dims[i]);
            if (test == false)
                throw std::runtime_error(" error: coordinates check FAIL for non-periodic directions");
        }
#endif

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //  neighbouring topology for data exchange in max. (2 * nDims) directions using virtual topology routines //
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

        /**
         *\brief the next communication neighbours of the nodes along a certain Cartesian dimension.
         *	Defining exchange between processes in new Cartesian communication world (neighbors shift by +/- one)
         *	x dimension: West  - East  neighbours
         *	y dimension: North - South neighbours
         *	z dimension: In    - Out   neighbours
         *  ring/circular shift (or linear/end-off shift in case of open boundary conditions.
         *  If no periodic BC's, neighbour may not exist􏰉 and MPI_PROC_NULL rank processor will be repoted)
         * \param comm_cart input communicator with Cartesian structure (handle)
         * \param i  coordinate dimension of shift (integer)
         * \param 1/-1 step size [displacement > 0: upwards shift, displacement < 0: downwards shift] (integer)
         * \return srcRank: shifted rank of source process (integer)
         * \return dstRank: shifted rank of destination process (integer)
         */

        // std::vector<std::vector<int>> srcRank (nDirs, std::vector<int>(nDims));	//!< std::vector
        // std::vector<std::vector<int>> dstRank (nDirs, std::vector<int>(nDims));

        // std::array<std::array<int, 3>, 2 > dstRank;								//!< std::array (T,COL,ROW)
        // std::array<std::array<int, 3>, 2 > srcRank;

        int srcRank[nDirs][nDims], dstRank[nDirs][nDims]; //!< C-style arrays
        int negShiftStep = -1 * ShiftStep;
        for (std::size_t i = 0; i < nDims; i++)
        {
            /// for positive displacement in each dimensions
            mpiErrorCheck(MPI_Cart_shift(comm_cart, i, ShiftStep, &srcRank[0][i], &dstRank[0][i]));

            /// for negative displacement in each dimensions
            if (nDirs == 2)
                mpiErrorCheck(MPI_Cart_shift(comm_cart, i, negShiftStep, &srcRank[1][i], &dstRank[1][i]));
        }

        /// for indirect discountinuous neighbour communication in parallel
        /* int srcRank1[nDirs][nDims], dstRank1[nDirs][nDims]; 							//!< C-style arrays
         int negShiftStep1 = -1*(ShiftStep+1); // ShiftStep, ShiftStep+1, ShiftStep+5, ShiftStep+11
         for(std::size_t i = 0; i < nDims; i++ )
         {
             /// for positive displacement in each dimensions
             mpiErrorCheck(MPI_Cart_shift( comm_cart, i,  ShiftStep+1, &srcRank1[0][i], &dstRank1[0][i] )); // ShiftStep, ShiftStep+1, ShiftStep+5, ShiftStep+11

             /// for negative displacement in each dimensions
             if (nDirs ==2) mpiErrorCheck(MPI_Cart_shift( comm_cart, i, negShiftStep1, &srcRank1[1][i], &dstRank1[1][i] ));
         }*/

        /// for indirect continuous neighbour communication in parallel
        /*   int srcRank1[nDirs][nDims], dstRank1[nDirs][nDims];                             //!< C-style arrays
           int srcRank2[nDirs][nDims], dstRank2[nDirs][nDims], srcRank3[nDirs][nDims], dstRank3[nDirs][nDims];                             //!< C-style arrays
           int srcRank4[nDirs][nDims], dstRank4[nDirs][nDims], srcRank5[nDirs][nDims], dstRank5[nDirs][nDims];                             //!< C-style arrays
           int srcRank6[nDirs][nDims], dstRank6[nDirs][nDims], srcRank7[nDirs][nDims], dstRank7[nDirs][nDims];                             //!< C-style arrays
           int srcRank8[nDirs][nDims], dstRank8[nDirs][nDims], srcRank9[nDirs][nDims], dstRank9[nDirs][nDims];                             //!< C-style arrays
           int srcRank10[nDirs][nDims], dstRank10[nDirs][nDims], srcRank11[nDirs][nDims], dstRank11[nDirs][nDims];                             //!< C-style arrays
           int negShiftStep1 = -1*(ShiftStep+1);
           int negShiftStep2 = -1*(ShiftStep+2);
           int negShiftStep3 = -1*(ShiftStep+3);
           int negShiftStep4 = -1*(ShiftStep+4);
           int negShiftStep5 = -1*(ShiftStep+5);
           int negShiftStep6 = -1*(ShiftStep+6);
           int negShiftStep7 = -1*(ShiftStep+7);
           int negShiftStep8 = -1*(ShiftStep+8);
           int negShiftStep9 = -1*(ShiftStep+9);
           int negShiftStep10 = -1*(ShiftStep+10);
           int negShiftStep11 = -1*(ShiftStep+11);
           for(std::size_t i = 0; i < nDims; i++ )
           {
               /// for positive displacement in each dimensions
               mpiErrorCheck(MPI_Cart_shift( comm_cart, i,  ShiftStep+1, &srcRank1[0][i], &dstRank1[0][i] ));
               mpiErrorCheck(MPI_Cart_shift( comm_cart, i,  ShiftStep+2, &srcRank2[0][i], &dstRank2[0][i] ));
               mpiErrorCheck(MPI_Cart_shift( comm_cart, i,  ShiftStep+3, &srcRank3[0][i], &dstRank3[0][i] ));
               mpiErrorCheck(MPI_Cart_shift( comm_cart, i,  ShiftStep+4, &srcRank4[0][i], &dstRank4[0][i] ));
               mpiErrorCheck(MPI_Cart_shift( comm_cart, i,  ShiftStep+5, &srcRank5[0][i], &dstRank5[0][i] ));
               mpiErrorCheck(MPI_Cart_shift( comm_cart, i,  ShiftStep+6, &srcRank6[0][i], &dstRank6[0][i] ));
               mpiErrorCheck(MPI_Cart_shift( comm_cart, i,  ShiftStep+7, &srcRank7[0][i], &dstRank7[0][i] ));
               mpiErrorCheck(MPI_Cart_shift( comm_cart, i,  ShiftStep+8, &srcRank8[0][i], &dstRank8[0][i] ));
               mpiErrorCheck(MPI_Cart_shift( comm_cart, i,  ShiftStep+9, &srcRank9[0][i], &dstRank9[0][i] ));
               mpiErrorCheck(MPI_Cart_shift( comm_cart, i,  ShiftStep+10, &srcRank10[0][i], &dstRank10[0][i] ));
               mpiErrorCheck(MPI_Cart_shift( comm_cart, i,  ShiftStep+11, &srcRank11[0][i], &dstRank11[0][i] ));

               /// for negative displacement in each dimensions
               if (nDirs ==2){
                   mpiErrorCheck(MPI_Cart_shift( comm_cart, i, negShiftStep1, &srcRank1[1][i], &dstRank1[1][i] ));
                   mpiErrorCheck(MPI_Cart_shift( comm_cart, i, negShiftStep2, &srcRank2[1][i], &dstRank2[1][i] ));
                   mpiErrorCheck(MPI_Cart_shift( comm_cart, i, negShiftStep3, &srcRank3[1][i], &dstRank3[1][i] ));
                   mpiErrorCheck(MPI_Cart_shift( comm_cart, i, negShiftStep4, &srcRank4[1][i], &dstRank4[1][i] ));
                   mpiErrorCheck(MPI_Cart_shift( comm_cart, i, negShiftStep5, &srcRank5[1][i], &dstRank5[1][i] ));
                   mpiErrorCheck(MPI_Cart_shift( comm_cart, i, negShiftStep6, &srcRank6[1][i], &dstRank6[1][i] ));
                   mpiErrorCheck(MPI_Cart_shift( comm_cart, i, negShiftStep7, &srcRank7[1][i], &dstRank7[1][i] ));
                   mpiErrorCheck(MPI_Cart_shift( comm_cart, i, negShiftStep8, &srcRank8[1][i], &dstRank8[1][i] ));
                   mpiErrorCheck(MPI_Cart_shift( comm_cart, i, negShiftStep9, &srcRank9[1][i], &dstRank9[1][i] ));
                   mpiErrorCheck(MPI_Cart_shift( comm_cart, i, negShiftStep10, &srcRank10[1][i], &dstRank10[1][i] ));
                   mpiErrorCheck(MPI_Cart_shift( comm_cart, i, negShiftStep11, &srcRank11[1][i], &dstRank11[1][i] ));
               }
           }
                 */
/**
 * \brief print my neighbours
 */
#ifdef USE_ALLPRINT
        // if ( rank == 0 ) {
        /* /// print std::vector
        std::cout<<"source ranks:"<<std::endl;
        print_2D(srcRank);
        std::cout<<"destination ranks:"<<std::endl;
        print_2D(dstRank);*/
        for (int i = 0; i < nDims; i++)
        {
            for (int j = 0; j < nDirs; j++)
            {
                std::cout << "srcRank[" << j << "][" << i << "]=" << srcRank[j][i] << std::endl;
                std::cout << "dstRank[" << j << "][" << i << "]=" << dstRank[j][i] << std::endl;
            }
        }
//}
#endif

        /**
         *\brief initialize user interface (time-rank output) environment
         */
        userInterface::timeRankOP TR(gridNTasks, gridrank);
        userInterface::timeRankOP TR1(gridNTasks, gridrank);
        userInterface::timeRankOP TR2(gridNTasks, gridrank);

/**
 *\brief problem initialization
 */
#ifdef USE_PISOLVER
        realT sum, x;
        const realT stepLength = 1. / nSteps;
        const realT PiExact = 3.1415926535897932; //!< Pi value in math.h
#endif

#ifdef USE_TRIAD
        triadT *A, *B, *C, *D;
        size_t arrayElements = arraySize / gridNTasks;                                             //!< array size of each MPI rank
        size_t memA = posix_memalign((void **)&A, arrayAlignment, arrayElements * sizeof(triadT)); //!< dynamic allocation of the array a
        size_t memB = posix_memalign((void **)&B, arrayAlignment, arrayElements * sizeof(triadT)); //!< dynamic allocation of the array b
        size_t memC = posix_memalign((void **)&C, arrayAlignment, arrayElements * sizeof(triadT)); //!< dynamic allocation of the array c
        size_t memD = posix_memalign((void **)&D, arrayAlignment, arrayElements * sizeof(triadT)); //!< dynamic allocation of the array c

#ifdef USE_DEBUG
        if (memA != 0)
        {
            throw std::runtime_error(" error: allocation of array A FAIL");
            MPI_Abort(comm_cart, 2);
        }
        if (memB != 0)
        {
            throw std::runtime_error(" error: allocation of array B FAIL");
            MPI_Abort(comm_cart, 2);
        }
        if (memC != 0)
        {
            throw std::runtime_error(" error: allocation of array C FAIL");
            MPI_Abort(comm_cart, 2);
        }
        if (memD != 0)
        {
            throw std::runtime_error(" error: allocation of array D FAIL");
            MPI_Abort(comm_cart, 2);
        }
#endif

        /// initialisation of the array a, b and c
        for (std::size_t i = 0; i < arrayElements; i++)
        {
            A[i] = 2.0;
            B[i] = 2.0;
            C[i] = 1.0;
            D[i] = 2.0;
        }
#endif

        /**
         *\brief send and receive buffers for uni/bi-directional displacements in each dimension
         */
        // realT sendBuffer[nDims][nDirs][length], recvBuffer[nDims][nDirs][length];

        // auto sendBuffer = new realT[nDims][2][140000000];
        // auto recvBuffer = new realT[nDims][2][140000000];

        realT ***sendBuffer = new realT **[nDims];
        for (std::size_t i = 0; i < nDims; ++i)
        {
            sendBuffer[i] = new realT *[nDirs];
            for (std::size_t j = 0; j < nDirs; ++j)
                sendBuffer[i][j] = new realT[length];
        }

        realT ***recvBuffer = new realT **[nDims];
        for (std::size_t i = 0; i < nDims; ++i)
        {
            recvBuffer[i] = new realT *[nDirs];
            for (std::size_t j = 0; j < nDirs; ++j)
                recvBuffer[i][j] = new realT[length]();
        }

        for (std::size_t i = 0; i < nDims; ++i)
        {
            for (std::size_t j = 0; j < nDirs; ++j)
            {
                for (std::size_t k = 0; k < length; ++k)
                {
                    sendBuffer[i][j][k] = (double)(i + 1) * (gridrank + 1) + i;
                    recvBuffer[i][j][k] = (double)(i + 1) * (gridrank + 1) + i;
                }
            }
        }

#ifndef USE_LIKWID
        timeT aggregate_time1 = 0.0, aggregate_time2 = 0.0, aggregate_time3 = 0.0;
        timeT aggregate_time4 = 0.0, aggregate_time5 = 0.0;
#endif
        int N0S0counter = 0, N0S1counter = 0, N1S0counter = 0, N1S1counter = 0;
        timeT timestemp = 100000000.0, timestempbase = 3000000000.0, mycomptime = 0.0, mycommtime = 0.0;

#ifdef USE_EXTRAWORK
        const char *filename = "NormalDist3.bin";

        // int totalElements= 2880;
        // MPI_Offset start = totalElements *  gridrank / gridNTasks;
        // MPI_Offset end = totalElements * (gridrank+1) / gridNTasks;
        // int bufsize = end - start;
        // MPI_Offset disp = start * sizeof(int);
        // std::cout << "process " << gridrank << " start " << start << " end " << end << " buffer size " << bufsize <<std::endl;

        mpiErrorCheck(MPI_Barrier(comm_cart));

        MPI_File inFile;
        mpiErrorCheck(MPI_File_open(comm_cart, filename, MPI::MODE_RDONLY, MPI::INFO_NULL, &inFile)); // MPI_COMM_SELF to open binary file by one process per task instead of all tasks at the same time with comm_cart

        MPI_Offset filesizeByte, filesize;
        mpiErrorCheck(MPI_File_get_size(inFile, &filesizeByte)); // in bytes (e.g., 2880 * 4 bytes)
        filesize = filesizeByte / sizeof(int);                   // in number of ints (e.g., 2880 ints)
        int bufsize = filesize / gridNTasks;                     // local number to read (e.g., 30 ints)
        int bufsizeByte = bufsize * sizeof(int);
        if (gridrank == 0)
            std::cout << "Each process read " << filesize << " ints/ " << filesizeByte << " bytes of filesize and " << bufsize << " ints / " << bufsizeByte << " bytes of buffer size " << std::endl;

        int *buf = (int *)malloc(bufsizeByte); // allocate memory
        for (std::size_t i = 0; i < bufsize; i++)
            buf[i] = 123;

        MPI_Offset disp = gridrank * bufsizeByte;
        mpiErrorCheck(MPI_File_set_view(inFile, disp, MPI_INT, MPI_INT, "native", MPI_INFO_NULL));
        MPI_Status status;
        mpiErrorCheck(MPI_File_read(inFile, buf, bufsize, MPI_INT, &status));
        // mpiErrorCheck(MPI_File_read_at(inFile, disp, buf, bufsize, MPI_INT, &status));

        int count;
        mpiErrorCheck(MPI_Get_count(&status, MPI_INT, &count));

        if (count != bufsize) // check if complete buffer has been read
            std::cout << " error: not complete buffer has been READ" << std::endl;

        MPI_File_close(&inFile);

        int *workIterations = (int *)malloc(bufsize * sizeof(int));
        const double divideTime_ns = 0.231256113833509; // time in nano seconds taken by a divide operation 6000000/25945260 (emmy)
        // const double divideTime_ns =  0.231872136958789; //0.017608333540623; // time in nano seconds taken by a divide operation 17763000/75945260 (meggie)

        for (int j = 0; j < gridNTasks; j++)
        {
            if (gridrank == j)
            {
                for (int i = 0; i < bufsize; i++)
                {
                    if (buf[i] != 0)
                        workIterations[i] = buf[i] / divideTime_ns;
                    else
                        workIterations[i] = 0;
#ifdef USE_ALLPRINT
                    std::cout << "Process " << gridrank << " read " << count << " ints and at index " << i << " has value " << buf[i] << " ns and number of iterations " << workIterations[i] << std::endl;
#endif
                }
            }
        }
#endif
        MPI_Request req[48 * nDims];
        MPI_Status stt[48 * nDims];
        mpiErrorCheck(MPI_Barrier(comm_cart));

        /// warm-up phase
        for (std::size_t warmupStep = 0; warmupStep <= 50; warmupStep++)
        {
#ifdef USE_PISOLVER
            sum = 0.0;
            /// each process will calculate a part of sum
            std::size_t tilesize = nSteps / gridNTasks;
#ifdef USE_HYBRID_OPENMP
#pragma omp parallel for
#endif
            for (std::size_t i = gridrank * tilesize; i < gridrank * tilesize + tilesize; i++)
            {
                x = (i + 0.5) * stepLength;
                sum = sum + 4.0 / (1.0 + x * x);
            }
            if (sum < 0)
                std::cout << "sum" << sum << std::endl;
#endif

#ifdef USE_TRIAD
#pragma vector nontemporal
#ifdef USE_HYBRID_OPENMP
#pragma omp parallel for
#endif
            for (std::size_t i = 0; i < arrayElements; i++)
                A[i] = B[i] + scalar * C[i];
            if (A[arrayElements / 2] < 0)
                std::cout << "A" << A << std::endl;
#endif

#ifdef B_COLLECTIVE_REDUCE
            if (timeStep != collectiveOPTimeStep)
            {
#endif

#ifdef B_COLLECTIVE_SENDRECV
                if (timeStep != collectiveOPTimeStep)
                {
#endif

#ifdef B_PAIRWISE
                    for (std::size_t i = 0; i < nDims; i++)
                    {
                        for (std::size_t j = 0; j < nDirs; j++)
                        {
                            mpiErrorCheck(MPI_Sendrecv(&sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank[j][i], j, &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank[j][i], j, comm_cart, MPI_STATUS_IGNORE));
                        }
                    }
#endif

/// CASE 2
#ifdef NB_ISENDIRECV

                    for (std::size_t i = 0; i < nDims; i++)
                    {
                        for (std::size_t j = 0; j < nDirs; j++)
                        {
                            mpiErrorCheck(MPI_Isend(&sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank[j][i], 0, comm_cart, &req[i * 4 + (0 + j * 2)]));
                            mpiErrorCheck(MPI_Irecv(&recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank[j][i], 0, comm_cart, &req[i * 4 + (1 + j * 2)]));
                        }
                    }
                    mpiErrorCheck(MPI_Waitall(2 * nDirs * nDims, &req[0], &stt[0]));
/*for ( std::size_t i = 0; i < nDims; i++ )
{
    for ( std::size_t j = 0; j < nDirs; j++ )
    {
        mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank[j][i], 0, comm_cart, &req[0] ));
        mpiErrorCheck(MPI_Irecv( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank[j][i], 0, comm_cart, &req[1] ));
        mpiErrorCheck(MPI_Waitall( 2, &req[0], &stt[0] ));
    }
}*/
#endif

/// CASE 1
#ifdef NB_ISENDRECV
                    for (std::size_t i = 0; i < nDims; i++)
                    {
                        for (std::size_t j = 0; j < nDirs; j++)
                        {
                            mpiErrorCheck(MPI_Isend(&sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank[j][i], 0, comm_cart, &req[0]));
                            mpiErrorCheck(MPI_Recv(&recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank[j][i], 0, comm_cart, MPI_STATUS_IGNORE));
                            mpiErrorCheck(MPI_Barrier(comm_cart));
                        }
                    }
#endif
                } //!< warmupStep
#ifndef USE_LIKWID
                high_resolution_clock::time_point maint1, maint2, maint3, maint4;
#endif

                mpiErrorCheck(MPI_Barrier(MPI_COMM_WORLD));
                mpiErrorCheck(MPI_Barrier(comm_cart));
                if (gridrank == 0)
                    std::cout << "\nStart" << std::endl;
                mpiErrorCheck(MPI_Barrier(comm_cart));

                /**
                 *\brief outer main time step loop
                 */
                for (std::size_t timeStep = 0; timeStep <= nTimeStep; timeStep++)
                {
#ifdef USE_LIKWID
                    LIKWID_MARKER_START("Case:Computation");
#else
            for (std::size_t i = 0; i < gridNTasks; ++i)
            {
                if (gridrank == i)
                    maint1 = high_resolution_clock::now();
            }
#ifdef TIME_REP
            for (std::size_t timing_repetitions = 0; timing_repetitions < nCppTimeRep; ++timing_repetitions)
            {
#endif
#endif

#ifdef USE_PISOLVER
                    //////////////////////////////////////////////////////////////////////////////////////////////////////////
                    //                                  MPI parallelized Pi solver                                          //
                    //////////////////////////////////////////////////////////////////////////////////////////////////////////
                    /**
                     *\brief Computation for finding PI in MPI environment and measure the duration of partial sum
                     * The algorithm is based on integral representation of PI.
                     * If f(x)=4/(1+x^2), then PI is the intergral of f(x) from 0 to 1.
                     * The Basic Idea: Tan(Pi/4) = 1.0. (i.e., the Legs of a right triangle with 2, 45 degree angles are equal)
                     * This Means that arctan(1.0) = Pi/4 SO - We need to compute arctan(1.0)
                     * Arctan(x) has a known derivative of x'/(1+x*x) = 1.0/(1+x*x)
                     * By integrating this, we can approximate Arctan(1.0)
                     * We'll perform a numerical integration to get an approximation of Pi.
                     */
                    if (gridrank == 0 && timeStep == 0)
                        std::cout << "\nComputation: compute-bound pi solver" << std::endl;

                    sum = 0.0;
                    /// each process will calculate a part of sum
                    std::size_t tilesize = nSteps / gridNTasks;
#ifdef USE_HYBRID_OPENMP
#pragma omp parallel for
#endif
                    for (std::size_t i = gridrank * tilesize; i < gridrank * tilesize + tilesize; i++)
                    {
                        x = (i + 0.5) * stepLength;
                        sum = sum + 4.0 / (1.0 + x * x);
                    }
                    if (sum < 0)
                        std::cout << "sum" << sum << std::endl;

/// inject idle period with an extra work for a processor for extraItTimeStep iteration and IdlePeriodRank
#ifdef USE_IDLEPERIOD
                    if (gridrank == 0 && timeStep == 0)
                        std::cout << "taking idle period values from configuration setting" << std::endl;
                    sum = 0.0;
                    // for ( std::size_t j = 5; j < 10; j++ ) {
                    // if ((timeStep==IdlePeriodTimeStep) && (gridrank % ppn == IdlePeriodRank)) { // for system-induced disturbances on ranks numbers= IdlePeriodRank+ 12 * n with n = 0, 1, ..., (ppn - IdlePeriodRank -1)
                    if ((timeStep == IdlePeriodTimeStep) && (gridrank % gridNTasks == IdlePeriodRank))
                    { // for application-induced disturbances
#ifdef USE_HYBRID_OPENMP
#pragma omp parallel for
#endif
                        for (std::size_t i = 0; i < extraIt; i++)
                        {
                            x = 0.5 * stepLength;
                            sum = sum + 4.0 / (1.0 + x * x);
                        }
                        // std::cout << "Rank " << gridrank << " performs " << extraIt << " number of extra iteration for time step iteration " << timeStep << std::endl;
                        if (sum < 0)
                            std::cout << "sum" << sum << std::endl;
                    }
                    //}
                    /* sum = 0.0;
                         if ((timeStep==IdlePeriodTimeStep) && (gridrank % gridNTasks == IdlePeriodRank+10)) { // for application-induced disturbances
                             for ( std::size_t i = 0; i < extraIt/2; i++ ) {
                                 x = 0.5*stepLength;
                                 sum = sum + 4.0 / ( 1.0 + x * x);
                             }
                             if (sum < 0) std::cout << "sum" << sum <<std::endl;
                         }
                                 sum = 0.0;*/
#endif

#ifdef USE_EXTRAWORK
                    /**
                     *\brief make a check that file exist. If file is not there then takes default values from configuration setting
                     */
                    if (std::fstream{filename})
                    {
                        if (gridrank == 0 && timeStep == 0)
                            std::cout << "taking fine-grained delay values from binary file" << std::endl;
                        /// an extra work of number of workIterations for all processores and timestep greater than and equal to extraItTimeStep iteration
                        sum = 0.0;
                        const realT stepLength1 = 1. / 44245260;
                        for (std::size_t i = 0; i < gridNTasks; ++i)
                        {
                            if ((timeStep >= extraItTimeStep) && (timeStep < extraItTimeStep + bufsize) && (gridrank == i))
                            {
                                for (std::size_t j = 0; j < workIterations[timeStep - extraItTimeStep]; j++)
                                {
                                    // if ((timeStep<bufsize) && (gridrank == i)) {
                                    // for ( std::size_t j = 0; j < workIterations[timeStep]; j++ ) {
                                    x = 0.5 * stepLength1;
                                    sum = sum + 4.0 / (1.0 + x * x);
                                }
                                // std::cout << "Process " << gridrank << " delay extra " << workIterations[timeStep-extraItTimeStep] << " iterations/ " << buf[timeStep-extraItTimeStep] << " ns for time step " << timeStep << std::endl;
                                if (sum < 0)
                                    std::cout << "sum" << sum << std::endl;
                                // std::this_thread::sleep_for(std::chrono::nanoseconds((int&)buf));
                            }
                        }
                    }

#endif
#endif

#ifdef USE_TRIAD
                    //////////////////////////////////////////////////////////////////////////////////////////////////////////
                    //                                  Stream Triad Benchmark  	                                        //
                    //////////////////////////////////////////////////////////////////////////////////////////////////////////
                    /**
                     *\brief a memory-bound operation used as a standard for measuring the main memory bandwidth in High Performance Computers
                     */
                    if (gridrank == 0 && timeStep == 0)
                        std::cout << "\nComputation: memory-bound triad benchmark" << std::endl;

#pragma vector nontemporal
#ifdef USE_HYBRID_OPENMP
#pragma omp parallel for
#endif
                    for (std::size_t i = 0; i < arrayElements; i++)
                        //				A[i] = B[i]+scalar*C[i];
                        A[i] = B[i] + cos(C[i] / D[i]);
                    if (A[arrayElements / 2] < 0)
                        std::cout << "A" << A << std::endl;

#ifdef USE_IDLEPERIOD
                    if (gridrank == 0 && timeStep == 0)
                        std::cout << "taking idle period values from configuration setting" << std::endl;
                    if ((timeStep == IdlePeriodTimeStep) && (gridrank % gridNTasks == IdlePeriodRank))
                    {
                        for (std::size_t j = 0; j < extraTriadIt; j++)
                        {
#pragma vector nontemporal
#ifdef USE_HYBRID_OPENMP
#pragma omp parallel for
#endif
                            for (std::size_t i = 0; i < arrayElements; i++)
                                A[i] = B[i] + scalar * C[i];
                            // std::cout << "Rank " << gridrank << " performs " << extraIt << " number of extra iteration for time step iteration " << extraItTimeStep << std::endl;
                            if (A[arrayElements / 2] < 0)
                                std::cout << "A" << A << std::endl;
                        }
                    }

                    // non-resource utilising injection
                    /*if ((timeStep==80) && (gridrank % gridNTasks == 15)) {
                    double sum =0;
                        for (double j=0; j<150000000; j+=1.0)	{
                        sum += 4.0 / j;
                        }
                        if (sum < 0) std::cout << "sum" << sum <<std::endl;
                    }*/
                    /*           if ((timeStep==IdlePeriodTimeStep1) && (gridrank % gridNTasks == IdlePeriodRank)) {
                                for (std::size_t j=0; j<extraTriadIt; j++)	{
                                #pragma vector nontemporal
                                #ifdef USE_HYBRID_OPENMP
                                #pragma omp parallel for
                                #endif
                                    for (std::size_t i=0; i<arrayElements; i++)
                                        A[i] = B[i]+scalar*C[i];
                                    if (A[arrayElements/2] < 0) std::cout << "A" << A <<std::endl;
                                  }
                                }
                                if ((timeStep==IdlePeriodTimeStep2) && (gridrank % gridNTasks == IdlePeriodRank)) {
                                for (std::size_t j=0; j<extraTriadIt; j++)	{
                                #pragma vector nontemporal
                                #ifdef USE_HYBRID_OPENMP
                                #pragma omp parallel for
                                #endif
                                    for (std::size_t i=0; i<arrayElements; i++)
                                        A[i] = B[i]+scalar*C[i];
                                    if (A[arrayElements/2] < 0) std::cout << "A" << A <<std::endl;
                                  }
                                }*/
#endif

#ifdef USE_EXTRAWORK
                    /**
                     *\brief make a check that file exist. If file is not there then takes default values from configuration setting
                     */
                    if (std::fstream{filename})
                    {
                        if (gridrank == 0 && timeStep == 0)
                            std::cout << "taking fine-grained delay values from binary file" << std::endl;
                        /// an extra work of number of workIterations for all processores and timestep greater than and equal to extraItTimeStep iteration
                        realT sum, x;
                        const realT stepLength = 1. / 44245260;
                        sum = 0.0;
                        // for ( std::size_t j = 0; j < 1100; j=j+80 ) {
                        std::size_t extraItTimeStep = extraItTimeStep1; // +j;
                        for (std::size_t i = 0; i < gridNTasks; ++i)
                        {
                            if ((timeStep >= extraItTimeStep) && (timeStep < extraItTimeStep + bufsize) && (gridrank == i))
                            {
                                for (std::size_t j = 0; j < workIterations[timeStep - extraItTimeStep]; j++)
                                {
                                    // if ((timeStep<bufsize) && (gridrank == i)) {
                                    // for ( std::size_t j = 0; j < workIterations[timeStep]; j++ ) {
                                    x = 0.5 * stepLength;
                                    sum = sum + 4.0 / (1.0 + x * x);
                                }
                                // std::cout << "Process " << gridrank << " delay extra " << workIterations[timeStep-extraItTimeStep] << " iterations/ " << buf[timeStep-extraItTimeStep] << " ns for time step " << timeStep << std::endl;
                                if (sum < 0)
                                    std::cout << "sum" << sum << std::endl;
                                // std::this_thread::sleep_for(std::chrono::nanoseconds((int&)buf));
                            }
                            //}
                        }
                    }
#endif
#endif

// if (timeStep==0) mpiErrorCheck(MPI_Barrier( comm_cart ));
#ifdef USE_LIKWID
                    LIKWID_MARKER_STOP("Case:Computation");
#else
#ifdef TIME_REP
            }
#endif
            for (std::size_t i = 0; i < gridNTasks; ++i)
            {
                if (gridrank == i)
                {
                    maint2 = high_resolution_clock::now();
                    // duration<double> CompDuration = duration_cast<duration<double>>(maint2 - maint1);
                    timeT CompDuration = duration_cast<nanoseconds>(maint2 - maint1).count();
#ifdef TIME_REP
                    CompDuration /= static_cast<double>(nCppTimeRep);
#endif
                    // aggregate_time1 += CompDuration.count();
                    // aggregate_time1 += CompDuration;
                    aggregate_time1 = aggregate_time3 + CompDuration;
                }
            }

            /* mpiErrorCheck(MPI_Barrier( comm_cart ));
             for (std::size_t i=0 ; i < gridNTasks ; ++i){
                 if (gridrank == i)
                     TR.add_loclop(i, 0, CompDuration.count(), 0);
                 mpiErrorCheck(MPI_Barrier( comm_cart ));
             }*/

            /// adding time rank information of computation phase to output file
            // if (timeStep == 0) TR.add_loclop(gridrank, 0, CompDuration/1000000000, 0);
            // else
            TR.add_loclop(gridrank, aggregate_time3 / 1000000000, aggregate_time1 / 1000000000, 0);
#endif

#ifdef B_COLLECTIVE_REDUCE
                    if (timeStep != collectiveOPTimeStep)
                    {
#endif

#ifdef B_COLLECTIVE_SENDRECV
                        if (timeStep != collectiveOPTimeStep)
                        {
#endif

// mpiErrorCheck(MPI_Barrier( comm_cart ));
#ifdef USE_LIKWID
                            LIKWID_MARKER_START("Case:PairwiseSendrecv");
#else
            for (std::size_t i = 0; i < gridNTasks; ++i)
            {
                if (gridrank == i)
                    maint3 = high_resolution_clock::now();
            }
#ifdef TIME_REP
            for (std::size_t timing_repetitions = 0; timing_repetitions < nCppTimeRep; ++timing_repetitions)
            {
#endif
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// CASE3: Data exchange with blocking sendrecv messages b/w max. 2N neighbours (ND domain decompisition)//
//////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Blocking sends and receives of a message. Both send and receive are disjoint but are using the same communicator.
 * MPI_SENDRECV operation is likely to be used along a coordinate direction to perform a shift of data.
 * \param sendBuffer initial address of send buffer
 * \param length number of elements in send buffer (integer)
 * \param MPI_REAL_TYPE type of elements in send buffer (handle)
 * \param dstRank rank of destination process (integer)
 * \param 0 send tag (integer)
 * \param length number of elements in receive buffer (integer)
 * \param MPI_REAL_TYPE type of elements in receive buffer (handle)
 * \param srcRank rank of source process (integer)
 * \param 0 receive tag (integer)
 * \param comm_cart new communicator (handle)
 * \return recvBuffer initial address of receive buffer
 * \return MPI_STATUS_IGNORE status object (Status). This refers to the receive operation.
 */
#ifdef B_PAIRWISE

                            if (gridrank == 0 && timeStep == 0)
                                std::cout << "\nCommunication Pattern: Pairwise blocking send and receive operations" << std::endl;

                            for (std::size_t i = 0; i < nDims; i++)
                            {
                                for (std::size_t j = 0; j < nDirs; j++)
                                {
                                    mpiErrorCheck(MPI_Sendrecv(&sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank[j][i], j, &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank[j][i], j, comm_cart, MPI_STATUS_IGNORE));
                                }
                            }

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CASE 1 & 2: Data exchange with non-blocking sendrecv messages b/w max. 2N neighbours (ND domain decompisition) //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// CASE 2
#ifdef NB_ISENDIRECV

                            if (gridrank == 0 && timeStep == 0)
                                std::cout << "\nCommunication Pattern: Pairwise non-blocking send and receive operations" << std::endl;

                            /// direct communication without delay
                            for (std::size_t i = 0; i < nDims; i++)
                            {
                                for (std::size_t j = 0; j < nDirs; j++)
                                {
                                    mpiErrorCheck(MPI_Isend(&sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank[j][i], 0, comm_cart, &req[i * 4 + (0 + j * 2)]));
                                    mpiErrorCheck(MPI_Irecv(&recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank[j][i], 0, comm_cart, &req[i * 4 + (1 + j * 2)]));
                                }
                            }
                            mpiErrorCheck(MPI_Waitall(2 * nDirs * nDims, &req[0], &stt[0]));

                            /// direct communication with delay
                            /*for ( std::size_t i = 0; i < nDims; i++ )
                            {
                                for ( std::size_t j = 0; j < nDirs; j++ )
                                {
                                    mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank[j][i], 0, comm_cart, &req[0] ));
                                    mpiErrorCheck(MPI_Irecv( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank[j][i], 0, comm_cart, &req[1] ));
                                    mpiErrorCheck(MPI_Waitall( 2, &req[0], &stt[0] ));
                                }
                            }*/

                            /// version 1: multi-neighbour discountinuos communication with delay (direct+indirect in parallel)
                            /*   for ( std::size_t i = 0; i < nDims; i++ )
                                   {
                                       for ( std::size_t j = 0; j < nDirs; j++ )
                                       {
                                           mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank[j][i], 0, comm_cart, &req[i*4+(0+j*2)] ));
                                           mpiErrorCheck(MPI_Irecv( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank[j][i], 0, comm_cart, &req[i*4+(1+j*2)] ));
                                       }
                               }
                               mpiErrorCheck(MPI_Waitall( 2*nDirs* nDims, &req[0], &stt[0] ));
                               for ( std::size_t i = 0; i < nDims; i++ )
                               {
                                   for ( std::size_t j = 0; j < nDirs; j++ )
                                   {
                                       mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank1[j][i], 0, comm_cart, &req[i*4+(0+j*2)] ));
                                       mpiErrorCheck(MPI_Irecv( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank1[j][i], 0, comm_cart, &req[i*4+(1+j*2)] ));
                                   }
                               }
                               mpiErrorCheck(MPI_Waitall( 2*nDirs* nDims, &req[0], &stt[0] ));*/

                            /// version 1: multi-neighbour countinuos communication with delay (direct+indirect in parallel)
                            /* for ( std::size_t i = 0; i < nDims; i++ )
                                    {
                                        for ( std::size_t j = 0; j < nDirs; j++ )
                                        {
                                            mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank[j][i], 0, comm_cart, &req[i*4+(0+j*2)] ));
                                            mpiErrorCheck(MPI_Irecv( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank[j][i], 0, comm_cart, &req[i*4+(1+j*2)] ));
                                        }
                                }
                                mpiErrorCheck(MPI_Waitall( 2*nDirs* nDims, &req[0], &stt[0] ));
                                for ( std::size_t i = 0; i < nDims; i++ )
                                {
                                    for ( std::size_t j = 0; j < nDirs; j++ )
                                    {
                                        mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank1[j][i], 0, comm_cart, &req[i*4+(0+j*2)] ));
                                        mpiErrorCheck(MPI_Irecv( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank1[j][i], 0, comm_cart, &req[i*4+(1+j*2)] ));
                                    }
                                }
                                mpiErrorCheck(MPI_Waitall( 2*nDirs* nDims, &req[0], &stt[0] ));

                             for ( std::size_t i = 0; i < nDims; i++ )
                                 {
                                     for ( std::size_t j = 0; j < nDirs; j++ )
                                     {
                                         mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank2[j][i], 0, comm_cart, &req[i*4+(0+j*2)] ));
                                         mpiErrorCheck(MPI_Irecv( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank2[j][i], 0, comm_cart, &req[i*4+(1+j*2)] ));
                                     }
                             }
                             mpiErrorCheck(MPI_Waitall( 2*nDirs* nDims, &req[0], &stt[0] ));
                             for ( std::size_t i = 0; i < nDims; i++ )
                             {
                                 for ( std::size_t j = 0; j < nDirs; j++ )
                                 {
                                     mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank3[j][i], 0, comm_cart, &req[i*4+(0+j*2)] ));
                                     mpiErrorCheck(MPI_Irecv( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank3[j][i], 0, comm_cart, &req[i*4+(1+j*2)] ));
                                 }
                             }
                             mpiErrorCheck(MPI_Waitall( 2*nDirs* nDims, &req[0], &stt[0] ));

                             for ( std::size_t i = 0; i < nDims; i++ )
                                 {
                                     for ( std::size_t j = 0; j < nDirs; j++ )
                                     {
                                         mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank4[j][i], 0, comm_cart, &req[i*4+(0+j*2)] ));
                                         mpiErrorCheck(MPI_Irecv( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank4[j][i], 0, comm_cart, &req[i*4+(1+j*2)] ));
                                     }
                             }
                             mpiErrorCheck(MPI_Waitall( 2*nDirs* nDims, &req[0], &stt[0] ));
                             for ( std::size_t i = 0; i < nDims; i++ )
                             {
                                 for ( std::size_t j = 0; j < nDirs; j++ )
                                 {
                                     mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank5[j][i], 0, comm_cart, &req[i*4+(0+j*2)] ));
                                     mpiErrorCheck(MPI_Irecv( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank5[j][i], 0, comm_cart, &req[i*4+(1+j*2)] ));
                                 }
                             }
                             mpiErrorCheck(MPI_Waitall( 2*nDirs* nDims, &req[0], &stt[0] ));

                             for ( std::size_t i = 0; i < nDims; i++ )
                                 {
                                     for ( std::size_t j = 0; j < nDirs; j++ )
                                     {
                                         mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank6[j][i], 0, comm_cart, &req[i*4+(0+j*2)] ));
                                         mpiErrorCheck(MPI_Irecv( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank6[j][i], 0, comm_cart, &req[i*4+(1+j*2)] ));
                                     }
                             }
                             mpiErrorCheck(MPI_Waitall( 2*nDirs* nDims, &req[0], &stt[0] ));
                             for ( std::size_t i = 0; i < nDims; i++ )
                             {
                                 for ( std::size_t j = 0; j < nDirs; j++ )
                                 {
                                     mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank7[j][i], 0, comm_cart, &req[i*4+(0+j*2)] ));
                                     mpiErrorCheck(MPI_Irecv( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank7[j][i], 0, comm_cart, &req[i*4+(1+j*2)] ));
                                 }
                             }
                             mpiErrorCheck(MPI_Waitall( 2*nDirs* nDims, &req[0], &stt[0] ));

                             for ( std::size_t i = 0; i < nDims; i++ )
                                 {
                                     for ( std::size_t j = 0; j < nDirs; j++ )
                                     {
                                         mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank8[j][i], 0, comm_cart, &req[i*4+(0+j*2)] ));
                                         mpiErrorCheck(MPI_Irecv( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank8[j][i], 0, comm_cart, &req[i*4+(1+j*2)] ));
                                     }
                             }
                             mpiErrorCheck(MPI_Waitall( 2*nDirs* nDims, &req[0], &stt[0] ));
                             for ( std::size_t i = 0; i < nDims; i++ )
                             {
                                 for ( std::size_t j = 0; j < nDirs; j++ )
                                 {
                                     mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank9[j][i], 0, comm_cart, &req[i*4+(0+j*2)] ));
                                     mpiErrorCheck(MPI_Irecv( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank9[j][i], 0, comm_cart, &req[i*4+(1+j*2)] ));
                                 }
                             }
                             mpiErrorCheck(MPI_Waitall( 2*nDirs* nDims, &req[0], &stt[0] ));

                             for ( std::size_t i = 0; i < nDims; i++ )
                                 {
                                     for ( std::size_t j = 0; j < nDirs; j++ )
                                     {
                                         mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank10[j][i], 0, comm_cart, &req[i*4+(0+j*2)] ));
                                         mpiErrorCheck(MPI_Irecv( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank10[j][i], 0, comm_cart, &req[i*4+(1+j*2)] ));
                                     }
                             }
                             mpiErrorCheck(MPI_Waitall( 2*nDirs* nDims, &req[0], &stt[0] ));
                             for ( std::size_t i = 0; i < nDims; i++ )
                             {
                                 for ( std::size_t j = 0; j < nDirs; j++ )
                                 {
                                     mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank11[j][i], 0, comm_cart, &req[i*4+(0+j*2)] ));
                                     mpiErrorCheck(MPI_Irecv( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank11[j][i], 0, comm_cart, &req[i*4+(1+j*2)] ));
                                 }
                             }
                             mpiErrorCheck(MPI_Waitall( 2*nDirs* nDims, &req[0], &stt[0] ));*/

                            /// version 2: multi-neighbour discountinuos communication with delay
                            /*for ( std::size_t i = 0; i < nDims; i++ )
                             {
                                 for ( std::size_t j = 0; j < nDirs; j++ )
                                 {
                                     mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank[j][i], 0, comm_cart, &req[0] ));
                                     mpiErrorCheck(MPI_Irecv( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank[j][i], 0, comm_cart, &req[1] ));

                                     mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank1[j][i], 0, comm_cart, &req[2] ));
                                     mpiErrorCheck(MPI_Irecv( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank1[j][i], 0, comm_cart, &req[3] ));
                                     mpiErrorCheck(MPI_Waitall( 4, &req[0], &stt[0] ));
                                 }
                             }*/

                            /// version 2: multi-neighbour countinuos communication with delay
                            /*for ( std::size_t i = 0; i < nDims; i++ )
                             {
                                 for ( std::size_t j = 0; j < nDirs; j++ )
                                 {
                                     mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank[j][i], 0, comm_cart, &req[0] ));
                                     mpiErrorCheck(MPI_Irecv( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank[j][i], 0, comm_cart, &req[1] ));

                                     mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank1[j][i], 0, comm_cart, &req[2] ));
                                     mpiErrorCheck(MPI_Irecv( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank1[j][i], 0, comm_cart, &req[3] ));

                                     mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank2[j][i], 0, comm_cart, &req[4] ));
                                     mpiErrorCheck(MPI_Irecv( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank2[j][i], 0, comm_cart, &req[5] ));

                                     mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank3[j][i], 0, comm_cart, &req[6] ));
                                     mpiErrorCheck(MPI_Irecv( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank3[j][i], 0, comm_cart, &req[7] ));

                                     mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank4[j][i], 0, comm_cart, &req[8] ));
                                     mpiErrorCheck(MPI_Irecv( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank4[j][i], 0, comm_cart, &req[9] ));

                                     mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank5[j][i], 0, comm_cart, &req[10] ));
                                     mpiErrorCheck(MPI_Irecv( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank5[j][i], 0, comm_cart, &req[11] ));

                                     mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank6[j][i], 0, comm_cart, &req[12] ));
                                     mpiErrorCheck(MPI_Irecv( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank6[j][i], 0, comm_cart, &req[13] ));

                                     mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank7[j][i], 0, comm_cart, &req[14] ));
                                     mpiErrorCheck(MPI_Irecv( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank7[j][i], 0, comm_cart, &req[15] ));

                                     mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank8[j][i], 0, comm_cart, &req[16] ));
                                     mpiErrorCheck(MPI_Irecv( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank8[j][i], 0, comm_cart, &req[17] ));

                                     mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank9[j][i], 0, comm_cart, &req[18] ));
                                     mpiErrorCheck(MPI_Irecv( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank9[j][i], 0, comm_cart, &req[19] ));

                                     mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank10[j][i], 0, comm_cart, &req[20] ));
                                     mpiErrorCheck(MPI_Irecv( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank10[j][i], 0, comm_cart, &req[21] ));

                                     mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank11[j][i], 0, comm_cart, &req[22] ));
                                     mpiErrorCheck(MPI_Irecv( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank11[j][i], 0, comm_cart, &req[23] ));

                                     mpiErrorCheck(MPI_Waitall( 24, &req[0], &stt[0] ));
                                 }
                             }*/

                            /// version 3: multi-neighbour discountinuos communication without delay
                            /* for ( std::size_t i = 0; i < nDims; i++ )
                             {
                                     mpiErrorCheck(MPI_Isend( &sendBuffer[i][0][0], length, MPI_REAL_TYPE, dstRank[0][i], 0, comm_cart, &req[4 * (i * 2)] ));
                                     mpiErrorCheck(MPI_Irecv( &recvBuffer[i][0][0], length, MPI_REAL_TYPE, srcRank[0][i], 0, comm_cart, &req[4 * (i * 2) + 1] ));

                                     mpiErrorCheck(MPI_Isend( &sendBuffer[i][0][0], length, MPI_REAL_TYPE, dstRank1[0][i], 0, comm_cart, &req[4 * (i * 2 + 1)] ));
                                     mpiErrorCheck(MPI_Irecv( &recvBuffer[i][0][0], length, MPI_REAL_TYPE, srcRank1[0][i], 0, comm_cart, &req[4 * (i * 2 + 1) + 1] ));

                                     mpiErrorCheck(MPI_Isend( &sendBuffer[i][1][0], length, MPI_REAL_TYPE, dstRank[1][i], 0, comm_cart, &req[4 * (i * 2 ) + 2] ));
                                     mpiErrorCheck(MPI_Irecv( &recvBuffer[i][1][0], length, MPI_REAL_TYPE, srcRank[1][i], 0, comm_cart, &req[4 * (i * 2 ) + 3] ));

                                     mpiErrorCheck(MPI_Isend( &sendBuffer[i][1][0], length, MPI_REAL_TYPE, dstRank1[1][i], 0, comm_cart, &req[4 * (i * 2 + 1) + 2] ));
                                     mpiErrorCheck(MPI_Irecv( &recvBuffer[i][1][0], length, MPI_REAL_TYPE, srcRank1[1][i], 0, comm_cart, &req[4 * (i * 2 + 1) + 3] ));
                             }
                             mpiErrorCheck(MPI_Waitall( 8* nDims, &req[0], &stt[0] ));*/

                            /// version 3: multi-neighbour countinuos communication without delay
                            /*for ( std::size_t i = 0; i < nDims; i++ )
                            {
                                    mpiErrorCheck(MPI_Isend( &sendBuffer[i][0][0], length, MPI_REAL_TYPE, dstRank[0][i], 0, comm_cart, &req[4 * (i * 2)] ));
                                    mpiErrorCheck(MPI_Irecv( &recvBuffer[i][0][0], length, MPI_REAL_TYPE, srcRank[0][i], 0, comm_cart, &req[4 * (i * 2) + 1] ));

                                    mpiErrorCheck(MPI_Isend( &sendBuffer[i][0][0], length, MPI_REAL_TYPE, dstRank1[0][i], 0, comm_cart, &req[4 * (i * 2 + 1)] ));
                                    mpiErrorCheck(MPI_Irecv( &recvBuffer[i][0][0], length, MPI_REAL_TYPE, srcRank1[0][i], 0, comm_cart, &req[4 * (i * 2 + 1) + 1] ));

                                    mpiErrorCheck(MPI_Isend( &sendBuffer[i][0][0], length, MPI_REAL_TYPE, dstRank1[0][i], 0, comm_cart, &req[4 * (i * 2 + 2)] ));
                                    mpiErrorCheck(MPI_Irecv( &recvBuffer[i][0][0], length, MPI_REAL_TYPE, srcRank1[0][i], 0, comm_cart, &req[4 * (i * 2 + 2) + 1] ));

                                    mpiErrorCheck(MPI_Isend( &sendBuffer[i][0][0], length, MPI_REAL_TYPE, dstRank1[0][i], 0, comm_cart, &req[4 * (i * 2 + 3)] ));
                                    mpiErrorCheck(MPI_Irecv( &recvBuffer[i][0][0], length, MPI_REAL_TYPE, srcRank1[0][i], 0, comm_cart, &req[4 * (i * 2 + 3) + 1] ));

                                    mpiErrorCheck(MPI_Isend( &sendBuffer[i][0][0], length, MPI_REAL_TYPE, dstRank1[0][i], 0, comm_cart, &req[4 * (i * 2 + 4)] ));
                                    mpiErrorCheck(MPI_Irecv( &recvBuffer[i][0][0], length, MPI_REAL_TYPE, srcRank1[0][i], 0, comm_cart, &req[4 * (i * 2 + 4) + 1] ));

                                    mpiErrorCheck(MPI_Isend( &sendBuffer[i][0][0], length, MPI_REAL_TYPE, dstRank1[0][i], 0, comm_cart, &req[4 * (i * 2 + 5)] ));
                                    mpiErrorCheck(MPI_Irecv( &recvBuffer[i][0][0], length, MPI_REAL_TYPE, srcRank1[0][i], 0, comm_cart, &req[4 * (i * 2 + 5) + 1] ));

                                    mpiErrorCheck(MPI_Isend( &sendBuffer[i][0][0], length, MPI_REAL_TYPE, dstRank1[0][i], 0, comm_cart, &req[4 * (i * 2 + 6)] ));
                                    mpiErrorCheck(MPI_Irecv( &recvBuffer[i][0][0], length, MPI_REAL_TYPE, srcRank1[0][i], 0, comm_cart, &req[4 * (i * 2 + 6) + 1] ));

                                    mpiErrorCheck(MPI_Isend( &sendBuffer[i][0][0], length, MPI_REAL_TYPE, dstRank1[0][i], 0, comm_cart, &req[4 * (i * 2 + 7)] ));
                                    mpiErrorCheck(MPI_Irecv( &recvBuffer[i][0][0], length, MPI_REAL_TYPE, srcRank1[0][i], 0, comm_cart, &req[4 * (i * 2 + 7) + 1] ));

                                    mpiErrorCheck(MPI_Isend( &sendBuffer[i][0][0], length, MPI_REAL_TYPE, dstRank1[0][i], 0, comm_cart, &req[4 * (i * 2 + 8)] ));
                                    mpiErrorCheck(MPI_Irecv( &recvBuffer[i][0][0], length, MPI_REAL_TYPE, srcRank1[0][i], 0, comm_cart, &req[4 * (i * 2 + 8) + 1] ));

                                    mpiErrorCheck(MPI_Isend( &sendBuffer[i][0][0], length, MPI_REAL_TYPE, dstRank1[0][i], 0, comm_cart, &req[4 * (i * 2 + 9)] ));
                                    mpiErrorCheck(MPI_Irecv( &recvBuffer[i][0][0], length, MPI_REAL_TYPE, srcRank1[0][i], 0, comm_cart, &req[4 * (i * 2 + 9) + 1] ));

                                    mpiErrorCheck(MPI_Isend( &sendBuffer[i][0][0], length, MPI_REAL_TYPE, dstRank1[0][i], 0, comm_cart, &req[4 * (i * 2 + 10)] ));
                                    mpiErrorCheck(MPI_Irecv( &recvBuffer[i][0][0], length, MPI_REAL_TYPE, srcRank1[0][i], 0, comm_cart, &req[4 * (i * 2 + 10) + 1] ));

                                    mpiErrorCheck(MPI_Isend( &sendBuffer[i][0][0], length, MPI_REAL_TYPE, dstRank1[0][i], 0, comm_cart, &req[4 * (i * 2 + 11)] ));
                                    mpiErrorCheck(MPI_Irecv( &recvBuffer[i][0][0], length, MPI_REAL_TYPE, srcRank1[0][i], 0, comm_cart, &req[4 * (i * 2 + 11) + 1] ));

                                    mpiErrorCheck(MPI_Isend( &sendBuffer[i][1][0], length, MPI_REAL_TYPE, dstRank[1][i], 0, comm_cart, &req[4 * (i * 2 ) + 2] ));
                                    mpiErrorCheck(MPI_Irecv( &recvBuffer[i][1][0], length, MPI_REAL_TYPE, srcRank[1][i], 0, comm_cart, &req[4 * (i * 2 ) + 3] ));

                                    mpiErrorCheck(MPI_Isend( &sendBuffer[i][1][0], length, MPI_REAL_TYPE, dstRank1[1][i], 0, comm_cart, &req[4 * (i * 2 + 1) + 2] ));
                                    mpiErrorCheck(MPI_Irecv( &recvBuffer[i][1][0], length, MPI_REAL_TYPE, srcRank1[1][i], 0, comm_cart, &req[4 * (i * 2 + 1) + 3] ));

                                    mpiErrorCheck(MPI_Isend( &sendBuffer[i][1][0], length, MPI_REAL_TYPE, dstRank1[1][i], 0, comm_cart, &req[4 * (i * 2 + 2) + 2] ));
                                    mpiErrorCheck(MPI_Irecv( &recvBuffer[i][1][0], length, MPI_REAL_TYPE, srcRank1[1][i], 0, comm_cart, &req[4 * (i * 2 + 2) + 3] ));

                                    mpiErrorCheck(MPI_Isend( &sendBuffer[i][1][0], length, MPI_REAL_TYPE, dstRank1[1][i], 0, comm_cart, &req[4 * (i * 2 + 3) + 2] ));
                                    mpiErrorCheck(MPI_Irecv( &recvBuffer[i][1][0], length, MPI_REAL_TYPE, srcRank1[1][i], 0, comm_cart, &req[4 * (i * 2 + 3) + 3] ));

                                    mpiErrorCheck(MPI_Isend( &sendBuffer[i][1][0], length, MPI_REAL_TYPE, dstRank1[1][i], 0, comm_cart, &req[4 * (i * 2 + 4) + 2] ));
                                    mpiErrorCheck(MPI_Irecv( &recvBuffer[i][1][0], length, MPI_REAL_TYPE, srcRank1[1][i], 0, comm_cart, &req[4 * (i * 2 + 4) + 3] ));

                                    mpiErrorCheck(MPI_Isend( &sendBuffer[i][1][0], length, MPI_REAL_TYPE, dstRank1[1][i], 0, comm_cart, &req[4 * (i * 2 + 5) + 2] ));
                                    mpiErrorCheck(MPI_Irecv( &recvBuffer[i][1][0], length, MPI_REAL_TYPE, srcRank1[1][i], 0, comm_cart, &req[4 * (i * 2 + 5) + 3] ));

                                    mpiErrorCheck(MPI_Isend( &sendBuffer[i][1][0], length, MPI_REAL_TYPE, dstRank1[1][i], 0, comm_cart, &req[4 * (i * 2 + 6) + 2] ));
                                    mpiErrorCheck(MPI_Irecv( &recvBuffer[i][1][0], length, MPI_REAL_TYPE, srcRank1[1][i], 0, comm_cart, &req[4 * (i * 2 + 6) + 3] ));

                                    mpiErrorCheck(MPI_Isend( &sendBuffer[i][1][0], length, MPI_REAL_TYPE, dstRank1[1][i], 0, comm_cart, &req[4 * (i * 2 + 7) + 2] ));
                                    mpiErrorCheck(MPI_Irecv( &recvBuffer[i][1][0], length, MPI_REAL_TYPE, srcRank1[1][i], 0, comm_cart, &req[4 * (i * 2 + 7) + 3] ));

                                    mpiErrorCheck(MPI_Isend( &sendBuffer[i][1][0], length, MPI_REAL_TYPE, dstRank1[1][i], 0, comm_cart, &req[4 * (i * 2 + 8) + 2] ));
                                    mpiErrorCheck(MPI_Irecv( &recvBuffer[i][1][0], length, MPI_REAL_TYPE, srcRank1[1][i], 0, comm_cart, &req[4 * (i * 2 + 8) + 3] ));

                                    mpiErrorCheck(MPI_Isend( &sendBuffer[i][1][0], length, MPI_REAL_TYPE, dstRank1[1][i], 0, comm_cart, &req[4 * (i * 2 + 9) + 2] ));
                                    mpiErrorCheck(MPI_Irecv( &recvBuffer[i][1][0], length, MPI_REAL_TYPE, srcRank1[1][i], 0, comm_cart, &req[4 * (i * 2 + 9) + 3] ));

                                    mpiErrorCheck(MPI_Isend( &sendBuffer[i][1][0], length, MPI_REAL_TYPE, dstRank1[1][i], 0, comm_cart, &req[4 * (i * 2 + 10) + 2] ));
                                    mpiErrorCheck(MPI_Irecv( &recvBuffer[i][1][0], length, MPI_REAL_TYPE, srcRank1[1][i], 0, comm_cart, &req[4 * (i * 2 + 10) + 3] ));

                                    mpiErrorCheck(MPI_Isend( &sendBuffer[i][1][0], length, MPI_REAL_TYPE, dstRank1[1][i], 0, comm_cart, &req[4 * (i * 2 + 11) + 2] ));
                                    mpiErrorCheck(MPI_Irecv( &recvBuffer[i][1][0], length, MPI_REAL_TYPE, srcRank1[1][i], 0, comm_cart, &req[4 * (i * 2 + 11) + 3] ));
                            }
                            mpiErrorCheck(MPI_Waitall( 48* nDims, &req[0], &stt[0] )); */

#endif

/// CASE 1
#ifdef NB_ISENDRECV

                            if (gridrank == 0 && timeStep == 0)
                                std::cout << "\nCommunication Pattern: Pairwise non-blocking send and blocking receive operations" << std::endl;

                            for (std::size_t i = 0; i < nDims; i++)
                            {
                                for (std::size_t j = 0; j < nDirs; j++)
                                {
                                    mpiErrorCheck(MPI_Isend(&sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank[j][i], 0, comm_cart, &req[0]));
                                    mpiErrorCheck(MPI_Recv(&recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank[j][i], 0, comm_cart, MPI_STATUS_IGNORE));
                                    mpiErrorCheck(MPI_Barrier(comm_cart));
                                }
                            }

#endif

// mpiErrorCheck(MPI_Barrier( comm_cart ));
#ifdef USE_LIKWID
                            LIKWID_MARKER_STOP("Case:PairwiseSendrecv");

#else
#ifdef TIME_REP
            }
#endif
            for (std::size_t i = 0; i < gridNTasks; ++i)
            {
                if (gridrank == i)
                {
                    maint4 = high_resolution_clock::now();
                    // duration<timeT> SendrecvDuration = duration_cast<duration<timeT>>(maint4 - maint3);
                    timeT SendrecvDuration = duration_cast<nanoseconds>(maint4 - maint3).count();
#ifdef TIME_REP
                    SendrecvDuration /= static_cast<double>(nCppTimeRep);
#endif
                    // aggregate_time2 += SendrecvDuration.count();
                    aggregate_time2 += SendrecvDuration;
                    // duration<timeT> CompSendrecvDuration = duration_cast<duration<timeT>>(maint4 - maint1);
                    timeT CompSendrecvDuration = duration_cast<nanoseconds>(maint4 - maint1).count();
#ifdef TIME_REP
                    CompSendrecvDuration /= static_cast<double>(nCppTimeRep);
#endif
                    // aggregate_time3 += CompSendrecvDuration.count();
                    aggregate_time3 += CompSendrecvDuration;

                    /*    if (aggregate_time1 <= timestemp)
                            mycomptime = aggregate_time1;

                        if (aggregate_time3 <= timestemp)
                            mycommtime = aggregate_time3;

                        if ((aggregate_time1 > timestemp) && (aggregate_time3 > timestemp ))
                        {
                            if (mycomptime > mycommtime)
                            {
                                //for(static bool first = true;first;first=false) { // to execute code only once }
                                std::cout  << "timestep: " << timeStep << ",\t rank in MPI calls: "<< gridrank << ",\t timestemp: " << timestemp <<  std::endl;
                                TR.time_vtkStep(gridrank, timeStep, timestemp);
                            }
                            timestemp += timestempbase;
                        }*/

                    // if( (timeStep <= 100) && ((timeStep==1) || ((timeStep % 20) ==0)) )
                    //    TR1.time_vtkStep(gridrank, timeStep, aggregate_time1);
                    //    TR2.time_vtkStep(gridrank, timeStep, aggregate_time2);
                    /*else if( (timeStep < 1000) && ((timeStep % vtkStep) ==0))
                        TR.time_vtkStep(gridrank, timeStep, aggregate_time3);
                    else if ( (timeStep < 10000) && ((timeStep % 1000) ==0))
                        TR.time_vtkStep(gridrank, timeStep, aggregate_time3);
                    else if ( (timeStep < 100000) && ((timeStep % 10000) ==0))
                        TR.time_vtkStep(gridrank, timeStep, aggregate_time3);
                    else if ( (timeStep >= 100000) && ((timeStep % 100000) ==0))
                        TR.time_vtkStep(gridrank, timeStep, aggregate_time3);*/
                    // TR.time_vtkStep(gridrank, timeStep, aggregate_time3); //CompSendrecvDuration
                }
            }
            /**
             * \brief adding time rank information of communication phase to output file
             */
            /* for(std::size_t i = 0; i < nDims; i++ ){
                 for ( std::size_t j = 0; j < 2; j++ ){
                    TR.add_transmission( srcRank[j][i], dstRank[j][i], aggregate_time1, (aggregate_time1+aggregate_time2), nTasks);
                 }
             }*/
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//    Data exchange with blocking collective messages b/w max. 2N neighbours (ND domain decompisition)  //
//////////////////////////////////////////////////////////////////////////////////////////////////////////

/// CASE 4
#ifdef B_COLLECTIVE_SENDRECV
                        }
                        realT result = 0.0;
                        if (timeStep == collectiveOPTimeStep)
                        {
                            if (gridrank == 0)
                                std::cout << "\nCommunication Pattern for time step iteration " << timeStep << ": Collective blocking send and receive operation\n"
                                          << std::endl;

// mpiErrorCheck(MPI_Barrier( comm_cart ));
#ifdef USE_LIKWID
                            LIKWID_MARKER_START("Case:BCollectiveSendRecv");
#else
                            // high_resolution_clock::time_point maint5 = high_resolution_clock::now();
                            for (std::size_t timing_repetitions = 0; timing_repetitions < nCppTimeRep; ++timing_repetitions)
                            {
#endif

                            /// sum up all "sum" into result
                            if (gridrank != 0)
                            {
                                mpiErrorCheck(MPI_Send(&sum, 1, MPI_REAL_TYPE, 0, 1, MPI_COMM_WORLD));
                            }
                            else
                            {
                                MPI_Status status;
                                result = sum;
                                for (int i = 1; i < gridNTasks; ++i)
                                {
                                    mpiErrorCheck(MPI_Recv(&sum, 1, MPI_REAL_TYPE, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status));
                                    result += sum;
                                }
                            }

// mpiErrorCheck(MPI_Barrier( comm_cart ));
#ifdef USE_LIKWID
                            LIKWID_MARKER_STOP("Case:BCollectiveSendRecv");
#else
                            }
                            /*		high_resolution_clock::time_point maint6 = high_resolution_clock::now();
                                    duration<timeT> BReduceDuration = duration_cast<duration<timeT>>(maint6 - maint5);
                                    BReduceDuration/= static_cast<double>(nCppTimeRep);
                                    aggregate_time2 += BReduceDuration.count();
                                    duration<timeT> CompBReduceDuration = duration_cast<duration<timeT>>(maint6 - maint1);
                                    CompBReduceDuration /= static_cast<double>(nCppTimeRep);
                                    aggregate_time3 += CompBReduceDuration.count();
                        */
#ifdef USE_PISOLVER
                            /// calculate and print pi for the iteration where collective operation happens
                            if (gridrank == 0)
                                std::cout << "PI = " << result * stepLength << " for time step iteration " << timeStep << " with error compare to the Pi in 'math.h': " << fabs((result * stepLength) - PiExact) << std::endl;
#endif

                                /// adding time rank information of communication phase to output file
                                /*
                                 for(std::size_t i = 0; i < nDims; i++ ){
                                     for ( std::size_t j = 0; j < nDirs ; j++ ){
                                         TR.add_transmission( srcRank[j][i], dstRank[j][i], SendrecvDuration.count()/1000000000, BReduceDuration.count()/1000000000, nTasks);
                                     }
                                 }*/

#endif
                        }
#endif // B_COLLECTIVE_SENDRECV

/**
 * \brief MPI_Reduce reduces "sum" values on all processes to a single "result" value.
 * \param &sum initial address of send buffer
 * \param 1 number of elements in send buffer (integer)
 * \param MPI_REAL_TYPE data type of elements of send buffer (handle)
 * \param MPI_SUM reduce operation (handle)
 * \param 0 rank of root process (integer)
 * \param comm_cart new communicator (handle)
 * \return &result initial address of recv buffer
 */
/// CASE 5
#ifdef B_COLLECTIVE_REDUCE
                    }

                    realT result = 0.0;
                    if (timeStep == collectiveOPTimeStep)
                    {
                        if (gridrank == 0)
                            std::cout << "\nCommunication Pattern for time step iteration " << timeStep << ": Collective blocking reduce operation\n"
                                      << std::endl;

// mpiErrorCheck(MPI_Barrier( comm_cart ));
#ifdef USE_LIKWID
                        LIKWID_MARKER_START("Case:BCollectiveReduce");
#else
                        high_resolution_clock::time_point maint5 = high_resolution_clock::now();
                        for (std::size_t timing_repetitions = 0; timing_repetitions < nCppTimeRep; ++timing_repetitions)
                        {
#endif

                        for (std::size_t i = 0; i < nDims; i++)
                        {
                            for (std::size_t j = 0; j < nDirs; j++)
                            {
                                /*mpiErrorCheck(MPI_Reduce(&sendBuffer[i][j][0], &recvBuffer[i][j][0], 1, MPI_REAL_TYPE, MPI_SUM, 0, comm_cart));
                                mpiErrorCheck(MPI_Allreduce(&sendBuffer[i][j][0], &recvBuffer[i][j][0], 1, MPI_REAL_TYPE, MPI_SUM, comm_cart));
                                mpiErrorCheck(MPI_Scatter(&sendBuffer[i][j][0], 1, MPI_REAL_TYPE, &recvBuffer[i][j][0], 1, MPI_REAL_TYPE, 0, comm_cart));
                                mpiErrorCheck(MPI_Gather(&sendBuffer[i][j][0], 1, MPI_REAL_TYPE, &recvBuffer[i][j][0], 1, MPI_REAL_TYPE, 0, comm_cart));
                                mpiErrorCheck(MPI_Allgather(&sendBuffer[i][j][0], 1, MPI_REAL_TYPE, &recvBuffer[i][j][0], 1, MPI_REAL_TYPE, comm_cart));
                                mpiErrorCheck(MPI_Alltoall(&sendBuffer[i][j][0], 1, MPI_REAL_TYPE, &recvBuffer[i][j][0], 1, MPI_REAL_TYPE, comm_cart));
                                mpiErrorCheck(MPI_Bcast(sendBuffer[i][j][0], 1, MPI_REAL_TYPE, 0, comm_cart));*/
                                mpiErrorCheck(MPI_Allgather(&sendBuffer[i][j][0], 1, MPI_REAL_TYPE, &recvBuffer[i][j][0], 1, MPI_REAL_TYPE, comm_cart));
                            }
                        }

// mpiErrorCheck(MPI_Barrier( comm_cart ));
#ifdef USE_LIKWID
                        LIKWID_MARKER_STOP("Case:BCollectiveReduce");
#else
                        }
                        high_resolution_clock::time_point maint6 = high_resolution_clock::now();
                        duration<timeT> BReduceDuration = duration_cast<duration<timeT>>(maint6 - maint5);
                        BReduceDuration /= static_cast<double>(nCppTimeRep);
                        aggregate_time2 += BReduceDuration.count();
                        duration<timeT> CompBReduceDuration = duration_cast<duration<timeT>>(maint6 - maint1);
                        CompBReduceDuration /= static_cast<double>(nCppTimeRep);
                        aggregate_time3 += CompBReduceDuration.count();

#ifdef USE_PISOLVER
                        /// calculate and print pi for the iteration where collective operation happens
                        if (gridrank == 0)
                            std::cout << "PI = " << result * stepLength << " for time step iteration " << timeStep << " with error compare to the Pi in 'math.h': " << fabs((result * stepLength) - PiExact) << std::endl;
#endif

/// adding time rank information of communication phase to output file

/*for(std::size_t i = 0; i < nDims; i++ ){
    for ( std::size_t j = 0; j < nDirs ; j++ ){
        TR.add_transmission( srcRank[j][i], dstRank[j][i], SendrecvDuration.count()/1000000000, BReduceDuration.count()/1000000000, nTasks);
    }
}*/
#endif
                    }
#endif // B_COLLECTIVE_REDUCE

                } //!< MAINtimeStep

/**
 *\brief print the final time
 */
#ifndef USE_LIKWID
                if (gridrank == 0)
                {
                    std::cout << "\nComputation phase for " << aggregate_time1 * 1000 << " ms" << std::endl;
                    std::cout << "Pairwise communication phase time without computation for " << length << " doubles: " << aggregate_time2 * 1000 << " ms" << std::endl;
                    std::cout << "Total Pairwise communication phase time with computation for " << length << " doubles: " << aggregate_time3 * 1000 << " ms" << std::endl;
                    std::cout << "Collective communication phase time without computation for " << length << " doubles: " << aggregate_time4 * 1000 << " ms" << std::endl;
                    std::cout << "Total Collective communication phase time with computation for " << length << " doubles: " << aggregate_time5 * 1000 << " ms" << std::endl;
                    std::cout << "\nFor detailed time-rank traces, see 'timeRankViz.cvs' file \n " << std::endl;
                    std::cout << " totaltime " << aggregate_time1 * 1000 << " " << aggregate_time2 * 1000 << " " << aggregate_time3 * 1000 << std::endl;
                }
#endif

                /**
                 *\brief free resources (marks the MPI communicator objects for deallocation)
                 */
                try
                {
                    for (std::size_t x = 0; x < nDims; ++x)
                    {
                        for (std::size_t y = 0; y < nDirs; ++y)
                        {
                            delete[] sendBuffer[x][y];
                        }
                        delete[] sendBuffer[x];
                    }
                    delete[] sendBuffer;

                    for (std::size_t x = 0; x < nDims; ++x)
                    {
                        for (std::size_t y = 0; y < nDirs; ++y)
                        {
                            delete[] recvBuffer[x][y];
                        }
                        delete[] recvBuffer[x];
                    }
                    delete[] recvBuffer;

#ifdef USE_TRIAD
                    free(A);
                    free(B);
                    free(C);
#endif

#ifdef USE_EXTRAWORK
                    free(buf);
#endif

                    mpiErrorCheck(MPI_Comm_free(&comm_cart));
                }
                catch (std::exception &e)
                {
                    std::cerr << "error on deallocating resouces: " << e.what() << std::endl;
                    return (EXIT_FAILURE);
                }
            } //!< IFLOOP
            else
            {
                std::cout << "Node rank " << rank << " of " << nTasks << " is outside the Cartesian grid topology" << std::endl;
            }

/**
 *\brief finalize likwid environment
 */
#ifdef USE_LIKWID
            LIKWID_MARKER_CLOSE;
#endif
            /**
             *\brief finalize MPI environment
             */
            mpiErrorCheck(MPI_Finalize());

            return EXIT_SUCCESS;
        }
