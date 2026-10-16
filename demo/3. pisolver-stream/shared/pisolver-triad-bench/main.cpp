///    \file
///    MAIN_CPP
///
///    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
///
/// Purpose: pisolver-triad-bench (Pi + STREAM Triad, MPI ring) for
///          DisCostiC-demo measured validation against PITRIADLBL.
///
#include <vector>
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <algorithm>    //!< e.g., std::all_of
#include <numeric>      //!< e.g., std::accumulate
#include <mpi.h>
#include <unistd.h>
#include <iomanip>
#include <fstream>
#include <thread>
#include <random>

#ifdef USE_HYBRID_OPENMP
#include <omp.h>
#endif

#ifdef USE_LIKWID
#include <likwid.h>     //!< time measurements METHOD 1
#else
#include <chrono>       //!< time measurements METHOD 2
using namespace std::chrono;
#endif

#include "helper.hpp"    		//!< helper functions like printing ND vectors, error check for all MPI call routines, etc.,
#include "timeRankOP.hpp"       //!< time rank output file
#include "configParser.hpp"     //!< configuration file parser

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
#ifdef USE_PISOLVER
void PISOLVER(realT sum, realT x, const long nSteps, int gridNTasks, int gridrank, const realT stepLength);
#endif
#ifdef USE_TRIAD
void TRIAD(std::size_t arrayElements, triadT *A, triadT *B, triadT *C, double scalar);
#endif

int main( int argc, char *argv[] )
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
	mpiErrorCheck(MPI_Init( &argc, &argv ));
	int rank, nTasks;
	mpiErrorCheck(MPI_Comm_rank( MPI_COMM_WORLD, &rank )); 	//!< get processors id (rank)
	mpiErrorCheck(MPI_Comm_size( MPI_COMM_WORLD, &nTasks ));   //!< get total number of processors (processors size)

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
	bool exists = CFG.getKey("filename");                        //!< check if a user-specified file exists
	//if ( rank == 0 ) std::cout << "\nUser-specified device: " << std::boolalpha << exists << std::endl;
	const char* filename ="NormalDist.bin";

	std::size_t ppn = CFG.getValue<std::size_t>("ppn");  //!< get the value of a user-specified processes count per node
	std::size_t nDims = CFG.getValue<std::size_t>("Dims");  //!< get the value of a user-specified Cartesian grid dimension

	std::vector<int> dims (nDims);                           //!< the number of nodes in each direction
	dims[0] = CFG.getValue<int>("dimX");                       //!< get the value of a user-specified number of nodes/domains in x dimension
	if(nDims==2)	dims[1] = CFG.getValue<int>("dimY");       //!< get the value of a user-specified number of nodes/domains in y dimension
	if(nDims==3){
		dims[1] = CFG.getValue<int>("dimY");        		   //!< get the value of a user-specified number of nodes/domains in y dimension
		dims[2] = CFG.getValue<int>("dimZ");      	           //!< get the value of a user-specified number of nodes/domains in z dimension
	}

	int const nDirs = CFG.getValue<int>("numDirShifts");             			//!< get the value of a user-specified number of direction of shifts in each dimension
	int ShiftStep = CFG.getValue<int>("ShiftStep");             		//!< get the value of a user-specified step size of shifts in each dimension
	std::vector<int> periods (nDims);                        			//!< specify which Cartesian directions are periodic (periods =0 for no periods) or non-periodic (periods =1 for wrap-around)
	periods[0] = CFG.getValue<int>("periodicX");               			//!< get the value of a user-specified periodicity in x direction of Cartesian grid dimension
	if(nDims==2)	periods[1] = CFG.getValue<int>("periodicY");		//!< get the value of a user-specified periodicity in y direction of Cartesian grid dimension
	if(nDims==3){
		periods[1] = CFG.getValue<int>("periodicY");					//!< get the value of a user-specified periodicity in y direction of Cartesian grid dimension
		periods[2] = CFG.getValue<int>("periodicZ");        	   		//!< get the value of a user-specified periodicity in z direction Cartesian grid dimension
	}

	if( rank == 0 ) {
		//std::cout << "Cartesian grid dimensionality: " << nDims << "D" << std::endl;
		//std::cout << "Periodicity of Cartesian directions: ";
		//print_1D(periods);
	}

	realT msgSizeBytes = CFG.getValue<realT>("msgSizeBytes");          //!< get the value of a user-specified message size in bytes
	int length = msgSizeBytes / sizeof(realT);
	std::size_t nTimeStep = CFG.getValue<std::size_t>("numTimeStep");  	//!< get the value of a user-specified outer iteration count
	std::size_t collectiveOPTimeStep = CFG.getValue<std::size_t>("collectiveOPTimeStep");	//!< get the value of a user-specified Outer iteration count where collective communication happens
	std::size_t nCppTimeRep = CFG.getValue<std::size_t>("timingRep");   //!< get the value of a user-specified timing repetitions
	std::size_t vtkStep = CFG.getValue<std::size_t>("vtk_step");
	std::size_t WUStep = CFG.getValue<std::size_t>("warmup_steps");

	//if( rank == 0 )   std::cout << "Message size: " << length << " doubles or " << length *sizeof(realT) << " bytes" << std::endl;

#ifdef USE_PISOLVER
	const long nSteps = CFG.getValue<realT>("piSize");          						//!< get the value of a user-specified number of steps for pi calculation
	if( rank == 0 ) {
		//std::cout << "Number of steps for pi calculation: " << nSteps << std::endl;
		//std::cout << "Number of outer loop iterations: "  << nTimeStep << std::endl;
	}
#endif

#ifdef USE_TRIAD
	std::size_t arraySize = CFG.getValue<std::size_t>("arraySize");          		//!< get the value of a user-specified number of elements per array
	std::size_t arrayAlignment = CFG.getValue<std::size_t>("arrayAlignment");   	//!< get the value of a user-specified scalar coefficient value
	double scalar = CFG.getValue<double>("scalar");          				//!< get the value of a user-specified array alignment value
	if( rank == 0 ) {
		//std::cout << "Total aggregate array size: "  << arraySize << std::endl;
		//std::cout << "Total aggregate memory per array in MB: "  << sizeof(triadT) * ( (double) arraySize / 1024.0/1024.0) << std::endl;
		//std::cout << "Required total aggregate memory in MB: "  << (3.0 * sizeof(triadT)) * ( (double) arraySize / 1024.0/1024.) << std::endl;
	}
#endif

