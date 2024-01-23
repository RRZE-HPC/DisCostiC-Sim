#include <iostream>
#include <list>
#include <algorithm>
#include <cmath>
#include <mpi.h>

#include "../include/ConfigParser.hpp"
#include "../include/NetworkConfigParser.hpp"
#include "../include/YAMLParser.hpp"
#include "../visualization/TimeRankOP.hpp"
#include "../include/GridInit.hpp"
#include <mutex>

// #define USE_VERBOSE
// #define USE_DEBUG
#define USE_CHROMEVIZ
// #define USE_DRAWVIZ

enum communication_mode
{
    LOGGP = 0,
    SIMPLELB = 1
};

enum communication_type
{
    INTRACHIP = 0,
    INTERCHIP = 1,
    INTERNODE = 2,
    INTERCLUSTER = 3
};

enum time
{
    START = 0,
    END = 1
};

enum bound_type
{
    COMPUTE = 0,
    MEMORY = 1
};

extern int bound;
extern int scaling_cores;
extern int bytes_to_send;

// Global variables for setting individual system properties
int virtual_rank = 0;
int system_number = 0;
int task_per_node = 0;
int node = 0;
int cc_numa_domain_per_socket = 0;
int cores_per_socket = 0;
int cc_numa_domain = 0;
int socket = 0;
int primary_processes = 0;
int secondary_processes = 0;
int heteregeneous_mode = 0;
std::string arch_name = "";

DisCosTiC::DisCosTiC_OP copy(DisCosTiC::DisCosTiC_OP op_og, DisCosTiC::DisCosTiC_OP op_copy)
{
    op_copy.time = op_og.time;
    op_copy.starttime = op_og.starttime;
    op_copy.syncstart = op_og.syncstart;
    op_copy.numOpsInQueue = op_og.numOpsInQueue;
    op_copy.bufSize = op_og.bufSize;
    op_copy.target = op_og.target;
    op_copy.rank = op_og.rank;
    op_copy.label = op_og.label;
    op_copy.tag = op_og.tag;
    op_copy.node = op_og.node;
    op_copy.network = op_og.network;
    op_copy.type = op_og.type;
    return op_copy;
}

// function to convert operation objects computationally relavent
double *finalize(double a, double b, double c, double d, double e, double arr[5])
{
    arr[0] = a;
    arr[1] = b;
    arr[2] = c;
    arr[3] = d;
    arr[4] = e;

    return arr;
}

int main(int argc, char **argv)
{
    if (argc > 1)
    {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
        {
            help();
            exit(0);
        }
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0)
        {
            version();
            exit(0);
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                            Initailize MPI environment                                                 //
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    int ierr, process_Rank, size_Of_Cluster, sucesss, N_process_Rank, N_size_Of_Cluster;
    MPI_Comm newcomm;
    ierr = MPI_Init(NULL, NULL);
    ierr = MPI_Comm_rank(MPI_COMM_WORLD, &process_Rank);

    MPI_Comm_split(MPI_COMM_WORLD, (process_Rank) < 1 ? 1 : 2, 1, &newcomm);
    ierr = MPI_Comm_rank(newcomm, &N_process_Rank);
    ierr = MPI_Comm_size(newcomm, &N_size_Of_Cluster);
    ierr = MPI_Comm_rank(MPI_COMM_WORLD, &process_Rank);
    ierr = MPI_Comm_size(MPI_COMM_WORLD, &size_Of_Cluster);
    std::queue<int> test;
    MPI_Status status;
    MPI_Request request;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                            Initailize config and YAML args                                           //
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    UserInterface::ConfigParser CFG_args("config.cfg");

    heteregeneous_mode = CFG_args.getValue<DisCosTiC_Datatype>("heteregeneous");

    primary_processes = CFG_args.getValue<DisCosTiC_Datatype>("number_of_processes");
    secondary_processes = CFG_args.getValue<DisCosTiC_Datatype>("secondary_number_of_processes");

    system_number = N_process_Rank < primary_processes ? 0 : 1;

    virtual_rank = system_number == 0 ? N_process_Rank : N_process_Rank - primary_processes;

    arch_name = system_number == 0 ? CFG_args.getValue<std::string>("micro_architecture") : CFG_args.getValue<std::string>("secondary_micro_architecture");

    if (heteregeneous_mode == 0)
    {
        if (CFG_args.getValue<DisCosTiC_Datatype>("number_of_processes") + 1 != size_Of_Cluster)
        {
            std::cout << "Simulator processes != number_of_processes+1" << std::endl;
            MPI_Finalize();
            exit(0);
        }
    }
    else
    {
        if (CFG_args.getValue<DisCosTiC_Datatype>("number_of_processes") + CFG_args.getValue<DisCosTiC_Datatype>("secondary_number_of_processes") + 1 != size_Of_Cluster)
        {
            std::cout << "Simulator processes != number_of_processes+1" << std::endl;
            MPI_Finalize();
            exit(0);
        }
    }

    UserInterface::YAMLParser YAML_args("nodelevel/machine-files/" + arch_name + ".yml");

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                            Assigning system properties to individual rank                            //
    //////////////////////////////////////////////////////////////////////////////////////////////////////////

    task_per_node = system_number == 0 ? CFG_args.getValue<DisCosTiC_Datatype>("task_per_node") : CFG_args.getValue<DisCosTiC_Datatype>("secondary_task_per_node");
    node = std::floor(virtual_rank / task_per_node);
    cc_numa_domain_per_socket = YAML_args.cores_per_numa_domain;
    cores_per_socket = YAML_args.cores_per_chip;
    cc_numa_domain = std::floor(virtual_rank / cc_numa_domain_per_socket);
    socket = std::floor(virtual_rank / cores_per_socket);

    // sleep(1 * N_process_Rank);
    // std::cout
    //     << "Actual rank : " << N_process_Rank << std::endl
    //     << "\t Virtual Rank : " << virtual_rank << std::endl
    //     << "\t System Number : " << system_number << std::endl
    //     << "\t task_per_node : " << task_per_node << std::endl
    //     << "\t node : " << node << std::endl
    //     << "\t cores_per_socket : " << cores_per_socket << std::endl
    //     << "\t socket Number : " << socket << std::endl
    //     << "\t cc_numa_domain_per_socket : " << cc_numa_domain_per_socket << std::endl
    //     << "\t cc_numa_domain Number : " << cc_numa_domain << std::endl;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                            Initailize configuration constanqueueOp and binary input environment      //
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    DisCosTiC::Benchmark benchmark(&CFG_args);

    DisCosTiC::idNodeTypePair ID = benchmark.Oneway_PositiveDisplacement(process_Rank, YAML_args, size_Of_Cluster, MPI_COMM_WORLD); //!< generate AST for Oneway_PositiveDisplacement communication pattern example
    MPI_Barrier(MPI_COMM_WORLD);

#ifdef USE_VERBOSE
    std::cout << "Rank : " << process_Rank << " completed Oneway_PositiveDisplacement()" << std::endl;
#endif

    DisCosTiC::CompModel compModel(&CFG_args, YAML_args, process_Rank);

    DisCosTiC::Grid_Init AST_args(benchmark.systemsize, benchmark.numOperations, benchmark, ID); /// rootGrabber: get root nodes of  DisCosTiC::AST_OP_TYPE type (i.e., exec in our case ) of all ranks

    if (process_Rank == 0)
    {
        system(("rm config{0.." + std::to_string(size_Of_Cluster) + "}.cfg 2> /dev/null").c_str());
        system(("rm ./nodelevel/configs/" + CFG_args.getValue<std::string>("benchmark_kernel") + "{0.." + std::to_string(size_Of_Cluster) + "}.cfg 2> /dev/null").c_str());
    }

#ifdef USE_VERBOSE
    std::cout << "Rank : " << process_Rank << " completed AST generation()" << std::endl;
#endif

    DisCosTiC::Operations operations_queue;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                     Reading user interface configuration input file                  //
    //////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
     *\brief it parses and print user-defined parameters or args of configuration file (config.cfg).
     */

    UserInterface::NetworkConfigParser IntrachipCFG_args;
    UserInterface::NetworkConfigParser InterchipCFG_args;
    UserInterface::NetworkConfigParser InternodeCFG_args;
    UserInterface::NetworkConfigParser InterclusterCFG_args;

    if (heteregeneous_mode == 0)
    {

        IntrachipCFG_args.readData("./nodelevel/network-files/" + arch_name + "_intrachip.cfg");

        InterchipCFG_args.readData("./nodelevel/network-files/" + arch_name + "_interchip.cfg");

        InternodeCFG_args.readData("./nodelevel/network-files/" + arch_name + "_internode.cfg");

        InterclusterCFG_args.readData("./nodelevel/network-files/" + arch_name + "_internode.cfg");
    }
    else
    {

        IntrachipCFG_args.readData("./nodelevel/network-files/" + arch_name + "_waitio_" + CFG_args.getValue<std::string>("waitio_mode") + "_intrachip.cfg");

        InterchipCFG_args.readData("./nodelevel/network-files/" + arch_name + "_waitio_" + CFG_args.getValue<std::string>("waitio_mode") + "_interchip.cfg");

        InternodeCFG_args.readData("./nodelevel/network-files/" + arch_name + "_waitio_" + CFG_args.getValue<std::string>("waitio_mode") + "_internode.cfg");

        InterclusterCFG_args.readData("./nodelevel/network-files/" + arch_name + "_waitio_" + CFG_args.getValue<std::string>("waitio_mode") + "_intercluster.cfg");
    }

    IntrachipCFG_args.setData(bytes_to_send);
    InterchipCFG_args.setData(bytes_to_send);
    InternodeCFG_args.setData(bytes_to_send);
    InterclusterCFG_args.setData(bytes_to_send);

    const DisCosTiC_Timetype comm_model = CFG_args.getValue<DisCosTiC_Datatype>("comm_model");

    const DisCosTiC::Networktype latency{IntrachipCFG_args.getValue<DisCosTiC_Timetype>("latency_in_ns"),
                                         InterchipCFG_args.getValue<DisCosTiC_Timetype>("latency_in_ns"),
                                         InternodeCFG_args.getValue<DisCosTiC_Timetype>("latency_in_ns"),
                                         InterclusterCFG_args.getValue<DisCosTiC_Timetype>("latency_in_ns")};

    const DisCosTiC::Networktype G{IntrachipCFG_args.getValue<DisCosTiC_Timetype>("G_in_ns"),
                                   InterchipCFG_args.getValue<DisCosTiC_Timetype>("G_in_ns"),
                                   InternodeCFG_args.getValue<DisCosTiC_Timetype>("G_in_ns"),
                                   InterclusterCFG_args.getValue<DisCosTiC_Timetype>("G_in_ns")};

    const DisCosTiC::Networktype inverse_bandwidth{IntrachipCFG_args.getValue<DisCosTiC_Timetype>("inverse_bandwidth"),
                                                   InterchipCFG_args.getValue<DisCosTiC_Timetype>("inverse_bandwidth"),
                                                   InternodeCFG_args.getValue<DisCosTiC_Timetype>("inverse_bandwidth"),
                                                   InterclusterCFG_args.getValue<DisCosTiC_Timetype>("inverse_bandwidth")};

    const DisCosTiC::Networktype o{IntrachipCFG_args.getValue<DisCosTiC_Timetype>("comm_o_in_ns"),
                                   InterchipCFG_args.getValue<DisCosTiC_Timetype>("comm_o_in_ns"),
                                   InternodeCFG_args.getValue<DisCosTiC_Timetype>("comm_o_in_ns"),
                                   InterclusterCFG_args.getValue<DisCosTiC_Timetype>("comm_o_in_ns")};

    const DisCosTiC::Networktype O{IntrachipCFG_args.getValue<DisCosTiC_Timetype>("comm_O_in_ns"),
                                   InterchipCFG_args.getValue<DisCosTiC_Timetype>("comm_O_in_ns"),
                                   InternodeCFG_args.getValue<DisCosTiC_Timetype>("comm_O_in_ns"),
                                   InterclusterCFG_args.getValue<DisCosTiC_Timetype>("comm_O_in_ns")};

    const DisCosTiC::Networktype g{IntrachipCFG_args.getValue<DisCosTiC_Timetype>("comm_g_in_ns"),
                                   InterchipCFG_args.getValue<DisCosTiC_Timetype>("comm_g_in_ns"),
                                   InternodeCFG_args.getValue<DisCosTiC_Timetype>("comm_g_in_ns"),
                                   InterclusterCFG_args.getValue<DisCosTiC_Timetype>("comm_g_in_ns")};

    const DisCosTiC_Datatype eagerLimit = InternodeCFG_args.getValue<DisCosTiC_Datatype>("comm_eagerlimit_in_bytes");

    const DisCosTiC_Datatype nunmberofiter = CFG_args.getValue<DisCosTiC_Datatype>("number_of_timesteps");

    const DisCosTiC_Datatype desync_feature = CFG_args.getValue<DisCosTiC_Datatype>("desync");
    const DisCosTiC_Datatype idlewave_feature = CFG_args.getValue<DisCosTiC_Datatype>("idlewave");
    const DisCosTiC_Datatype desync_rank = CFG_args.getValue<DisCosTiC_Datatype>("which_rank");
    const DisCosTiC_Datatype desync_timestep = CFG_args.getValue<DisCosTiC_Datatype>("at_timestep");

#ifdef USE_VERBOSE
    std::cout << "Rank : " << process_Rank << " completed network configuration" << std::endl;
#endif

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                     Reading generated AST data                                       //
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    const DisCosTiC_Indextype numRanks = AST_args.graphVec.size();

    const DisCosTiC_Indextype numCores = benchmark.GetNumCores();
    const DisCosTiC_Indextype numNetworks = benchmark.GetNumNetworks();
    std::vector<real_t> hosqueueOpync(numRanks); //!< number of synchronization induced by MPI routines

    if (process_Rank == 0)
    {
        std::cout << "\n----------------------------------------------------------------" << std::endl;
        std::cout << "DisCosTiC (Distributed CosT in Cluster):" << std::endl;
        std::cout << "----------------------------------------------------------------" << std::endl;
        std::cout << "Version: v1.0" << std::endl; //!< print version number of toolkit
        auto timestamp = system_clock::to_time_t(system_clock::now());
        std::cout << "timestamp: " << ctime(&timestamp); //!< print current date and time
        std::cout << "\n\n----------------------------------------------------------------" << std::endl;
        std::cout << "GENERATED DEPENDENCY GRAPH (Directed Acyclic Benchmark (DisCosTiC)):" << std::endl;
        std::cout << "----------------------------------------------------------------" << std::endl;
        std::cout << "number of timesteps: " << nunmberofiter << std::endl;
        std::cout << "OneWay_PosDist communetworkation pattern: number of Operation_t (send or recv or exec) per timestep per rank: " << 1 << std::endl;
        std::cout << "\n\n----------------------------------------------------------------" << std::endl;
        std::cout << "COMMUNICATION PERFORMANCE MODEL (Network+Systemtopology):" << std::endl;
        std::cout << "----------------------------------------------------------------" << std::endl;
        std::cout << "number of processes: " << numRanks << std::endl;
        std::cout << "number of chips: " << numCores << std::endl;
        std::cout << "SIMPLE LATENCY-BANDWIDTH MODEL:: intra chip latency [ns]: " << latency[INTRACHIP]
                  << ", inter chip latency [ns]: " << latency[INTERCHIP]
                  << ", inter node latency [ns]: " << latency[INTERNODE]
                  << ", inter cluster latency [ns]: " << latency[INTERCLUSTER]
                  << ", intra chip inverse bandwidth [s/GB]: " << inverse_bandwidth[INTRACHIP]
                  << ", inter chip inverse bandwidth [s/GB]: " << inverse_bandwidth[INTERCHIP]
                  << ", inter node inverse bandwidth [s/GB]: " << inverse_bandwidth[INTERNODE]
                  << ", inter cluster inverse bandwidth [s/GB]: " << inverse_bandwidth[INTERCLUSTER] << std::endl;
        std::cout << "LogGP MODEL additional parameters (Intra chip) :: o [ns]: " << o[INTRACHIP] << ", g [ns]: " << g[INTRACHIP] << ", O [ns]: " << O[INTRACHIP] << std::endl;
        std::cout << "LogGP MODEL additional parameters (Inter chip) :: o [ns]: " << o[INTERCHIP] << ", g [ns]: " << g[INTERCHIP] << ", O [ns]: " << O[INTERCHIP] << std::endl;
        std::cout << "LogGP MODEL additional parameters (Inter node) :: o [ns]: " << o[INTERNODE] << ", g [ns]: " << g[INTERNODE] << ", O [ns]: " << O[INTERNODE] << std::endl;
        std::cout << "LogGP MODEL additional parameters (Inter cluster) :: o [ns]: " << o[INTERCLUSTER] << ", g [ns]: " << g[INTERCLUSTER] << ", O [ns]: " << O[INTERCLUSTER] << std::endl;

        std::cout << "Eager limit: " << eagerLimit << " bytes" << std::endl;
        std::cout << "num_synchronizations: " << hosqueueOpync.size() << std::endl;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                   Initailize user interface (time-rank output) environment and other variables       //
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    UserInterface::TimeRankOP traceVisualizer(&CFG_args, N_process_Rank, numRanks);
    UserInterface::ChromeTraceViz traceVisualizer2(&CFG_args, process_Rank, numRanks);

    /// define and initialize next available time of computation, receive and send for all CPU nodes and ranks
    Timevec2T oSuccessor(numRanks), grecvSuccessor(numRanks), gsendSuccessor(numRanks);
    for (auto rank : DisCosTiC::getRange(numRanks))
    {
        oSuccessor[rank].resize(numCores);
        grecvSuccessor[rank].resize(numNetworks);
        gsendSuccessor[rank].resize(numNetworks);
        std::fill(oSuccessor[rank].begin(), oSuccessor[rank].end(), 0);
        std::fill(grecvSuccessor[rank].begin(), grecvSuccessor[rank].end(), 0);
        std::fill(gsendSuccessor[rank].begin(), gsendSuccessor[rank].end(), 0);
    }

    DisCosTiC_Indextype numOpsInQueue = 0; //  ORDER: keep order between comm & comp operations for non-blocking routines
    DisCosTiC_Indextype rank = 0;
    DisCosTiC_Indextype DisCosTiCOps = 0;
    DisCosTiC_Indextype DisCosTiCProgress = 0;
    DisCosTiC_Indextype computingProcessesCount = 0;
    DisCosTiC_Timetype COMPCount = 0;
    DisCosTiC_Timetype timeOld = 0;
    DisCosTiC_Indextype flow_id = 0;

    bool newOps = true;
    vec1T finishedRankList;
    vec1T CompRankList;
    vec1T new_CompRankList;
    DisCosTiC::OpMatcher M;
    DisCosTiC::PriorityQueue_t queue;
    DisCosTiC::VecListqueueOp recvQueue(numRanks), UMQ(numRanks); // unexpected message queue
    DisCosTiC_Datatype execTime;                                  //  execution run time of computational kernel in micro seconds
                                                                  // std::vector<DisCosTiC_Timetype> buffersize = {3000 , 3200 , 3400 , 4100 , 3700 , 5900 , 5000 , 6000 , 3800 , 3560, 3789 , 3500 , 3450 , 3210 , 3100 , 2300 , 3000 , 4000,  3100 , 2300};
    static bool initialized;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                     /**  \brief Starting Time   */                                   //
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    high_resolution_clock::time_point startTime, endTime;
    startTime = high_resolution_clock::now();
    DisCosTiC_Timetype starttimeProcess;
    srand(time(NULL) + process_Rank + 5);

    /**
     * \brief initialize root operations for each rank (only once per run!).
     * These sorted operations order by type retrieve from test case application
     * added to the queue (that has operations of all processes
     * and their earliest start times). (each operation = time, rank, next operation).
     */

    DisCosTiC::DisCosTiC_OP operation;
    DisCosTiC::DisCosTiC_OP operation_og;
    int iter = 0;
    /**
     * \brief Start of parallelization code
     * runs for all processes that are not the master rank
     */
    if (process_Rank != 0)
    {

        for (auto rank : DisCosTiC::getRange(numRanks))
        {                                                            // loop over rank
            DisCosTiC::Grid *rankLocalIt = &AST_args.graphVec[rank]; // keep iterating over the element till you find all rank and reach the end of AST_args.graphVec vector
            DisCosTiC::Operations operations;
            DisCosTiCOps += rankLocalIt->getNumOps(); // rankLocalIt->getNumOps()=15 (x18) , rankLocalIt->num_ranks_in_schedule=18 (x18) DisCosTiCOps: 15, 30, 45, ..., 270 (x18)
            rankLocalIt->getSortedRootOps(operations);

            iter = nunmberofiter;
            for (auto &op : operations)
            {
                if (rank == N_process_Rank)
                {
                    operation_og = copy(op, operation_og);
                    operation = op;
                    starttimeProcess = operation.starttime;
                    // print_OpPropertiesNonPointerT(op);
                }
#ifdef USE_VERBOSE
                switch (op.type)
                {
                case DisCosTiC::Operation_t::COMP:
                    verboseCompInitPrint(rank, op);
                    break;
                case DisCosTiC::Operation_t::SEND:
                    verboseSendInitPrint(rank, op);
                    break;
                case DisCosTiC::Operation_t::RECV:
                    verboseRecvInitPrint(rank, op);
                    break;
                default:
                    std::cerr << "YET NOT IMPLEMENTED!" << std::endl;
                }
#endif
                execTime = op.bufSize;
            }
        }

        MPI_Barrier(newcomm);

#ifdef USE_VERBOSE
        std::cout << "Rank : " << N_process_Rank << " starting simulation" << std::endl;
#endif

        /** \brief the switch on type of each operation:
         * whenever a blocking or non-blocking dependency or something is satisfied,
         * the rank is added to the list of ranks that completed someting (finishedRankList)
         */

        rank = N_process_Rank;

        // int active_task_per_node = (node == N_size_Of_Cluster / task_per_node ? N_size_Of_Cluster % task_per_node : task_per_node);

        int commTime = 0;

        double nonoverlap = 0;
        // std::cout << "Rank : " << rank << " node : " << node << " scaling cores : " << scaling_cores << " active_task_per_node : " << active_task_per_node << std::endl;

        bool cycle = true;
        int iter_num = 0;

        double gather_buff_comp[2][N_size_Of_Cluster];
        double gather_buff_optime[N_size_Of_Cluster];
        double overlap_time[N_size_Of_Cluster];
        long int last_idle_time = 0, max_idle_time = 0;

        for (int counter = 0; counter < N_size_Of_Cluster; ++counter)
        {
            gather_buff_optime[counter] = 0;
        }

#ifdef USE_VERBOSE
        std::cout << "Rank : " << N_process_Rank << " properties allocated" << std::endl;
#endif

        //////////////////////////////////////////////////////////////////////////////////////////////////////////
        //                            Simulation                                                 //
        //////////////////////////////////////////////////////////////////////////////////////////////////////////
        do
        { // Cycle for the number of iterations
            do
            { // Cycle for the number of steps per iteration

                if ((timeOld - operation.time) != 0) //}// later: coresPerChip-1
                {
                    /// operations of all ranks with their starting times and counter over operation statuses
                    COMPCount = computingProcessesCount;

                    new_CompRankList = CompRankList;
                    // print_vec1T(new_CompRankList);

                    timeOld = operation.time;
                }

                // print_vec1T(new_CompRankList);

                std::vector<DisCosTiC_Timetype> buffersize(20, execTime);

                auto it = std::find(new_CompRankList.begin(), new_CompRankList.end(), operation.rank);

                if ((operation.time != 0) && (it != new_CompRankList.end()))
                {
                    if (COMPCount == 0)
                    {
                        operation.time = operation.time - execTime; //!< get rid of baseline comp time
                    }
                    else
                    {
                        operation.time = operation.time - execTime + buffersize[COMPCount - 1]; //!< get rid of baseline comp time and add original computing time modeled by analytical roofline model
                    }
                }

                MPI_Allgather(&operation.time, 1, MPI_DOUBLE, gather_buff_optime, 1, MPI_DOUBLE, newcomm);
#ifdef USE_VERBOSE
                std::cout << "Rank : " << N_process_Rank << " in iter : " << iter_num << " MPI_Allgather completed" << std::endl;
#endif

                switch (operation.type)
                {

                case DisCosTiC::Operation_t::COMP:
                {

#ifdef USE_VERBOSE
                    std::cout << "Rank : " << N_process_Rank << " in iter : " << iter_num << " in COMP" << std::endl;
                    verboseCompPrint(operation);
#endif

                    computingProcessesCount++;
                    CompRankList.push_back(operation.rank);

                    if (oSuccessor[operation.rank][operation.node] <= operation.time)
                    {

                        oSuccessor[operation.rank][operation.node] = operation.time + operation.bufSize;

                        // Gathering data for Bandwidth contention model
                        last_idle_time = operation.time - last_idle_time;

                        MPI_Allgather(&operation.time, 1, MPI_DOUBLE, gather_buff_comp[0], 1, MPI_DOUBLE, newcomm);
                        MPI_Allgather(&oSuccessor[operation.rank][operation.node], 1, MPI_DOUBLE, gather_buff_comp[1], 1, MPI_DOUBLE, newcomm);

#ifdef USE_VERBOSE
                        std::cout << "Rank : " << N_process_Rank << " in iter : " << iter_num << " gathered data for BCM" << std::endl;
#endif

                        // Logic to handle overlapping and memory bandwidth

                        int startComputingProcess = 0, endComputingProcess = 0, overlappComputingProcess = 0;
                        if ((bound == MEMORY) && scaling_cores != -1)
                        {
                            // std::cout << "Timestep : " << iter_num << " "<<operation.bufSize <<std::endl;
                            overlap_time[rank] = operation.bufSize;

                            for (int loop = 0; loop < N_size_Of_Cluster; ++loop)
                            {

                                int loop_system = loop < primary_processes ? 0 : 1;

                                int loop_node = loop < primary_processes ? std::floor(loop / task_per_node) : std::floor(loop - primary_processes / task_per_node);
                                int loop_cc_numa_domain = loop < primary_processes ? std::floor(loop / cc_numa_domain_per_socket) : std::floor(loop - primary_processes / cc_numa_domain_per_socket);
                                int loop_socket = loop < primary_processes ? std::floor(loop / cores_per_socket) : std::floor(loop - primary_processes / cores_per_socket);

                                if (loop != rank && (loop_node == node) && (loop_cc_numa_domain == cc_numa_domain) && (loop_socket == socket) && (loop_system == system_number))
                                {
                                    if ((gather_buff_comp[START][rank] <= gather_buff_comp[START][loop] && gather_buff_comp[END][rank] >= gather_buff_comp[START][loop]) || (gather_buff_comp[START][rank] <= gather_buff_comp[END][loop] && gather_buff_comp[END][rank] >= gather_buff_comp[END][loop]))
                                    {
                                        if (gather_buff_comp[START][rank] <= gather_buff_comp[START][loop] && gather_buff_comp[END][rank] >= gather_buff_comp[START][loop])
                                        {
                                            overlap_time[loop] = gather_buff_comp[END][rank] - gather_buff_comp[START][loop] < 0 ? 0 : gather_buff_comp[END][rank] - gather_buff_comp[START][loop];
                                        }
                                        else if (gather_buff_comp[START][rank] <= gather_buff_comp[END][loop] && gather_buff_comp[END][rank] >= gather_buff_comp[END][loop])
                                        {
                                            overlap_time[loop] = gather_buff_comp[END][loop] - gather_buff_comp[START][rank] < 0 ? 0 : gather_buff_comp[END][loop] - gather_buff_comp[START][rank];
                                        }

                                        // std::cout << "Rank "<<rank<<" overlapp with Rank " << loop << " is " << overlap_time[loop] / 1e9 << std::endl;
                                        nonoverlap = (overlap_time[rank] - overlap_time[loop] < 50000 ? 0 : overlap_time[rank] - overlap_time[loop]);
                                        // std::cout << "Rank "<<rank<<" non overlapp with Rank " << loop << " is " << nonoverlap/1e9 << std::endl;
                                        operation.bufSize -= nonoverlap * 0.05;
                                    }
                                    else
                                    {

                                        int flag = 0;
                                        double loop_computing_time = gather_buff_comp[END][loop] - gather_buff_comp[START][loop];
                                        int overlapp_pieces = last_idle_time / loop_computing_time;

                                        double new_loop_start_time = gather_buff_comp[START][loop];
                                        double new_loop_end_time = gather_buff_comp[END][loop];
                                        if (gather_buff_comp[START][loop] < gather_buff_comp[START][rank]) // rank in future, loop in past
                                        {

                                            if ((abs(rank - loop) % task_per_node > overlapp_pieces) && last_idle_time > max_idle_time)
                                            {

                                                for (int k = 1; k <= overlapp_pieces; ++k)
                                                {
                                                    double temp = new_loop_start_time;
                                                    new_loop_start_time = new_loop_end_time;
                                                    new_loop_end_time += ((new_loop_end_time - temp) * 0.95);
                                                }

                                                new_loop_start_time += overlapp_pieces + 1 * commTime;
                                                new_loop_end_time += overlapp_pieces + 1 * commTime;
                                            }
                                            else
                                            {
                                                new_loop_start_time = gather_buff_comp[START][loop] + (((int(ceil(abs(loop - rank) / 2)) % task_per_node)) * last_idle_time) + (((abs(loop - rank) % task_per_node)) * commTime);
                                                new_loop_end_time = gather_buff_comp[END][loop] + (((int(ceil(abs(loop - rank) / 2)) % task_per_node)) * last_idle_time) + (((abs(loop - rank) % task_per_node)) * commTime);
                                            }

                                            if ((gather_buff_comp[START][rank] <= new_loop_start_time && gather_buff_comp[END][rank] >= new_loop_start_time) || (gather_buff_comp[START][rank] <= new_loop_end_time && gather_buff_comp[END][rank] >= new_loop_end_time))
                                            {
                                                if (gather_buff_comp[START][rank] <= new_loop_start_time && gather_buff_comp[END][rank] >= new_loop_start_time)
                                                {
                                                    overlap_time[loop] = gather_buff_comp[END][rank] - new_loop_start_time < 0 ? 0 : gather_buff_comp[END][rank] - new_loop_start_time;
                                                }
                                                else if (gather_buff_comp[START][rank] <= new_loop_end_time && gather_buff_comp[END][rank] >= new_loop_end_time)
                                                {
                                                    overlap_time[loop] = new_loop_end_time - gather_buff_comp[START][rank] < 0 ? 0 : new_loop_end_time - gather_buff_comp[START][rank];
                                                }

                                                nonoverlap = (overlap_time[rank] - overlap_time[loop] < 50000 ? 0 : overlap_time[rank] - overlap_time[loop]);

                                                operation.bufSize -= nonoverlap * 0.01;
                                            }
                                            else
                                            {
                                                operation.bufSize -= operation.bufSize * 0.05;
                                            }
                                        }
                                        else if (gather_buff_comp[START][loop] > gather_buff_comp[START][rank]) // rank in past, loop in future
                                        {

                                            if ((abs(rank - loop) % task_per_node > overlapp_pieces) && last_idle_time > max_idle_time)
                                            {

                                                for (int k = 1; k <= overlapp_pieces; ++k)
                                                {
                                                    double temp = new_loop_start_time;
                                                    new_loop_start_time = new_loop_end_time;
                                                    new_loop_end_time += ((new_loop_end_time - temp) * 0.95);
                                                }

                                                new_loop_start_time += overlapp_pieces + 1 * commTime;
                                                new_loop_end_time += overlapp_pieces + 1 * commTime;
                                            }
                                            else
                                            {
                                                new_loop_start_time = gather_buff_comp[START][loop] - (((int(ceil(abs(loop - rank) / 2)) % task_per_node)) * last_idle_time) - (((abs(loop - rank) % task_per_node)) * commTime);
                                                new_loop_end_time = gather_buff_comp[END][loop] - (((int(ceil(abs(loop - rank) / 2)) % task_per_node)) * last_idle_time) - (((abs(loop - rank) % task_per_node)) * commTime);
                                            }

                                            if ((gather_buff_comp[START][rank] <= new_loop_start_time && gather_buff_comp[END][rank] >= new_loop_start_time) || (gather_buff_comp[START][rank] <= new_loop_end_time && gather_buff_comp[END][rank] >= new_loop_end_time))
                                            {
                                                if (gather_buff_comp[START][rank] <= new_loop_start_time && gather_buff_comp[END][rank] >= new_loop_start_time)
                                                {
                                                    overlap_time[loop] = gather_buff_comp[END][rank] - new_loop_start_time < 0 ? 0 : gather_buff_comp[END][rank] - new_loop_start_time;
                                                }
                                                else if (gather_buff_comp[START][rank] <= new_loop_end_time && gather_buff_comp[END][rank] >= new_loop_end_time)
                                                {
                                                    overlap_time[loop] = new_loop_end_time - gather_buff_comp[START][rank] < 0 ? 0 : new_loop_end_time - gather_buff_comp[START][rank];
                                                }

                                                nonoverlap = (overlap_time[rank] - overlap_time[loop] < 50000 ? 0 : overlap_time[rank] - overlap_time[loop]);

                                                operation.bufSize -= nonoverlap * 0.01;
                                            }
                                            else
                                            {
                                                operation.bufSize -= operation.bufSize * 0.05;
                                            }
                                        }
                                    }
                                }
                            }
                        }

#ifdef USE_VERBOSE
                        std::cout << "Rank : " << N_process_Rank << " in iter : " << iter_num << " finished BCM" << std::endl;
#endif

                        oSuccessor[operation.rank][operation.node] = operation.time + operation.bufSize;

                        /// injected long disturbance at given rank and given timestep
                        if (!initialized && (operation.rank == desync_rank) && (iter_num == desync_timestep) && (idlewave_feature == 1))
                        {

                            initialized = true;
                            oSuccessor[operation.rank][operation.node] = operation.time + operation.bufSize + 25 * operation.bufSize;

#ifdef USE_VERBOSE
                            std::cout << "Rank : " << N_process_Rank << " in iter : " << iter_num << " injected delay to create idle wave" << std::endl;
#endif
                        }

                        AST_args.graphVec[operation.rank].setOp(operation.rank, operation.label, benchmark, ID);

                        AST_args.graphVec[operation.rank].unsetOp(operation.rank, operation.label, benchmark, ID);

#ifdef USE_VERBOSE
                        std::cout << "Rank : " << N_process_Rank << " in iter : " << iter_num << " AST graph set and unSet operation done" << std::endl;
#endif
                        finishedRankList.push_back(operation.rank);
                        operation.numOpsInQueue = numOpsInQueue++;

#ifdef USE_CHROMEVIZ
                        // traceVisualizer2.comp(VIS_HOST_DUR, new HostDurationVisEvent("Name:COMP", operation.rank, ("CPU" + std::to_string(operation.node)), operation.time,operation.time+operation.bufSize));
                        traceVisualizer2.args("CPU" + std::to_string(operation.node), operation.rank);
                        traceVisualizer2.completeEvents("Name:COMP", operation.rank, traceVisualizer2.arc[("CPU" + std::to_string(operation.node))], std::to_string(operation.time / 1000), std::to_string(((operation.time + operation.bufSize) - operation.time) / 1000));
#endif

#ifdef USE_DRAWVIZ
                        traceVisualizer.comp(operation.rank, operation.time, operation.time + operation.bufSize, operation.node);
#endif
                    }
                    commTime = 0;
                    max_idle_time = std::max(max_idle_time, last_idle_time);
                    last_idle_time = operation.time;

#ifdef USE_VERBOSE
                    std::cout << "Rank : " << N_process_Rank << " in iter : " << iter_num << " finished inserting data in DMS file and completed COMP" << std::endl;
#endif
                }
                break;

                case DisCosTiC::Operation_t::SEND:
                {
                    if (computingProcessesCount != 0)
                        computingProcessesCount--;

                    CompRankList.erase(std::remove(CompRankList.begin(), CompRankList.end(), operation.rank), CompRankList.end());

#ifdef USE_VERBOSE
                    std::cout << "Rank : " << N_process_Rank << " in iter : " << iter_num << " in SEND" << std::endl;
                    verboseSendPrint(operation);
#endif
                    int comm_type = 0;

                    if (std::max(oSuccessor[operation.rank][operation.node], gsendSuccessor[operation.rank][operation.network]) <= operation.time && (operation.bufSize != 0))
                    {
                        int target_cluster = operation.target < primary_processes ? 0 : 1;
                        int target_node = std::floor(operation.target / task_per_node);
                        int target_socket = std::floor(operation.target / cores_per_socket);

                        // AST_args.graphVec[operation.rank].setOp(operation.rank, operation.label, benchmark, ID); //(CAN COMMENT)

                        if (target_cluster != system_number)
                        {
                            comm_type = INTERCLUSTER;
                        }
                        else if (target_node != node)
                        {
                            comm_type = INTERNODE;
                        }
                        else if (target_socket != socket)
                        {
                            comm_type = INTERCHIP;
                        }
                        else
                        {
                            comm_type = INTRACHIP;
                        }

#ifdef USE_VERBOSE
                        std::cout << "Rank : " << N_process_Rank << " in iter : " << iter_num << " communication properties determined" << std::endl;
#endif

                        if (comm_model == LOGGP)
                        {

                            oSuccessor[operation.rank][operation.node] = operation.time + o[comm_type] + 2 * ((operation.bufSize - 1) * O[comm_type]);
                            gsendSuccessor[operation.rank][operation.network] = operation.time + g[comm_type] + 2 * ((operation.bufSize - 1) * G[comm_type]); // TODO: G should be charged in network layer only

#ifdef USE_CHROMEVIZ
                            traceVisualizer2.args("CPU" + std::to_string(operation.node), operation.rank);
                            traceVisualizer2.completeEvents("Name:SEND", operation.rank, traceVisualizer2.arc[("CPU" + std::to_string(operation.node))], std::to_string(operation.time / 1000), std::to_string(2 * ((operation.time + o[comm_type] + (operation.bufSize - 1) * O[comm_type]) - operation.time) / 1000));
#endif

#ifdef USE_DRAWVIZ
                            traceVisualizer.osend(operation.rank, operation.time, operation.time + o + (operation.bufSize - 1) * O, operation.node);
#endif
                        }
                        if (comm_model == SIMPLELB)
                        {
                            oSuccessor[operation.rank][operation.node] = operation.time + 2 * ((operation.bufSize) * inverse_bandwidth[comm_type] + latency[comm_type]);
                            gsendSuccessor[operation.rank][operation.network] = operation.time + 2 * ((operation.bufSize) * inverse_bandwidth[comm_type] + latency[comm_type]); // TODO: G should be charged in network layer only

#ifdef USE_CHROMEVIZ
                            traceVisualizer2.args("CPU" + std::to_string(operation.node), operation.rank);
                            traceVisualizer2.completeEvents("Name:SEND", operation.rank, traceVisualizer2.arc[("CPU" + std::to_string(operation.node))], std::to_string(operation.time / 1000), std::to_string(2 * ((operation.bufSize) * inverse_bandwidth[comm_type] + latency[comm_type]) / 1000));
#endif

#ifdef USE_DRAWVIZ
                            traceVisualizer.osend(operation.rank, operation.time, operation.time + o + (operation.bufSize - 1) * O, operation.node);
#endif
                        }

                        commTime += (oSuccessor[operation.rank][operation.node] - operation.time);

                        if (operation.bufSize > eagerLimit)
                        {
#ifdef USE_VERBOSE
                            std::cout << "Rank : " << N_process_Rank << " in iter : " << iter_num << " obeying RENDEZVOUS protocol" << std::endl;
#endif
                            if (operation.time < gather_buff_optime[operation.target])
                            {

                                operation.time = gather_buff_optime[operation.target] + commTime;
                                oSuccessor[operation.rank][operation.node] = gather_buff_optime[operation.target] + commTime;
                                gsendSuccessor[operation.rank][operation.network] = gather_buff_optime[operation.target] + commTime;
                            }
                        }

                        starttimeProcess = operation.starttime;

                        operation.starttime = operation.time;

                        if (comm_model == LOGGP)
                        {
                            operation.time = operation.time + o[comm_type] + latency[comm_type]; // time of first byte only (bandwidth G will be charged at receiver)
                        }
                        if (comm_model == SIMPLELB)
                        {
                            operation.time = operation.time + 2 * ((operation.bufSize) * inverse_bandwidth[comm_type] + latency[comm_type]); // time of first byte only (bandwidth G will be charged at receiver)
                        }

                        DisCosTiCOps++;
                        operation.numOpsInQueue = numOpsInQueue++;
                        int temp = oSuccessor[operation.rank][operation.node];

                        double arr[13];

                        // Serialize after operation information is recieved

                        arr[0] = operation.time;
                        arr[1] = operation.starttime;
                        arr[2] = operation.syncstart;
                        arr[3] = operation.numOpsInQueue;
                        arr[4] = operation.bufSize;
                        arr[5] = operation.target;
                        arr[6] = operation.rank;
                        arr[7] = operation.label;
                        arr[8] = operation.tag;
                        arr[9] = operation.node;
                        arr[10] = operation.network;
                        arr[11] = operation.type;
                        arr[12] = temp;

                        request = MPI_REQUEST_NULL;

                        if (operation.mode == DisCosTiC::Mode_t::NONBLOCKING)
                        {
                            // MPI_Send(&arr, 13, MPI_DOUBLE, operation.target, operation.tag, newcomm);
                            MPI_Isend(&arr, 13, MPI_DOUBLE, operation.target, operation.tag, newcomm, &request);
                            MPI_Wait(&request, &status);
                        }
                        else
                        {
                            MPI_Send(&arr, 13, MPI_DOUBLE, operation.target, operation.tag, newcomm);
                        }
#ifdef USE_VERBOSE
                        std::cout << "Rank : " << N_process_Rank << " in iter : " << iter_num << " serialized data and sent to respective rank" << std::endl;
#endif
                        finishedRankList.push_back(operation.rank);

                        cycle = true;
                    }
#ifdef USE_VERBOSE
                    std::cout << "Rank : " << N_process_Rank << " in iter : " << iter_num << " finished SEND" << std::endl;
#endif
                }
                break;

                case DisCosTiC::Operation_t::RECV:
                {
                    if (operation.bufSize != 0)
                    {

#ifdef USE_VERBOSE
                        std::cout << "Rank : " << N_process_Rank << " in iter : " << iter_num << " finished SEND" << std::endl;
                        verboseRecvPrint(operation);
#endif
                        int comm_type = 0;
                        double brr[13] = {0};

                        // AST_args.graphVec[operation.rank].setOp(operation.rank, operation.label, benchmark, ID); //(CAN COMMENT)

                        request = MPI_REQUEST_NULL;

                        if (operation.mode == DisCosTiC::Mode_t::NONBLOCKING)
                        {
                            // MPI_Recv(&brr, 13, MPI_DOUBLE, MPI_ANY_SOURCE, operation.tag, newcomm, &status);
                            MPI_Irecv(&brr, 13, MPI_DOUBLE, MPI_ANY_SOURCE, operation.tag, newcomm, &request);
                            MPI_Wait(&request, &status);
                        }
                        else
                        {

                            // MPI_Irecv(&brr, 13, MPI_DOUBLE, MPI_ANY_SOURCE, operation.tag, newcomm, &request);

                            MPI_Recv(&brr, 13, MPI_DOUBLE, MPI_ANY_SOURCE, operation.tag, newcomm, &status);
                        }
                        //  Deserialize after operation information is recieved

                        operation.time = std::max(operation.time, brr[0]);
                        operation.starttime = brr[1];
                        operation.syncstart = brr[2];
                        operation.numOpsInQueue = brr[3];
                        operation.bufSize = brr[4];
                        operation.target = brr[5];
                        // operation.rank=brr[6];
                        operation.label = brr[7];
                        operation.tag = brr[8];
                        operation.node = brr[9];
                        operation.network = brr[10];
                        operation.type = brr[11];
                        oSuccessor[operation.rank][operation.node] = brr[12];
                        cycle = true;

#ifdef USE_VERBOSE
                        std::cout << "Rank : " << N_process_Rank << " in iter : " << iter_num << " received buffer from respective rank and deserialized" << std::endl;
#endif

                        int target_cluster = brr[6] < primary_processes ? 0 : 1;
                        int target_node = std::floor(brr[6] / task_per_node);
                        int target_socket = std::floor(brr[6] / cores_per_socket);

                        if (target_cluster != system_number)
                        {
                            comm_type = INTERCLUSTER;
                        }
                        else if (target_node != node)
                        {
                            comm_type = INTERNODE;
                        }
                        else if (target_socket != socket)
                        {
                            comm_type = INTERCHIP;
                        }
                        else
                        {
                            comm_type = INTRACHIP;
                        }

                        if (comm_model == LOGGP)
                        {
                            oSuccessor[operation.rank][operation.node] = operation.time + o[comm_type] + 2 * std::max((operation.bufSize - 1) * O[comm_type], (operation.bufSize - 1) * G[comm_type]); //!< message is only received after G is charged
                            grecvSuccessor[operation.rank][operation.network] = operation.time + g[comm_type] + 2 * ((operation.bufSize - 1) * G[comm_type]);
                        }
                        if (comm_model == SIMPLELB)
                        {
                            oSuccessor[operation.rank][operation.node] = operation.time + 2 * ((operation.bufSize) * inverse_bandwidth[comm_type]);
                            gsendSuccessor[operation.rank][operation.network] = operation.time + 2 * ((operation.bufSize) * inverse_bandwidth[comm_type]); // TODO: G should be charged in network layer only
                        }
                        commTime += (oSuccessor[operation.rank][operation.node] - operation.time);

                        // print_OpPropertiesNonPointerT(operation);
                        operation.numOpsInQueue = numOpsInQueue++;
                        finishedRankList.push_back(operation.rank);

#ifdef USE_VERBOSE
                        std::cout << "[RECV ==> SATISFY] local non-blocking dependency" << std::endl;
                        std::cout << "Unexpected message queue bufSize: " << UMQ.size() << std::endl;
                        std::cout << "matching queue search for src " << operation.target << " tag " << operation.tag << std::endl;
#endif

#ifdef USE_VERBOSE
                        std::cout << " [RECV ==> FOUND] in local unexpected queue" << std::endl;
#endif

                        if (operation.bufSize > eagerLimit)
                        { // rendezvous - free remote request
                            /// satisfy remote requires

                            // AST_args.graphVec[operation.target].unsetOp(operation.target, operation.label /*the label of the remote packet*/, benchmark, ID); //(CAN COMMENT)

                            finishedRankList.push_back(operation.target);
                            /// set the remote clocks to operation.time (there is no event < operation.time in the queue)

                            // if (oSuccessor[operation.target][operation.node] < operation.time)
                            //     oSuccessor[operation.target][operation.node] = operation.time;
                            // if (gsendSuccessor[operation.target][operation.network] < operation.time)
                            //     gsendSuccessor[operation.target][operation.network] = operation.time;

#ifdef USE_VERBOSE
                            verboseRendezvousRecvPrint(operation);
#endif

#ifdef USE_DRAWVIZ
                            traceVisualizer.msg(operation.rank, operation.target, operation.time + o, operation.time, operation.bufSize, G[operation.network]);
#endif
                        }

                        // #ifdef USE_CHROMEVIZ
                        //                     traceVisualizer2.args("CPU" + std::to_string(operation.node), operation.rank);
                        //                     traceVisualizer2.completeEvents("Name:RECV", operation.rank, traceVisualizer2.arc[("CPU" + std::to_string(operation.node))], std::to_string(operation.time / 1000), std::to_string(((operation.time + o) - operation.time) / 1000));
                        // #endif
                        if (comm_model == LOGGP)
                        {
#ifdef USE_CHROMEVIZ
                            traceVisualizer2.args("CPU" + std::to_string(0), operation.target);
                            traceVisualizer2.args("CPU" + std::to_string(0), operation.rank);
                            traceVisualizer2.flowEventBegin("Name:startMSG", operation.target, traceVisualizer2.arc[("CPU" + std::to_string(0))], std::to_string((operation.starttime + o[comm_type]) / 1000), flow_id);
                            traceVisualizer2.flowEventEnd("Name:endMSG", operation.rank, traceVisualizer2.arc[("CPU" + std::to_string(0))], std::to_string(operation.time / 1000), flow_id);
                            flow_id++;

                            traceVisualizer2.args("CPU" + std::to_string(operation.node), operation.rank);
                            traceVisualizer2.completeEvents("Name:RECV", operation.rank, traceVisualizer2.arc[("CPU" + std::to_string(operation.node))], std::to_string((operation.time + (operation.bufSize - 1) * G[comm_type] - (operation.bufSize - 1) * O[comm_type]) / 1000), std::to_string(2 * ((operation.time + o[comm_type] + std::max((operation.bufSize - 1) * O[comm_type], (operation.bufSize - 1) * G[comm_type])) - (operation.time + (operation.bufSize - 1) * G[comm_type] - (operation.bufSize - 1) * O[comm_type])) / 1000));

#endif

#ifdef USE_DRAWVIZ
                            traceVisualizer.orecv(operation.rank, operation.time, operation.time + o, operation.node);
#endif
                        }
                        if (comm_model == SIMPLELB)
                        {
#ifdef USE_CHROMEVIZ
                            traceVisualizer2.args("CPU" + std::to_string(0), operation.target);
                            traceVisualizer2.args("CPU" + std::to_string(0), operation.rank);
                            traceVisualizer2.flowEventBegin("Name:startMSG", operation.target, traceVisualizer2.arc[("CPU" + std::to_string(0))], std::to_string((operation.starttime) / 1000), flow_id);
                            traceVisualizer2.flowEventEnd("Name:endMSG", operation.rank, traceVisualizer2.arc[("CPU" + std::to_string(0))], std::to_string(operation.time / 1000), flow_id);
                            ++flow_id;

                            traceVisualizer2.args("CPU" + std::to_string(operation.node), operation.rank);
                            traceVisualizer2.completeEvents("Name:RECV", operation.rank, traceVisualizer2.arc[("CPU" + std::to_string(operation.node))], std::to_string((operation.time) / 1000), std::to_string(2 * ((operation.bufSize) * inverse_bandwidth[comm_type]) / 1000));

#endif

#ifdef USE_DRAWVIZ
                            traceVisualizer.orecv(operation.rank, operation.time, operation.time + o, operation.node);
#endif
                        }

                        finishedRankList.push_back(operation.rank);

                        if ((desync_feature == 1) && (iter_num > 50 && iter_num < 200))
                        {
#ifdef USE_VERBOSE
                            std::cout << "Rank : " << N_process_Rank << " in iter : " << iter_num << " desynchronizing simulation by injecting noise" << std::endl;
#endif
                            oSuccessor[operation.rank][operation.node] += rand() % 5000;
                        }
                    }
#ifdef USE_VERBOSE
                    std::cout << "Rank : " << N_process_Rank << " in iter : " << iter_num << " finished RECV" << std::endl;
#endif
                }
                break;

                default:
                    std::cout << "YET NOT IMPLEMENTED! : " << (int)operation.type << std::endl;
                    break;
                }

                std::sort(finishedRankList.begin(), finishedRankList.end());
                finishedRankList.erase(std::unique(finishedRankList.begin(), finishedRankList.end()), finishedRankList.end());

#ifdef USE_VERBOSE
                std::cout << "Rank : " << N_process_Rank << " finished iter : " << iter_num << std::endl;
#endif
                /**
                 * \brief add new sorted operations order by type to the queue
                 */

                for (auto &rank : finishedRankList)
                {
                    DisCosTiC::Grid *it = &AST_args.graphVec[rank];
                    DisCosTiC::Operations operations;
                    it->getTypeSortedOps(operations);

                    if (operations.size() != 0)
                        operations_queue.insert(operations_queue.end(), operations.begin(), operations.end());

                    if (operations_queue.size() != 0)
                    {
                        auto op = operations_queue.front();
                        operations_queue.erase(operations_queue.begin()); // removes the front element.
                        op.rank = rank;
                        switch (op.type)
                        {
                        case DisCosTiC::Operation_t::COMP:
                            op.time = oSuccessor[rank][op.node];
#ifdef USE_VERBOSE
                            verboseCompFinalPrint(rank, op);
#endif
                            break;

                        case DisCosTiC::Operation_t::SEND:
                            op.time = std::max(oSuccessor[rank][op.node], gsendSuccessor[rank][op.network]);
#ifdef USE_VERBOSE
                            verboseSendFinalPrint(rank, op);
#endif
                            break;

                        case DisCosTiC::Operation_t::RECV:
                            op.time = oSuccessor[rank][op.node];
#ifdef USE_VERBOSE
                            verboseRecvFinalPrint(rank, op);
#endif
                            break;

                        default:
                            std::cerr << "YET NOT IMPLEMENTED!" << std::endl;
                        }

                        operation = op;
#ifdef USE_VERBOSE
                        std::cout << "Rank : " << N_process_Rank << " in iter : " << iter_num << " set new operation" << std::endl;
#endif
                    }
                    else
                    {
                        // double start = MPI_Wtime();

                        // // I simulate the sleep function. Required for receiving the send and recv in given fashion
                        // while(MPI_Wtime() - start < 0.05)
                        // {
                        //     // We keep looping until <waiting_time> seconds have elapsed
                        // }
                        iter_num++;
                        cycle = false;
#ifdef USE_VERBOSE
                        std::cout << "Rank : " << N_process_Rank << " in iter : " << iter_num << " exiting cycle as no new operation in queue" << std::endl;
#endif
                    }
                    // exit(0);
                }

#ifdef USE_VERBOSE
                std::cout << "Rank : " << N_process_Rank << " in iter : " << iter_num << " finished RECV" << std::endl;
#endif

            } while (cycle);
            cycle = true;
            operation_og.time = oSuccessor[operation.rank][operation.node];
            operation = copy(operation_og, operation);

        } while (iter_num < iter);
        double brr[5];
        *finalize(oSuccessor[rank][operation.node], numRanks, numOpsInQueue, rank, operation.node, brr);
        MPI_Send(&brr, 5, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD);

#ifdef USE_VERBOSE
        std::cout << "Rank : " << process_Rank << " send data to MASTER RANK" << std::endl;
#endif
    }
    /**
     * \brief Start of Master Ranks code
     * runs for the master rank
     */
    else if (process_Rank == 0)
    {
        bool cycle = true;
        double brr[5];

        while (test.size() < numRanks)
        {

            MPI_Recv(&brr, 5, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            if (status.MPI_TAG == 1)
            {
                test.emplace(status.MPI_SOURCE);
                oSuccessor[brr[3]][brr[4]] = brr[0];
                numOpsInQueue = brr[2];
                // print_vec2T(oSuccessor);
            }
        }
#ifdef USE_VERBOSE
        std::cout << "Rank : " << process_Rank << " MASTER RANK received data from all simulation processes" << std::endl;
#endif
    }
    /**
     * \brief end of parallelization
     */

    ierr = MPI_Comm_rank(MPI_COMM_WORLD, &process_Rank);
    MPI_Barrier(MPI_COMM_WORLD);
    traceVisualizer2.closeFile();

#ifdef USE_VERBOSE
    std::cout << "Rank : " << process_Rank << " All simulation processes completed simulation" << std::endl;
#endif

    if (process_Rank == 0)
    {

        //////////////////////////////////////////////////////////////////////////////////////////////////////////
        //                  /* \brief Ending Time and Printing Performance information  */                      //
        //////////////////////////////////////////////////////////////////////////////////////////////////////////

        endTime = high_resolution_clock::now();
        DisCosTiC_Timetype DisCosTiC = duration_cast<nanoseconds>(endTime - startTime).count();
        ;

        std::cout << "\n\nDisCosTiC PERFORMANCE:" << std::endl;
        std::cout << "----------------------------------------------------------------" << std::endl;
        std::cout << "Number of operations    	Runtime [s]       Speed [operations/ns]" << std::endl;
        std::cout << "----------------------------------------------------------------" << std::endl;
        std::cout << numOpsInQueue << "     \t\t\t\t" << DisCosTiC / 1e9 << "   \t\t" << numOpsInQueue / DisCosTiC << std::endl;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //                                   /* \brief check if all queues are empty (debugging the queues) if true                //
        //    * so print finishing time (either for all ranks, or for only maximum rank with enabled batch mode)  */               //
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        if (CFG_args.getValue<int>("batchmode"))
        {
            slowRankTime(numRanks, oSuccessor);
        }
        else if (!CFG_args.getValue<int>("batchmode"))
        {
            allRanksTime(numRanks, oSuccessor);
        }
        else
        {
            throw std::runtime_error("ERROR: QUEUES NOT EMPTY!\n");
        }
#ifdef USE_CHROMEVIZ
        system(("rm " + CFG_args.getValue<std::string>("chromevizfilename") + ".dms 2> /dev/null").c_str());

        system(("cat " + CFG_args.getValue<std::string>("chromevizfilename") + "{0.." + std::to_string(size_Of_Cluster - 2) + "}.dms > " + CFG_args.getValue<std::string>("chromevizfilename") + ".dms").c_str());

        std::string filename = CFG_args.getValue<std::string>("chromevizfilename") + ".dms";
        std::ofstream ofs;
        ofs.open(filename, std::ios_base::app);

        ofs << "{}\n],"
            << "\"Name\": \"Trace visualisation ofs DisCosTiC toolkit\", \n "
            << "\"Full form\": \"Distributed CosT in Cluster\", \n "
            << "\"Version\": \"v0.0.0 (initial release)\", \n "
            << "\"Author\": \"Ayesha Afzal <ayesha.afzal@fau.de>\", \n "
            << "\"Copyright\": \"© 2023 HPC, FAU Erlangen-Nuremberg. All rights reserved\" \n "
            << "}\n";
        ofs.close();

#endif
#ifdef USE_DRAWVIZ
        system("rm DisCosTiC.out 2> /dev/null");

        system("cat DisCosTiC*.out >> DisCosTiC.out");

#endif
        system(("rm DisCosTiC{0.." + std::to_string(size_Of_Cluster - 2) + "}.out 2> /dev/null").c_str());
        system(("rm " + CFG_args.getValue<std::string>("chromevizfilename") + "{0.." + std::to_string(size_Of_Cluster - 2) + "}.dms 2> /dev/null").c_str());

#ifdef USE_VERBOSE
        std::cout << "Rank : " << process_Rank << " wrote data to MASTER file and cleaned up files" << std::endl;
#endif
    }

    MPI_Finalize();

    return EXIT_SUCCESS;
}