#ifdef USE_IDLEPERIOD
	std::size_t extraIt = CFG.getValue<std::size_t>("extraIt");         			//!< get the value of a user-specified number of extra iterations for a processor
	std::size_t extraTriadIt = CFG.getValue<std::size_t>("extraTRIADIt");               //!< get the value of a user-specified number of extra iterations for a processor
	//std::size_t extraTriadIt1 = CFG.getValue<std::size_t>("extraTRIADIt1");
    //std::size_t extraTriadIt2 = CFG.getValue<std::size_t>("extraTRIADIt2");
    std::size_t IdlePeriodTimeStep = CFG.getValue<std::size_t>("idlePeriodTimeStep"); 	//!< get the value of a user-specified time step number for extra iterations
	//std::size_t IdlePeriodTimeStep1 = CFG.getValue<std::size_t>("idlePeriodTimeStep1");   //!< get the value of a user-specified time step number for extra iterations
	//std::size_t IdlePeriodTimeStep2 = CFG.getValue<std::size_t>("idlePeriodTimeStep2");   //!< get the value of a user-specified time step number for extra iterations
    int IdlePeriodRank = CFG.getValue<int>("idlePeriodRank");     		//!< get the value of a user-specified number of extra iterations for a processor
    //int minimum_iterations = CFG.getValue<int>("minimum_iterations");
    //int maximum_iterations = CFG.getValue<int>("maximum_iterations");
    //int minimum_step = CFG.getValue<int>("minimum_step");

#endif


#if defined(USE_EXTRAWORK) || defined(USE_EXTRAWORKPI)
	std::size_t extraItTimeStep1 = CFG.getValue<std::size_t>("extraItTimeStep");     //!< get the value of a user-specified time step number for extra iterations
    std::size_t extraItTimeStep2 = CFG.getValue<std::size_t>("extraItTimeStep1");
    std::size_t extraItTimeStep3 = CFG.getValue<std::size_t>("extraItTimeStep2");
    std::size_t NoisePeriodRank = CFG.getValue<std::size_t>("NoisePeriodRank");
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

	if ( std::any_of(dims.begin(), dims.end(), [](int i) { return i==0; })){
		mpiErrorCheck(MPI_Dims_create( nTasks, nDims, &dims[0] ));
	}

	if ( rank == 0 ){
		std::size_t nDDDims= nDims - std::count(dims.begin(), dims.end(), 1);
		//std::cout << "Domain decomposition dimensionality: " << nDDDims << "D" << std::endl;

		//std::cout << "Cartesian grid topology: ";
		//print_1D(dims);

		/**
		 * \brief debug use only
		 *  it is erroneous if it specifies a grid that is larger than the group size. (no. of processes per node must >= 1)
		 *	vector sizes of grid dimensionality and periodicity should be same
		 */
#ifdef USE_DEBUG
		if( std::accumulate( dims.begin(), dims.end(), 1, std::multiplies<int>() ) > nTasks )
			throw std::runtime_error(" error: not enough nodes in original communicator");

		if (nDDDims > nDims)
			throw std::runtime_error(" error: not enough grid dimensionality for decomposition");

		if ( dims.size() != periods.size() )
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

	MPI_Comm comm_cart = MPI_COMM_NULL;                       	//!< new Cartesian communicator
	std::size_t reorder = 1;                                  	//!< rank reorder is allowed (rank in MPI_COMM_WORLD and comm_cart communicators may differ)
	mpiErrorCheck(MPI_Cart_create( MPI_COMM_WORLD, nDims, &dims[0], &periods[0], reorder, &comm_cart));

	/// if the node is inside the Cartesian grid topology
	if ( comm_cart != MPI_COMM_NULL){
		/**
		  \brief rank of current node in new communicator (rank differ from old communicator, if reorder allowed)
		 */
		int gridrank =MPI_PROC_NULL;
		mpiErrorCheck(MPI_Comm_rank( comm_cart, &gridrank ));

		/**
		  \brief the number of nodes in current new communicator
		 */
		int gridNTasks=0;
		mpiErrorCheck(MPI_Comm_size( comm_cart, &gridNTasks ));

		/**
		 * \brief Cartesian coodinates of nodes in current new communicator
		 * \param comm_cart communicator with Cartesian structure (handle)
		 * \param gridrank rank of a process within group of comm (integer)
		 * \param nDims length of vector coords in the calling program (integer)
		 * \return Cartesian coodinates of specified process/node in current new communicator (integer vector of size nDims)
		 */
		std::vector<int> coords( nDims );
		mpiErrorCheck(MPI_Cart_coords( comm_cart, gridrank, nDims, &coords[0] ));

		/**
		 * \brief debug use only
		 * grid rank should be in the grid
		 * vector sizes of grid dimensionality and coordinates should be same
		 * coordinates check for non-periodic directions
		 */
#ifdef USE_DEBUG
		if (gridrank == 0){
			if ( gridrank < 0 || gridrank > gridNTasks )
				throw std::runtime_error(" error: gridrank is not in the grid");

			if ( coords.size() != dims.size() )
				throw std::runtime_error(" error: size mismatch for dims and coords");

			bool test = true;
			for ( std::size_t i = 0; test && i < coords.size(); ++i )
				test = periods[i] || ( coords[i] > -1 && coords[i] < dims[i] );
			if (test == false)
				throw std::runtime_error( " error: coordinates check FAIL for non-periodic directions");
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

		//std::vector<std::vector<int>> srcRank (nDirs, std::vector<int>(nDims));	//!< std::vector
		//std::vector<std::vector<int>> dstRank (nDirs, std::vector<int>(nDims));

		//std::array<std::array<int, 3>, 2 > dstRank;								//!< std::array (T,COL,ROW)
		//std::array<std::array<int, 3>, 2 > srcRank;

		int srcRank[nDirs][nDims], dstRank[nDirs][nDims]; 							//!< C-style arrays
		int negShiftStep = -1*ShiftStep;
		for(std::size_t i = 0; i < nDims; i++ )
		{
			/// for positive displacement in each dimensions
			mpiErrorCheck(MPI_Cart_shift( comm_cart, i,  ShiftStep, &srcRank[0][i], &dstRank[0][i] ));

			/// for negative displacement in each dimensions
			if (nDirs ==2) mpiErrorCheck(MPI_Cart_shift( comm_cart, i, negShiftStep, &srcRank[1][i], &dstRank[1][i] ));
		}

		/// for indirect neighbour communication in parallel
		/*int srcRank1[nDirs][nDims], dstRank1[nDirs][nDims]; 							//!< C-style arrays
		  int negShiftStep1 = -1*(ShiftStep+1);
		  for(std::size_t i = 0; i < nDims; i++ )
		  {
		/// for positive displacement in each dimensions
		mpiErrorCheck(MPI_Cart_shift( comm_cart, i,  ShiftStep+1, &srcRank1[0][i], &dstRank1[0][i] ));

		/// for negative displacement in each dimensions
		if (nDirs ==2) mpiErrorCheck(MPI_Cart_shift( comm_cart, i, negShiftStep1, &srcRank1[1][i], &dstRank1[1][i] ));
		} */

		/**
		 * \brief print my neighbours
		 */
#ifdef USE_ALLPRINT
		//if ( rank == 0 ) {
		/* /// print std::vector
		   std::cout<<"source ranks:"<<std::endl;
		   print_2D(srcRank);
		   std::cout<<"destination ranks:"<<std::endl;
		   print_2D(dstRank);*/
		for ( int i = 0; i < nDims; i++ ){
			for ( int j = 0; j < nDirs; j++ ){
				std::cout << "srcRank[" << j << "][" << i << "]=" <<  srcRank[j][i] << std::endl;
				std::cout << "dstRank[" << j << "][" << i << "]=" <<  dstRank[j][i] << std::endl;

			}
		}
		//}
#endif

		/**
		 *\brief initialize user interface (time-rank output) environment
		 */
		userInterface::timeRankOP TR(gridNTasks, gridrank);

		/**
		 *\brief problem initialization
		 */
#ifdef USE_PISOLVER
		realT sum, x;
		const realT stepLength = 1./nSteps;
		const realT PiExact = 3.1415926535897932;										//!< Pi value in math.h
#endif

#ifdef USE_TRIAD
		triadT * A, *B, *C;
		size_t arrayElements = arraySize / gridNTasks;												//!< array size of each MPI rank
		size_t memA = posix_memalign((void **)&A, arrayAlignment, arrayElements * sizeof(triadT));	//!< dynamic allocation of the array a
		size_t memB = posix_memalign((void **)&B, arrayAlignment, arrayElements * sizeof(triadT));	//!< dynamic allocation of the array b
		size_t memC = posix_memalign((void **)&C, arrayAlignment, arrayElements * sizeof(triadT));	//!< dynamic allocation of the array c

#ifdef USE_DEBUG
		if (memA != 0) {
			throw std::runtime_error(" error: allocation of array A FAIL");
			MPI_Abort(comm_cart, 2);
		}
		if (memB != 0) {
			throw std::runtime_error(" error: allocation of array B FAIL");
			MPI_Abort(comm_cart, 2);
		}
		if (memC != 0) {
			throw std::runtime_error(" error: allocation of array C FAIL");
			MPI_Abort(comm_cart, 2);
		}
#endif

		/// initialisation of the array a, b and c
		for ( std::size_t i=0; i<arrayElements; i++) {
			A[i] = 2.0;
			B[i] = 2.0;
			C[i] = 1.0;
		}
#endif

		/**
		 *\brief send and receive buffers for uni/bi-directional displacements in each dimension
		 */
		//realT sendBuffer[nDims][nDirs][length], recvBuffer[nDims][nDirs][length];

		//auto sendBuffer = new realT[nDims][2][140000000];
		//auto recvBuffer = new realT[nDims][2][140000000];

		realT ***sendBuffer = new realT**[nDims];
		for(std::size_t i = 0; i < nDims; ++i) {
			sendBuffer[i] = new realT*[nDirs];
			for (std::size_t j = 0; j < nDirs; ++j)
				sendBuffer[i][j] = new realT[length];
		}

		realT ***recvBuffer = new realT**[nDims];
		for(std::size_t i = 0; i < nDims; ++i) {
			recvBuffer[i] = new realT*[nDirs];
			for (std::size_t j = 0; j < nDirs; ++j)
				recvBuffer[i][j] = new realT[length]();
		}

		for ( std::size_t i = 0; i < nDims; ++i ){
			for ( std::size_t j = 0; j < nDirs; ++j ){
				for ( std::size_t k = 0; k < length; ++k ){
					sendBuffer[i][j][k] = (double) (i+1)*(gridrank+1) + i;
					recvBuffer[i][j][k] = 0.0;
				}
			}
		}

#ifndef USE_LIKWID
		timeT aggregate_time1 = 0.0, aggregate_time2 = 0.0, aggregate_time3 = 0.0;
		timeT aggregate_time4 = 0.0, aggregate_time5 = 0.0;
#endif

#if defined(USE_EXTRAWORK) || defined(USE_EXTRAWORKPI)
		const char* filename ="NormalDist.bin";

		//int totalElements= 2880;
		//MPI_Offset start = totalElements *  gridrank / gridNTasks;
		//MPI_Offset end = totalElements * (gridrank+1) / gridNTasks;
		//int bufsize = end - start;
		//MPI_Offset disp = start * sizeof(int);
		//std::cout << "process " << gridrank << " start " << start << " end " << end << " buffer size " << bufsize <<std::endl;

		mpiErrorCheck(MPI_Barrier( comm_cart ));

		MPI_File inFile;
		mpiErrorCheck(MPI_File_open(comm_cart, filename, MPI::MODE_RDONLY, MPI::INFO_NULL, &inFile)); // MPI_COMM_SELF to open binary file by one process per task instead of all tasks at the same time with comm_cart

		MPI_Offset filesizeByte, filesize;
		mpiErrorCheck(MPI_File_get_size(inFile, &filesizeByte));    // in bytes (e.g., 2880 * 4 bytes)
		filesize    = filesizeByte / sizeof(int);                   // in number of ints (e.g., 2880 ints)
		int bufsize = filesize / (gridNTasks);                        // local number to read (e.g., 30 ints)
		int bufsizeByte = bufsize * sizeof(int);
		//if (gridrank == 0) std::cout << "Each process read " << filesize << " ints/ " << filesizeByte << " bytes of filesize and " << bufsize << " ints / " << bufsizeByte << " bytes of buffer size " <<std::endl;

		int *buf = (int *) malloc (bufsizeByte);                    // allocate memory
		for ( std::size_t i = 0; i < bufsize ; i++ )
			buf[i] = 123;

		MPI_Offset disp = gridrank * bufsizeByte;
		mpiErrorCheck(MPI_File_set_view(inFile, disp, MPI_INT, MPI_INT, "native", MPI_INFO_NULL));
		MPI_Status status;
		mpiErrorCheck(MPI_File_read(inFile, buf, bufsize, MPI_INT, &status));
		//mpiErrorCheck(MPI_File_read_at(inFile, disp, buf, bufsize, MPI_INT, &status));

		int count;
		mpiErrorCheck(MPI_Get_count(&status, MPI_INT, &count));

		if (count != bufsize)                                       // check if complete buffer has been read
			std::cout << " error: not complete buffer has been READ"<< std::endl;

		MPI_File_close(&inFile);

		int *workIterations = (int *) malloc (bufsize * sizeof(int));
		const double divideTime_ns = 0.231256113833509; // time in nano seconds taken by a divide operation 6000000/25945260 (emmy)
		//const double divideTime_ns =  0.231872136958789; //0.017608333540623; // time in nano seconds taken by a divide operation 17763000/75945260 (meggie)

		for ( int j = 0; j < gridNTasks ; j++ ) {
			if (gridrank == j) {
				for ( int i = 0; i < bufsize ; i++ ){
					workIterations[i] = buf[i] * 15 /  divideTime_ns;
#ifdef USE_ALLPRINT
					std::cout << "Process " << gridrank << " read " << count << " ints and at index " << i << " has value " <<  buf[i] << " ns and number of iterations " << workIterations[i] << std::endl;
#endif
				}
			}
		}
#endif
		mpiErrorCheck(MPI_Barrier( comm_cart ));

		/// warm-up phase
		for ( std::size_t warmupStep = 0 ; warmupStep <= WUStep ; warmupStep++ )
		{
#ifdef USE_PISOLVER
			//printf("ntimesteps:%d, stepsize:%d", WUStep, nSteps);
			PISOLVER(sum, x, nSteps, gridNTasks, gridrank, stepLength);
#endif

#ifdef USE_TRIAD
			TRIAD(arrayElements, A, B, C, scalar);
#endif


#ifdef B_PAIRWISE
					for ( std::size_t i = 0; i < nDims; i++ )
					{
						for ( std::size_t j = 0; j < nDirs; j++ )
						{
							mpiErrorCheck(MPI_Sendrecv( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank[j][i], j, &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank[j][i], j, comm_cart, MPI_STATUS_IGNORE ));
						}
					}
#endif

					MPI_Request req[2*nDims];
					MPI_Status  stt[2*nDims];

					/// CASE 2
#ifdef NB_ISENDIRECV
					for ( std::size_t i = 0; i < nDims; i++ )
					{
						for ( std::size_t j = 0; j < nDirs; j++ )
						{
							mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank[j][i], 5, comm_cart, &req[0] ));
							mpiErrorCheck(MPI_Irecv( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank[j][i], 5, comm_cart, &req[1] ));
							mpiErrorCheck(MPI_Waitall( 2, &req[0], &stt[0] ));
						}
					}
#endif

					/// CASE 1
#ifdef NB_ISENDRECV
					for ( std::size_t i = 0; i < nDims; i++ )
					{
						for ( std::size_t j = 0; j < nDirs; j++ )
						{
							mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank[j][i], 0, comm_cart, &req[0] ));
							mpiErrorCheck(MPI_Recv ( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank[j][i], 0, comm_cart, MPI_STATUS_IGNORE ));
							//mpiErrorCheck(MPI_Barrier( comm_cart ));
						}
					}
#endif
				} //!< warmupStep
#ifdef USE_LIKWID
#else
				high_resolution_clock::time_point maint1, maint2, maint3, maint4;
#endif
				mpiErrorCheck(MPI_Barrier( MPI_COMM_WORLD ));
				mpiErrorCheck(MPI_Barrier( comm_cart ));
				//if( gridrank == 0) std::cout << "\nStart" <<std::endl;
				mpiErrorCheck(MPI_Barrier( comm_cart ));

				/**
				 *\brief outer main time step loop
				 */
                double pi_time = 0.0;
                double triad_time = 0.0;
                double max_triad_time;
                //int updated_step = IdlePeriodTimeStep;

                int k= 0;
                //if (gridrank == 0)
                    //std::cout << "Rank" << "," << "TRIAD TIME" << "," << "Performance(MFlop/s)" << "," << "Memory Bandwith(GB/s)" << std::endl;
                //std::cout << "Rank" << "," << "NITER" << "," << "Triad Time" << "," << "Pi Time" << "," << "Metric" << std::endl;
                if (gridrank == 0)
                    std::cout << "Rank" << "," << "NITER" << "," << "TriadTime" << "," << "PiTime" << "," << "TimePerReps" << "," << "Performance" << std::endl;
				for ( std::size_t timeStep = 0 ; timeStep <= nTimeStep ; timeStep++ )
				{
					double step_triad_time = 0.0;
					double step_pi_time = 0.0;
#ifdef USE_LIKWID
					LIKWID_MARKER_START("Case:Computation");
#else
					auto startReps = std::chrono::high_resolution_clock::now();
                    for (std::size_t i=0 ; i < gridNTasks ; ++i){
						if (gridrank == i) maint1 = high_resolution_clock::now();
					}
                    for (std::size_t timing_repetitions = 0; timing_repetitions < nCppTimeRep; ++timing_repetitions)
                    {
#endif


#ifdef USE_TRIAD
									//////////////////////////////////////////////////////////////////////////////////////////////////////////
									//                                  Stream Triad Benchmark  	                                        //
									//////////////////////////////////////////////////////////////////////////////////////////////////////////
							/**
							 *\brief a memory-bound operation used as a standard for measuring the main memory bandwidth in High Performance Computers
							 */
							//if( gridrank == 0 && timeStep == 0) std::cout << "\nComputation: memory-bound triad benchmark" <<std::endl;

							auto triad_start = std::chrono::high_resolution_clock::now();
                            TRIAD(arrayElements, A, B, C, scalar);
                            auto triad_stop = std::chrono::high_resolution_clock::now();
                            std::chrono::duration<double> triad_duration = triad_stop - triad_start;
                            step_triad_time = triad_duration.count();
                            triad_time += step_triad_time;

#ifdef USE_IDLEPERIOD
							if( gridrank == 0 && timeStep == 0)
								std::cout << "taking idle period values from configuration setting" << std::endl;
							
                            if ((timeStep==IdlePeriodTimeStep) && (gridrank % gridNTasks == IdlePeriodRank)) {
								for (std::size_t j=0; j<extraTriadIt; j++)	{
									//TRIAD(arrayElements, A, B, C, scalar);
                                    realT sum, x;
                                    const realT stepLength = 1.0 / 442452600;
                                    sum = 0.0;
                                    for (int j = 0; j < 50000000; j++) {
                                        x = 0.5*stepLength;
                                        sum = sum + 4.0 / (1.0 + x * x);
                                    }
                                    if (sum < 0) std::cout << "sum" << sum << std::endl;
								}
							}

                            
                            //realT sum, x;
                            //const realT stepLength = 1.0 / 1000000000;
                            //sum = 0.0;
                            //for (int j = 0; j < 1000000000; j++) {
                              //  x = 0.5*stepLength;
                                //sum = sum + 4.0 / (1.0 + x * x);
                            //}
                            //if (sum < 0) std::cout << "sum" << sum << std::endl;


#endif

#ifdef USE_EXTRAWORK
							/**
							 *\brief make a check that file exist. If file is not there then takes default values from configuration setting
							 */
							if (std::fstream{filename}) {
								if( gridrank == 0 && timeStep == 0)
									//std::cout << "taking fine-grained delay values from binary file" << std::endl;
								/// an extra work of number of workIterations for all processores and timestep greater than and equal to extraItTimeStep iteration
								realT sum, x;
								const realT stepLength = 1./44245260;
								sum = 0.0;
								//for ( std::size_t j = 0; j < 1100; j=j+80 ) {
								std::size_t extraItTimeStep =extraItTimeStep1;// +j;
								for (std::size_t i=0 ; i < gridNTasks ; ++i){
									if ((timeStep>=extraItTimeStep) && (timeStep<extraItTimeStep+bufsize) && (gridrank == i)) {
										for ( std::size_t j = 0; j < workIterations[timeStep-extraItTimeStep]; j++ ) {
											//if ((timeStep<bufsize) && (gridrank == i)) {
											//for ( std::size_t j = 0; j < workIterations[timeStep]; j++ ) {
											x = 0.5*stepLength;
											sum = sum + 4.0 / ( 1.0 + x * x);
										}
										//std::cout << "Process " << gridrank << " delay extra " << workIterations[timeStep-extraItTimeStep] << " iterations/ " << buf[timeStep-extraItTimeStep] << " ns for time step " << timeStep << std::endl;
										if (sum < 0) std::cout << "sum" << sum <<std::endl;
										//std::this_thread::sleep_for(std::chrono::nanoseconds((int&)buf));
										}
										//}
									}
								}
#endif
#endif

#ifdef USE_PISOLVER
						//if( gridrank == 0 && timeStep == 0) std::cout << "\nComputation: compute-bound pi solver" <<std::endl;
						/// inject idle period with an extra work for a processor for extraItTimeStep iteration and IdlePeriodRank
						/**MPI4LB varaibles**/
                        auto pi_start = std::chrono::high_resolution_clock::now();
                        PISOLVER(sum, x, nSteps, gridNTasks, gridrank, stepLength);
                        auto pi_stop = std::chrono::high_resolution_clock::now();
                        std::chrono::duration<double> pi_duration = pi_stop - pi_start;
                        step_pi_time = pi_duration.count();
                        pi_time = step_pi_time;

#ifdef USE_IDLEPERIOD_CUSTOM
						if( gridrank == 0 && timeStep == 0)
							std::cout << "taking idle period values from configuration setting" << std::endl;
                            
                        realT sum, x;
                        const realT stepLength = 1.0 / 1000000000;
                        sum = 0.0;
                        //static int randomProcessor = -1;
                        
                        static std::random_device rd;
                        std::mt19937 gen(timeStep);
                        std::uniform_int_distribution<> dist(1, 18);
                        int randomProcessor = dist(gen);

                        bool executeLoop = false;
                        //if (gridrank == 0) {
                            //randomProcessor = gen() % gridNTasks; // Random number in range [0, gridNTasks-1]
                        //}       

                        //mpiErrorCheck(MPI_Bcast(&randomProcessor, 1, MPI_INT, 0, MPI_COMM_WORLD));

                        
                        if (timeStep > 2000 && timeStep <= 4000 && timeStep % 25 == 0) {
                            //executeLoop = true; // From 2k to 4k, loop occurs every 100 iterations
                        } else if (timeStep > 4000 && timeStep <= 7000 && timeStep % 10 == 0) {
                            //executeLoop = true; // From 4k to 7k, loop occurs every 10 iterations
                        } else if (timeStep > 7000 && timeStep <= 10000 && timeStep % 4 == 0) {
                            //executeLoop = true; // From 7k to 10k, loop occurs every 4 iterations
                        }
                        
                        //Uncomment if you want to increase noise more frequently.

                        //if (executeLoop && (gridrank+1)%18 == randomProcessor)
                        if (timeStep%2 == 0 && timeStep != 0 && (((gridrank+1)%18) == randomProcessor))
                        {
                            for (int j = 0; j < 50000000; j++) {
                                x = 0.5/1000000000;
                                sum = sum + 4.0 / (1.0 + x * x);
                            }

                            if (sum < 0) std::cout << "sum" << sum << std::endl;
                        }


                        //int randomProcessor = dist(gen);
                            
                        //if (timeStep%4 == 0 && timeStep != 0 && (((gridrank+1)%18) == randomProcessor)) {
                            //std::cout << "Executing loop at iteration: " << timeStep << ", Random processor: " << randomProcessor << std::endl;
                            //for ( int rank = 0; rank < gridNTasks; rank ++ ) { 
                                //int steps = 500000 + ( 10000 * ((gridrank + 1) % 9));
                                //for (int j = 0; j < 50000000; j++) {
                                    //x = 0.5/1000000000;
                                    //sum = sum + 4.0 / (1.0 + x * x);
                                //}
                            //}
                            
                                
                            //if (sum < 0) std::cout << "sum" << sum << std::endl;
                        //}
#endif

#ifdef USE_EXTRAWORKPI

                     
                     if (std::fstream{filename}) {
                        //if( gridrank == 0 && timeStep == 0)
                            //std::cout << "taking fine-grained delay values from binary file" << std::endl;
                        /// an extra work of number of workIterations for all processores and timestep greater than and equal to extraItTimeStep iteration
                        sum = 0.0;
                        const realT stepLength1 = 1./44245260;
                        std::size_t extraItTimeStep =extraItTimeStep1;
                        for (std::size_t i=0 ; i < gridNTasks ; ++i){
                            if ((timeStep>=extraItTimeStep) && (timeStep<extraItTimeStep+bufsize) && (gridrank == i)) {
                                    for ( std::size_t j = 0; j < workIterations[timeStep-extraItTimeStep]; j++ ) {
                                    //if ((timeStep<bufsize) && (gridrank == i)) {
                                    //for ( std::size_t j = 0; j < workIterations[timeStep]; j++ ) {
                                    x = 0.5*stepLength1;
                                    sum = sum + 4.0 / ( 1.0 + x * x);
                                    }
                                    //std::cout << "Process " << gridrank << " delay extra " << workIterations[timeStep-extraItTimeStep] << " iterations/ " << buf[timeStep-extraItTimeStep] << " ns for time step " << timeStep << std::endl;
                                    if (sum < 0) std::cout << "sum" << sum <<std::endl;
                                    //std::this_thread::sleep_for(std::chrono::nanoseconds((int&)buf));
                                    }
                                    }
                                    }


                    if (std::fstream{filename}) {
                        //if( gridrank == 0 && timeStep == 0)
                            //std::cout << "taking fine-grained delay values from binary file" << std::endl;
                        /// an extra work of number of workIterations for all processores and timestep greater than and equal to extraItTimeStep iteration
                        sum = 0.0;
                        const realT stepLength1 = 1./44245260;
                        std::size_t extraItTimeStep1 =extraItTimeStep2;
                        for (std::size_t i=0 ; i < gridNTasks ; ++i){
                            if ((timeStep>=extraItTimeStep1) && (timeStep<extraItTimeStep1+bufsize) && (gridrank == i)) {
                                    for ( std::size_t j = 0; j < workIterations[timeStep-extraItTimeStep1]; j++ ) {
                                    //if ((timeStep<bufsize) && (gridrank == i)) {
                                    //for ( std::size_t j = 0; j < workIterations[timeStep]; j++ ) {
                                    x = 0.5*stepLength1;
                                    sum = sum + 4.0 / ( 1.0 + x * x);
                                    }
                                    //std::cout << "Process " << gridrank << " delay extra " << workIterations[timeStep-extraItTimeStep1] << " iterations/ " << buf[timeStep-extraItTimeStep1] << " ns for time step " << timeStep << std::endl;
                                    if (sum < 0) std::cout << "sum" << sum <<std::endl;
                                    //std::this_thread::sleep_for(std::chrono::nanoseconds((int&)buf));
                                    }
                                    }
                                    }


                    if (std::fstream{filename}) {
                        //if( gridrank == 0 && timeStep == 0)
                            //std::cout << "taking fine-grained delay values from binary file" << std::endl;
                        /// an extra work of number of workIterations for all processores and timestep greater than and equal to extraItTimeStep iteration
                        sum = 0.0;
                        const realT stepLength1 = 1./44245260;
                        std::size_t extraItTimeStep2 =extraItTimeStep3;
                        for (std::size_t i=0 ; i < gridNTasks ; ++i){
                            if ((timeStep>=extraItTimeStep2) && (timeStep<extraItTimeStep2+bufsize) && (gridrank == i)) {
                                    for ( std::size_t j = 0; j < workIterations[timeStep-extraItTimeStep2]; j++ ) {
                                    //if ((timeStep<bufsize) && (gridrank == i)) {
                                    //for ( std::size_t j = 0; j < workIterations[timeStep]; j++ ) {
                                    x = 0.5*stepLength1;
                                    sum = sum + 4.0 / ( 1.0 + x * x);
                                    }
                                    //std::cout << "Process " << gridrank << " delay extra " << workIterations[timeStep-extraItTimeStep2] << " iterations/ " << buf[timeStep-extraItTimeStep2] << " ns for time step " << timeStep << std::endl;
                                    if (sum < 0) std::cout << "sum" << sum <<std::endl;
                                    //std::this_thread::sleep_for(std::chrono::nanoseconds((int&)buf));
                                    }
                                    }
                                    }
#endif
#endif



								//if (timeStep==0) mpiErrorCheck(MPI_Barrier( comm_cart ));
#ifdef USE_LIKWID
								LIKWID_MARKER_STOP("Case:Computation");
#else
							}
							for (std::size_t i=0 ; i < gridNTasks ; ++i){
								if (gridrank == i) {
									maint2 = high_resolution_clock::now();
									duration<double> CompDuration = duration_cast<duration<double>>(maint2 - maint1);
									CompDuration /= static_cast<double>(nCppTimeRep);
									aggregate_time1 += CompDuration.count();
								}
							}

							/*mpiErrorCheck(MPI_Barrier( comm_cart ));
							  for (std::size_t i=0 ; i < gridNTasks ; ++i){
							  if (gridrank == i)
							  TR.add_loclop(i, 0, CompDuration.count(), 0);
							  mpiErrorCheck(MPI_Barrier( comm_cart ));
							  }

							/// adding time rank information of computation phase to output file
							if (timeStep == 0) TR.add_loclop(gridrank, 0, CompDuration.count()*1000000000, 0);
							else TR.add_loclop(gridrank, aggregate_time3*1000000000, (aggregate_time1+aggregate_time2)*1000000000, 0); */
#endif

#ifdef B_COLLECTIVE_REDUCE
							if (timeStep!=collectiveOPTimeStep){
#endif

#ifdef B_COLLECTIVE_SENDRECV
								if (timeStep!=collectiveOPTimeStep){
#endif

									//mpiErrorCheck(MPI_Barrier( comm_cart ));
#ifdef USE_LIKWID
									//LIKWID_MARKER_START("Case:PairwiseSendrecv");
#else
									for (std::size_t i=0 ; i < gridNTasks ; ++i){
										if (gridrank == i) maint3 = high_resolution_clock::now();
									}
									for (std::size_t timing_repetitions = 0; timing_repetitions < nCppTimeRep; ++timing_repetitions)
									{
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

										if( gridrank == 0 && timeStep == 0) std::cout << "\nCommunication Pattern: Pairwise blocking send and receive operations" <<std::endl;

										for ( std::size_t i = 0; i < nDims; i++ )
										{
											for ( std::size_t j = 0; j < nDirs; j++ ){
												mpiErrorCheck(MPI_Sendrecv( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank[j][i], j, &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank[j][i], j, comm_cart, MPI_STATUS_IGNORE ));
											}
										}

#endif

										////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
										// CASE 1 & 2: Data exchange with non-blocking sendrecv messages b/w max. 2N neighbours (ND domain decompisition) //
										////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
										MPI_Request req[2*nDims];
										MPI_Status  stt[2*nDims];

										/// CASE 2
#ifdef NB_ISENDIRECV

										//if( gridrank == 0 && timeStep == 0) std::cout << "\nCommunication Pattern: Pairwise non-blocking send and receive operations" <<std::endl;

                                        for ( std::size_t i = 0; i < nDims; i++ )
										{
											for ( std::size_t j = 0; j < nDirs; j++ )
											{
												mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank[j][i], 8, comm_cart, &req[0] ));
												mpiErrorCheck(MPI_Irecv( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank[j][i], 8, comm_cart, &req[1] ));
												mpiErrorCheck(MPI_Waitall( 2, &req[0], &stt[0] ));
											}
										}
                                        //mpiErrorCheck(MPI_Barrier( comm_cart ));


                                        auto stopReps = std::chrono::high_resolution_clock::now();
                                        std::chrono::duration<double> duration = stopReps - startReps;
                                        double timePerReps = duration.count();

                                        for (std::size_t rank=0; rank < gridNTasks; rank++) {
                                            if (gridrank == rank)
                                                std::cout << gridrank << "," << timeStep << "," << step_triad_time << "," << step_pi_time << "," << timePerReps << "," << 1.0/timePerReps << std::endl;
                                        }

										/// for indirect neighbour communication in parallel
										/*for ( std::size_t i = 0; i < nDims; i++ )
										  {
										  for ( std::size_t j = 0; j < nDirs; j++ )
										  {
										  mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank1[j][i], 0, comm_cart, &req[0] ));
										  mpiErrorCheck(MPI_Irecv( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank1[j][i], 0, comm_cart, &req[1] ));
										  mpiErrorCheck(MPI_Waitall( 2, &req[0], &stt[0] ));
										  }
										  }*/
#endif

										/// CASE 1
#ifdef NB_ISENDRECV

										//if( gridrank == 0 && timeStep == 0) std::cout << "\nCommunication Pattern: Pairwise non-blocking send and blocking receive operations" <<std::endl;

                                        for ( std::size_t i = 0; i < nDims; i++ )
										{
											for ( std::size_t j = 0; j < nDirs; j++ )
											{   
												mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank[j][i], 0, comm_cart, &req[0] ));
											    mpiErrorCheck(MPI_Recv ( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank[j][i], 0, comm_cart, MPI_STATUS_IGNORE ));
												//mpiErrorCheck(MPI_Barrier( comm_cart ));
											}
										}

                                        auto stopReps = std::chrono::high_resolution_clock::now();
                                        std::chrono::duration<double> duration = stopReps - startReps;
                                        double timePerReps = duration.count();

                                        for (std::size_t rank=0; rank < gridNTasks; rank++) {
                                            if (gridrank == rank)
                                                std::cout << gridrank << "," << timeStep << "," << step_triad_time << "," << step_pi_time << "," << timePerReps << "," << 1.0/timePerReps << std::endl;
                                        }
#endif

										
                                        //for (std::size_t rank=0; rank < gridNTasks; rank++){
                                            //if (gridrank == rank)
                                                //std::cout << gridrank << "," << timeStep << "," << triad_time << "," << pi_time << "," << triad_time/(triad_time+pi_time) << std::endl;
                                        //}
                                        
                                        //mpiErrorCheck(MPI_Barrier( comm_cart ));
#ifdef USE_LIKWID
										//LIKWID_MARKER_STOP("Case:PairwiseSendrecv");

#else
									
                                    }
                                    
									for (std::size_t i=0 ; i < gridNTasks ; ++i){
										if (gridrank == i){ maint4 = high_resolution_clock::now();
											duration<timeT> SendrecvDuration = duration_cast<duration<timeT>>(maint4 - maint3);
											SendrecvDuration /= static_cast<double>(nCppTimeRep);
											aggregate_time2 += SendrecvDuration.count();
											duration<timeT> CompSendrecvDuration = duration_cast<duration<timeT>>(maint4 - maint1);
											CompSendrecvDuration /= static_cast<double>(nCppTimeRep);
											aggregate_time3 += CompSendrecvDuration.count();
											if( (timeStep <= 100) && ((timeStep==1) || ((timeStep % 20) ==0)) )
												TR.time_vtkStep(gridrank, timeStep, aggregate_time3);
											else if( (timeStep < 1000) && ((timeStep % vtkStep) ==0))
												TR.time_vtkStep(gridrank, timeStep, aggregate_time3);
											else if ( (timeStep < 10000) && ((timeStep % 1000) ==0))
												TR.time_vtkStep(gridrank, timeStep, aggregate_time3);
											else if ( (timeStep < 100000) && ((timeStep % 10000) ==0))
												TR.time_vtkStep(gridrank, timeStep, aggregate_time3);
											else if ( (timeStep >= 100000) && ((timeStep % 100000) ==0))
												TR.time_vtkStep(gridrank, timeStep, aggregate_time3);
										}
									}


									/**
									 * \brief adding time rank information of communication phase to output file
									 */
									/*for(std::size_t i = 0; i < nDims; i++ ){
									  for ( std::size_t j = 0; j < 2; j++ ){
									  TR.add_transmission( srcRank[j][i], dstRank[j][i], (aggregate_time1)*1000000000, (aggregate_time1+aggregate_time2)*1000000000, nTasks);
									  }
									  }*/
#endif

									//////////////////////////////////////////////////////////////////////////////////////////////////////////
									//    Data exchange with blocking collective messages b/w max. 2N neighbours (ND domain decompisition)  //
									//////////////////////////////////////////////////////////////////////////////////////////////////////////

									/// CASE 4
#ifdef B_COLLECTIVE_SENDRECV
								}
								realT result=0.0;
								if (timeStep==collectiveOPTimeStep){
									if( gridrank == 0) std::cout << "\nCommunication Pattern for time step iteration " << timeStep << ": Collective blocking send and receive operation\n" << std::endl;

									//mpiErrorCheck(MPI_Barrier( comm_cart ));
#ifdef USE_LIKWID
									//LIKWID_MARKER_START("Case:BCollectiveSendRecv");
#else
									high_resolution_clock::time_point maint5 = high_resolution_clock::now();
									for (std::size_t timing_repetitions = 0; timing_repetitions < nCppTimeRep; ++timing_repetitions)
									{
#endif

										/// sum up all "sum" into result
										if(gridrank != 0){
											mpiErrorCheck(MPI_Send(&sum, 1, MPI_REAL_TYPE, 0, 1, MPI_COMM_WORLD));
										}else{
											MPI_Status status;
											result=sum;
											for(int i=1;i<gridNTasks;++i){
												mpiErrorCheck(MPI_Recv(&sum, 1, MPI_REAL_TYPE, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD,&status));
												result+=sum;
											}
										}

										// mpiErrorCheck(MPI_Barrier( comm_cart ));
#ifdef USE_LIKWID
										//LIKWID_MARKER_STOP("Case:BCollectiveSendRecv");
#else
									}
									high_resolution_clock::time_point maint6 = high_resolution_clock::now();
									duration<timeT> BReduceDuration = duration_cast<duration<timeT>>(maint6 - maint5);
									BReduceDuration/= static_cast<double>(nCppTimeRep);
									aggregate_time2 += BReduceDuration.count();
									duration<timeT> CompBReduceDuration = duration_cast<duration<timeT>>(maint6 - maint1);
									CompBReduceDuration /= static_cast<double>(nCppTimeRep);
									aggregate_time3 += CompBReduceDuration.count();

#ifdef USE_PISOLVER
									/// calculate and print pi for the iteration where collective operation happens
									if( gridrank == 0 )
										std::cout << "PI = " << result * stepLength << " for time step iteration " << timeStep <<" with error compare to the Pi in 'math.h': " << fabs((result * stepLength) - PiExact) << std::endl;
#endif

									/// adding time rank information of communication phase to output file
									/*
									    for(std::size_t i = 0; i < nDims; i++ ){
									    for ( std::size_t j = 0; j < nDirs ; j++ ){
									    TR.add_transmission( srcRank[j][i], dstRank[j][i], SendrecvDuration.count()*1000000000, BReduceDuration.count()*1000000000, nTasks);
									    }
									    }*/

#endif
								}
#endif

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

							realT result=0.0;
							if (timeStep==collectiveOPTimeStep){
								if( gridrank == 0) std::cout << "\nCommunication Pattern for time step iteration " << timeStep << ": Collective blocking reduce operation\n" << std::endl;

								//mpiErrorCheck(MPI_Barrier( comm_cart ));
#ifdef USE_LIKWID
								//LIKWID_MARKER_START("Case:BCollectiveReduce");
#else
								high_resolution_clock::time_point maint5 = high_resolution_clock::now();
								for (std::size_t timing_repetitions = 0; timing_repetitions < nCppTimeRep; ++timing_repetitions)
								{
#endif

									for ( std::size_t i = 0; i < nDims; i++ )
									{
										for ( std::size_t j = 0; j < nDirs; j++ )
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
									//LIKWID_MARKER_STOP("Case:BCollectiveReduce");
#else
								}
								high_resolution_clock::time_point maint6 = high_resolution_clock::now();
								duration<timeT> BReduceDuration = duration_cast<duration<timeT>>(maint6 - maint5);
								BReduceDuration/= static_cast<double>(nCppTimeRep);
								aggregate_time2 += BReduceDuration.count();
								duration<timeT> CompBReduceDuration = duration_cast<duration<timeT>>(maint6 - maint1);
								CompBReduceDuration /= static_cast<double>(nCppTimeRep);
								aggregate_time3 += CompBReduceDuration.count();

#ifdef USE_PISOLVER
								/// calculate and print pi for the iteration where collective operation happens
								//if( gridrank == 0 )
									//std::cout << "PI = " << result * stepLength << " for time step iteration " << timeStep <<" with error compare to the Pi in 'math.h': " << fabs((result * stepLength) - PiExact) << std::endl;
#endif

								/// adding time rank information of communication phase to output file
								/*
								   for(std::size_t i = 0; i < nDims; i++ ){
								   for ( std::size_t j = 0; j < nDirs ; j++ ){
								   TR.add_transmission( srcRank[j][i], dstRank[j][i], SendrecvDuration.count()*1000000000, BReduceDuration.count()*1000000000, nTasks);
								   }
								   }*/
#endif
							}
#endif

							} //!< MAINtimeStep

                            //MPI_Reduce(&triad_time, &max_triad_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

                            //if (gridrank == 0)
                                //std::cout << gridNTasks << "," << max_triad_time << "," << (2.0 * arrayElements * gridNTasks * nTimeStep)/(max_triad_time * 1000.0 * 1000.0)  << std::endl;

                            //for (std::size_t i=0; i < gridNTasks; i++)
                            //{
                                //if(gridrank == i)
                                    //std::cout << gridrank << "," << triad_time << std::endl;
                            //}

							/**
							 *\brief print the final time
							 */
#ifndef USE_LIKWID
							//if( gridrank == 0 ) {
								//std::cout << "\nComputation phase for " << aggregate_time1 * 1000 << " ms" << std::endl;
								//std::cout << "Pairwise communication phase time without computation for " << length << " doubles: " << aggregate_time2 * 1000 << " ms" << std::endl;
								//std::cout << "Total Pairwise communication phase time with computation for " << length << " doubles: " << aggregate_time3 * 1000 << " ms" <<    std::endl;
								//std::cout << "Collective communication phase time without computation for " << length << " doubles: " << aggregate_time4 * 1000 << " ms" <<    std::endl;
								//std::cout << "Total Collective communication phase time with computation for " << length << " doubles: " << aggregate_time5 * 1000 << " ms" <<    std::endl;
								//std::cout << "\nFor detailed time-rank traces, see 'timeRankViz.cvs' file \n " <<    std::endl;
								//std::cout << " totaltime "<< aggregate_time1 * 1000 << " " << aggregate_time2 * 1000  << " " << aggregate_time3 * 1000 << std::endl;
							//}
#endif

							/**
							 *\brief free resources (marks the MPI communicator objects for deallocation)
							 */
							try {
								for(std::size_t x = 0; x < nDims; ++x){
									for(std::size_t y = 0; y < nDirs; ++y){
										delete[] sendBuffer[x][y];
									}
									delete[] sendBuffer[x];
								}
								delete[] sendBuffer;

								for(std::size_t x = 0; x < nDims; ++x){
									for(std::size_t y = 0; y < nDirs; ++y){
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

#if defined(USE_EXTRAWORK) || defined(USE_EXTRAWORKPI)
								free(buf);
#endif

								mpiErrorCheck(MPI_Comm_free( &comm_cart ));
							}
							catch ( std::exception &e){
								std::cerr << "error on deallocating resouces: " << e.what() << std::endl;
								return (EXIT_FAILURE);
							}
						} //!< IFLOOP
						else{
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

#ifdef USE_PISOLVER
					void PISOLVER(realT sum, realT x, const long nSteps, int gridNTasks, int gridrank, const realT stepLength){
						//////////////////////////////////////////////////////////////////////////////////////////////////////////
						//                 MPI parallelized Pi solver           //
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
						sum = 0.0;
						std::size_t tilesize = nSteps / gridNTasks;
#ifdef USE_HYBRID_OPENMP
#pragma omp parallel for private(x) reduction(+:sum) schedule(runtime)
#endif
						for (std::size_t i = gridrank * tilesize; i < gridrank * tilesize + tilesize; i++){
							x = (i+0.5)*stepLength;
							sum = sum + 4.0 / ( 1.0 + x * x);
						}
						if (sum < 0) std::cout << "sum" << sum <<std::endl;
					}
#endif
#ifdef USE_TRIAD
					void TRIAD(std::size_t arrayElements, triadT *A, triadT *B, triadT *C, double scalar){
#pragma vector nontemporal
#ifdef USE_HYBRID_OPENMP
#pragma omp parallel for
#endif
						for (std::size_t i=0; i<arrayElements; i++)
							A[i] = B[i]+scalar*C[i];
						if (A[arrayElements/2] < 0) std::cout << "A" << A <<std::endl;
					}
#endif
