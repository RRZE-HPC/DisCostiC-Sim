#ifndef AST_HPP
#define AST_HPP

#pragma once

// #include <string>
#include <iostream>
#include <fstream>
#include <mpi.h>
#include <string>
#include "enum.hpp"         //!<  enumerated types
#include "DataStruct.hpp"   //!< data structures
#include "DataType.hpp"     //!< data types
#include "ConfigParser.hpp" //!< configuration file parser
#include "../nodelevel/include/NodeLvlScg.hpp"
#include "YAMLParser.hpp"

UserInterface::ConfigParser CFG_args("config.cfg");
static bool Verbose = CFG_args.getValue<DisCosTiC_Datatype>("Verbose");
static bool kerncraftExecuted = false;
static bool barrier = false;
static bool barrier_hetero = false;

int scaling_cores = -1;
int bytes_to_send = 0;

// Global variables for setting individual system properties
extern int virtual_rank;
extern int system_number;
extern int task_per_node;
extern int node;
extern int cc_numa_domain_per_socket;
extern int cores_per_socket;
extern int cc_numa_domain;
extern int socket;
extern int primary_processes;
extern int secondary_processes;
extern int heteregeneous_mode;
extern std::string arch_name;

class AST
{
public:
    std::vector<DisCosTiC::AST_OP *> allNodes;
    std::vector<DisCosTiC::AST_OP *> RootNodes;
    // DisCosTiC_Datatype nodesCount, networksCount;

    /// private variables required for nonblocking calls
private:
    idSetT start, end; //!< the operations which are independent at start and end
    std::string content, filename;
    std::fstream myfile;
    DisCosTiC_Indextype labelCount, edgesCount;

    std::vector<bool> ranks_init;
    DisCosTiC_Indextype count;
    DisCosTiC_Datatype dummyNode, node;
    DisCosTiC_Datatype sendCount, recvCount, compCount, rankCount, depCount, curtag; // timesteps;
    DisCosTiC_Datatype rank, timeunit_conv;

    std::map<std::string, DisCosTiC::AST_OP *> *indicesTable = new std::map<std::string, DisCosTiC::AST_OP *>;
    std::map<std::string, std::string> *depTable = new std::map<std::string, std::string>;
    std::map<std::string, DisCosTiC::AST_OP_TYPE> *indicesDeserializedTable = new std::map<std::string, DisCosTiC::AST_OP_TYPE>;

    DisCosTiC_Datatype mode;
    DisCosTiC_Timetype execsize;
    std::string func;

public:
    /**
     * \brief constructor that initializes the coordinates for sendCount, recvCount, compCount, rankCount, depCount, and curtag. (create filename)
     * \param CFG_args and numRanks
     */
    AST(UserInterface::ConfigParser *CFG_args, DisCosTiC_Datatype numRanks) : sendCount(0),
                                                                              recvCount(0),
                                                                              compCount(0),
                                                                              rankCount(0),
                                                                              depCount(0),
                                                                              curtag(-1)
    {
        this->filename.clear();
        this->filename.append(std::string(CFG_args->getValue<std::string>("filename")));
        this->myfile.open(this->filename.c_str(),
                          std::fstream::out | std::fstream::trunc);
        std::stringstream ss;
        ss << "num_ranks " << numRanks << std::endl;
        AppendString(ss.str(), content);
        this->rankCount = numRanks;
        this->node = CFG_args->getValue<DisCosTiC_Datatype>("node");
        this->func = CFG_args->getValue<std::string>("benchmark_kernel");
        this->execsize = CFG_args->getValue<DisCosTiC_Datatype>("balance_comp_size_in_ns");
        this->timeunit_conv = CFG_args->getValue<DisCosTiC_Datatype>("timeunit_conv");
        // this->ranks_init.resize(numRanks);
        // std::fill(this->ranks_init.begin(), this->ranks_init.end(), false);
        // if(!ranks_init[rank])  {do something; ranks_init[rank] = true;}
    }

    /**
     * \brief destructor
     */
    ~AST()
    {
        fileclose(myfile, filename);

        /*std::cout << "\nGenerated AST: " << std::endl;
        std::cout << "----------------------------------------------------------------"<< std::endl;
        std::cout << "number of processes : " << rankCount << std::endl;
        std::cout << "number of sendCount : " << sendCount  << std::endl;
        std::cout << "number of receives : " << recvCount << std::endl;
        std::cout << "number of executions : " << compCount << std::endl;
        std::cout << "number of dependencies : " << depCount << std::endl;
        std::cout << "(NOTE: Number of operations = Number of rankCount * Number of timesteps * 4 (COMP+SEND+RECV+MSG))"<< std::endl;
        if(mode== DisCosTiC::Mode_t::NONBLOCKING)
            std::cout << "Non-blocking communication routines" << std::endl;
        else if(mode== DisCosTiC::Mode_t::BLOCKING) {
            std::cout << "Blocking communication routines" << std::endl;

        }
        std::cout << "----------------------------------------------------------------"<< std::endl;*/
    }

    /**
     * \brief append to independent set
     */
    inline void InsertSrcDest()
    {
        start.insert(count);
        end.insert(count);
    }

    /**
     * \brief EraseSrcDest from independent set
     * \param [in] src of DisCosTiC_Indextype datatype
     * \param [in] dest of DisCosTiC_Indextype datatype
     */
    inline void EraseSrcDest(DisCosTiC_Indextype src, DisCosTiC_Indextype dest)
    {
        start.erase(src);
        end.erase(dest);
    }

    /**
     * \brief start an operation
     */
    inline void StartOp()
    {
        start.clear();
        end.clear();
    }

    /**
     * \brief complete an operation
     */
    inline locopPair_t EndOp()
    {
        locop_t rstart, rend;
        for (idSetT::iterator it = start.begin(); it != start.end(); it++)
        {
            rstart.push_back(std::make_pair(*it, DisCosTiC::BLOCKING));
        }
        for (idSetT::iterator it = end.begin(); it != end.end(); it++)
        {
            rend.push_back(std::make_pair(*it, DisCosTiC::BLOCKING));
        }
        return std::make_pair(rstart, rend);
    }

    /**
     * \brief set a tag
     * \param [in] message tag of integer type
     */
    inline void Settag(DisCosTiC_Datatype tag)
    {
        this->curtag = tag;
    }

    inline void SetRank(DisCosTiC_Datatype r)
    {
        this->rank = r;
        // std::cout << "current rank: " << rank << std::endl;
    }

    inline void SetNumRanks(DisCosTiC_Datatype nr)
    {
        this->rankCount = nr;
        // std::cout << "total number of processes: " << rankCount << std::endl;
    }

    inline void insertID(std::map<std::string, DisCosTiC::AST_OP *> *idTable, char *ID, DisCosTiC::AST_OP *op)
    {
        idTable->insert(std::make_pair(std::string(ID), op));
        // free(ID);
    }

    inline void insertdeserialID(std::map<std::string, DisCosTiC::AST_OP_TYPE> *idTable, char *ID, DisCosTiC::AST_OP_TYPE op)
    {
        idTable->insert(std::make_pair(ID, op));
        // free(ID);
    }

    inline void insertDep(std::map<std::string, std::string> *depTable, char *ID1, char *ID2)
    {
        depTable->insert(std::make_pair(std::string(ID1), std::string(ID2)));
        // free(ID1);
        // free(ID2);
    }

    inline DisCosTiC::AST_OP *retrieveID(std::map<std::string, DisCosTiC::AST_OP *> *idTable, std::string id)
    {
        std::map<std::string, DisCosTiC::AST_OP *>::iterator it;
        it = idTable->find(id);
        if (id.compare("l-1") == 0)
        {
            std::cerr << "INVALID IDENTIFIER: l-1 " << std::endl;
            exit(EXIT_FAILURE);
        }
        if ((it == idTable->end()) && (!(id).empty()))
        {
            std::cerr << "ERROR: DEPENDENCY REFERENCES LABEL " << id << " UNDEFINED!" << std::endl;
            exit(EXIT_FAILURE);
        }
        // free(id);
        return it->second;
    }

    inline DisCosTiC::AST_OP_TYPE retrievedeserialID(std::map<std::string, DisCosTiC::AST_OP_TYPE> *idTable, std::string id)
    {
        std::map<std::string, DisCosTiC::AST_OP_TYPE>::iterator it;
        it = idTable->find(id);
        if (id.compare("l-1") == 0)
        {
            std::cerr << "INVALID IDENTIFIER: l-1 " << std::endl;
            exit(EXIT_FAILURE);
        }
        if ((it == idTable->end()) && (!(id).empty()))
        {
            std::cerr << "ERROR: DEPENDENCY REFERENCES LABEL " << id << " UNDEFINED!" << std::endl;
            exit(EXIT_FAILURE);
        }
        // free(id);
        return it->second;
    }

    /**
     * \brief start a rank
     * \param [in] processor rank of integer type
     */
    inline void Rank_Init(DisCosTiC_Datatype rank)
    {
        this->rankCount++;
        this->curtag = 0;
        this->edgesCount = 0;
        this->labelCount = 0;
        this->count = 0; //!< reset label counter
        this->SetRank(rank);
        this->allNodes.clear();
        this->RootNodes.clear();
        this->indicesTable->clear();
        this->depTable->clear();

        std::stringstream ss;
        ss << "\nrank " << rank << " {\n";
        AppendString(ss.str(), content);
    }

    /*void StartTimeSteps(DisCosTiC_Datatype timestep) {
        this->timesteps++;
    }*/

    inline DisCosTiC::AST_OP *addNode()
    {
        DisCosTiC::AST_OP *N = new DisCosTiC::AST_OP;
        N->depCount = 0;
        N->label = labelCount;
        labelCount++;
        allNodes.push_back(N);
        return N;
    }

    inline DisCosTiC_Datatype MaxCPU(DisCosTiC_Datatype node = 0)
    {
        static DisCosTiC_Datatype max_node = 0;
        if (node > max_node)
            max_node = node;
        return max_node;
    }

    inline DisCosTiC_Datatype Maxnetwork(DisCosTiC_Datatype network = 0)
    {
        static DisCosTiC_Datatype max_network = 0;
        if (network > max_network)
            max_network = network;
        return max_network;
    }

    /**
     * \brief get the number of operations for each rank
     */
    inline DisCosTiC_Datatype getNumOps()
    {
        return this->allNodes.size();
    }

    /**
     * \brief perform a message sending operation
     * \param [in] the number of sending buffer elements (bufsize) of integer datatype of each send buffer element
     * \param [in] destination rank (dst) of integer datatype
     */
    inline DisCosTiC::Event Send(DisCosTiC_Datatype bufsize, DisCosTiC_Datatype dst, DisCosTiC::Event depOP)
    {
        this->sendCount++;
        this->count++;

        std::stringstream ss;
        ss << "l" << this->count << ": send " << bufsize << "b to " << dst << " tag " << curtag << std::endl;
        AppendString(ss.str(), content);

        InsertSrcDest();
        // return this->count;

        DisCosTiC::AST_OP *n = addNode();

        n->type = DisCosTiC::Operation_t::SEND;
        n->target = dst;
        n->tag = curtag;
        n->node = 0 /*node*/;
        n->network = 0 /*network*/;
        n->bufSize = bufsize;

        MaxCPU(0 /*node*/);
        Maxnetwork(0 /*network*/);

        this->mode = DisCosTiC::Mode_t::BLOCKING;
        blockingDep(std::make_pair(this->count, n), depOP);

        return std::make_pair(this->count, n);
    }

    /**
     * \brief perform a message sending operation
     * \param [in] the number of sending buffer elements (bufsize) of integer datatype of each send buffer element
     * \param [in] destination rank (dst) of integer datatype
     */
    inline DisCosTiC::Event Send(const void *buf, DisCosTiC_Datatype bufsize, MPI_Datatype datatype, DisCosTiC_Datatype dst, int tag,
                                 MPI_Comm comm, MPI_Request *request, DisCosTiC::Event depOP)
    {
        this->sendCount++;
        this->count++;

        std::stringstream ss;
        ss << "l" << this->count << ": send " << bufsize << "b to " << dst << " tag " << curtag << std::endl;
        AppendString(ss.str(), content);

        InsertSrcDest();
        // return this->count;

        DisCosTiC::AST_OP *n = addNode();

        n->type = DisCosTiC::Operation_t::SEND;
        n->target = dst;
        n->tag = curtag;
        n->node = 0 /*node*/;
        n->network = 0 /*network*/;
        n->bufSize = bufsize;

        MaxCPU(0 /*node*/);
        Maxnetwork(0 /*network*/);

        this->mode = DisCosTiC::Mode_t::BLOCKING;
        blockingDep(std::make_pair(this->count, n), depOP);

        return std::make_pair(this->count, n);
    }

    /**
     * \brief perform a non-blocking message sending operation
     * \param [in] the number of sending buffer elements (bufsize) of integer datatype of each send buffer element
     * \param [in] destination rank (dst) of integer datatype
     */
    inline DisCosTiC::Event Isend(DisCosTiC_Datatype bufsize, DisCosTiC_Datatype dst, DisCosTiC::Event depOP)
    {
        this->sendCount++;
        this->count++;

        std::stringstream ss;
        ss << "l" << this->count << ": send " << bufsize << "b to " << dst << " tag " << curtag << std::endl;
        AppendString(ss.str(), content);

        InsertSrcDest();
        // return this->count;

        DisCosTiC::AST_OP *n = addNode();

        n->type = DisCosTiC::Operation_t::SEND;
        n->target = dst;
        n->tag = curtag;
        n->node = 0 /*node*/;
        n->network = 0 /*network*/;
        n->bufSize = bufsize;

        MaxCPU(0 /*node*/);
        Maxnetwork(0 /*network*/);

        this->mode = DisCosTiC::Mode_t::NONBLOCKING;
        nonBlockingDep(std::make_pair(this->count, n), depOP);

        return std::make_pair(this->count, n);
    }

    /**
     * \brief perform a non-blocking message sending operation
     * \param [in] the number of sending buffer elements (bufsize) of integer datatype of each send buffer element
     * \param [in] destination rank (dst) of integer datatype
     */

    inline DisCosTiC::Event Isend(const void *buf, DisCosTiC_Datatype bufsize, MPI_Datatype datatype, DisCosTiC_Datatype dst, int tag,
                                  MPI_Comm comm, MPI_Request *request, DisCosTiC::Event depOP)
    {
        this->sendCount++;
        this->count++;
        bytes_to_send = bufsize;
        std::stringstream ss;
        ss << "l" << this->count << ": send " << bufsize << "b to " << dst << " tag " << tag << std::endl;
        AppendString(ss.str(), content);
        // std::cout << ss.str() << std::endl;
        InsertSrcDest();
        // return this->count;

        DisCosTiC::AST_OP *n = addNode();

        n->type = DisCosTiC::Operation_t::SEND;
        n->target = dst;
        n->tag = tag;
        n->node = 0 /*node*/;
        n->network = 0 /*network*/;
        n->bufSize = bufsize;

        MaxCPU(0 /*node*/);
        Maxnetwork(0 /*network*/);

        this->mode = DisCosTiC::Mode_t::NONBLOCKING;
        nonBlockingDep(std::make_pair(this->count, n), depOP);

        return std::make_pair(this->count, n);
    }

    /**
     * \brief perform a message receiving operation
     * \param [in] the maximum number of receiving buffer elements (bufsize: message size in bytes) of integer datatype of each send buffer element
     * \param [in] source rank (src) of integer datatype
     */
    inline DisCosTiC::Event Recv(DisCosTiC_Datatype bufsize, DisCosTiC_Datatype src, DisCosTiC::Event depOP)
    {
        this->recvCount++;
        this->count++;

        std::stringstream ss;
        ss << "l" << this->count << ": recv " << bufsize << "b from " << src << " tag " << curtag << std::endl;
        AppendString(ss.str(), content);

        InsertSrcDest();
        // return this->count;
        DisCosTiC::AST_OP *n = addNode();

        n->type = DisCosTiC::Operation_t::RECV;
        n->target = src;
        n->tag = curtag;
        n->node = 0 /*node*/;
        n->network = 0 /*network*/;
        n->bufSize = bufsize;

        MaxCPU(0 /*node*/);
        Maxnetwork(0 /*network*/);

        this->mode = DisCosTiC::Mode_t::BLOCKING;
        blockingDep(std::make_pair(this->count, n), depOP);

        return std::make_pair(this->count, n);
    }

    /**
     * \brief perform a message receiving operation
     * \param [in] the maximum number of receiving buffer elements (bufsize: message size in bytes) of integer datatype of each send buffer element
     * \param [in] source rank (src) of integer datatype
     */
    inline DisCosTiC::Event Recv(const void *buf, DisCosTiC_Datatype bufsize, MPI_Datatype datatype, DisCosTiC_Datatype src, int tag,
                                 MPI_Comm comm, MPI_Request *request, DisCosTiC::Event depOP)
    {
        this->recvCount++;
        this->count++;

        std::stringstream ss;
        ss << "l" << this->count << ": recv " << bufsize << "b from " << src << " tag " << curtag << std::endl;
        AppendString(ss.str(), content);

        InsertSrcDest();
        // return this->count;
        DisCosTiC::AST_OP *n = addNode();

        n->type = DisCosTiC::Operation_t::RECV;
        n->target = src;
        n->tag = curtag;
        n->node = 0 /*node*/;
        n->network = 0 /*network*/;
        n->bufSize = bufsize;

        MaxCPU(0 /*node*/);
        Maxnetwork(0 /*network*/);

        this->mode = DisCosTiC::Mode_t::BLOCKING;
        blockingDep(std::make_pair(this->count, n), depOP);

        return std::make_pair(this->count, n);
    }

    /**
     * \brief perform a non-blocking message receiving operation
     * \param [in] the maximum number of receiving buffer elements (bufsize: message size in bytes) of integer datatype of each send buffer element
     * \param [in] source rank (src) of integer datatype
     */
    inline DisCosTiC::Event Irecv(DisCosTiC_Datatype bufsize, DisCosTiC_Datatype src, DisCosTiC::Event depOP)
    {
        this->recvCount++;
        this->count++;

        std::stringstream ss;
        ss << "l" << this->count << ": recv " << bufsize << "b from " << src << " tag " << curtag << std::endl;
        AppendString(ss.str(), content);

        InsertSrcDest();
        // return this->count;
        DisCosTiC::AST_OP *n = addNode();

        n->type = DisCosTiC::Operation_t::RECV;
        n->target = src;
        n->tag = curtag;
        n->node = 0 /*node*/;
        n->network = 0 /*network*/;
        n->bufSize = bufsize;

        MaxCPU(0 /*node*/);
        Maxnetwork(0 /*network*/);

        this->mode = DisCosTiC::Mode_t::NONBLOCKING;
        nonBlockingDep(std::make_pair(this->count, n), depOP);

        return std::make_pair(this->count, n);
    }

    /**
     * \brief perform a non-blocking message receiving operation
     * \param [in] the maximum number of receiving buffer elements (bufsize: message size in bytes) of integer datatype of each send buffer element
     * \param [in] source rank (src) of integer datatype
     */
    inline DisCosTiC::Event Irecv(const void *buf, DisCosTiC_Datatype bufsize, MPI_Datatype datatype, DisCosTiC_Datatype src, int tag,
                                  MPI_Comm comm, MPI_Request *request, DisCosTiC::Event depOP)
    {
        this->recvCount++;
        this->count++;

        std::stringstream ss;
        ss << "l" << this->count << ": recv " << bufsize << "b from " << src << " tag " << tag << std::endl;
        AppendString(ss.str(), content);
        // std::cout << ss.str() << std::endl;
        InsertSrcDest();
        // return this->count;
        DisCosTiC::AST_OP *n = addNode();

        n->type = DisCosTiC::Operation_t::RECV;
        n->target = src;
        n->tag = tag;
        n->node = 0 /*node*/;
        n->network = 0 /*network*/;
        n->bufSize = bufsize;

        MaxCPU(0 /*node*/);
        Maxnetwork(0 /*network*/);

        this->mode = DisCosTiC::Mode_t::NONBLOCKING;
        nonBlockingDep(std::make_pair(this->count, n), depOP);

        return std::make_pair(this->count, n);
    }

    /**
     * \brief perform computation operation with blocking communication routines
     * \param [in] name of computational phase (opname) of string datatype
     * \param [in] time of computational phase (bufSize) of DisCosTiC_Timetype datatype
     * \param [in] node number of multinode system (node) of integer datatype
     */
    inline DisCosTiC::Event Exec(std::string opname, DisCosTiC::Event depOP1, UserInterface::YAMLParser YAML_args, DisCosTiC_Datatype process_Rank, DisCosTiC_Datatype N_size_Of_Cluster, MPI_Comm comm)
    {
        DisCosTiC_Timetype runTime = 0;
        // std::cout<<process_Rank<<" : "<<opname<<std::endl;
        execNodeLVL(opname, YAML_args, &runTime, process_Rank, N_size_Of_Cluster, comm);
        // std::cout<<"Runtime : "<<runTime<<std::endl;
        //  TODO What shall I do with this drunken Sailor!
        //  if(this->func  != opname)
        //	    throw std::runtime_error("computation operation NOT YET IMPLEMENTED\n");

        this->compCount++;
        this->count++;
        this->execsize = runTime;
        // std::cout<<"Execsize : "<<this->execsize<<std::endl;

        std::stringstream ss;
        ss << "l" << this->count << ": calc " << execsize * timeunit_conv /* bufSize*/ << " node " << node << std::endl;
        AppendString(ss.str(), content);

        // return this->count;
        DisCosTiC::AST_OP *n = addNode();

        n->type = DisCosTiC::Operation_t::COMP;
        n->target = 0;
        n->tag = 0;
        n->node = 0; // node;
        n->network = 0 /*network*/;
        n->bufSize = execsize * timeunit_conv; // bufSize;

        MaxCPU(0 /*node*/);
        Maxnetwork(0 /*network*/);

        this->mode = DisCosTiC::Mode_t::BLOCKING;
        blockingDep(std::make_pair(this->count, n), depOP1);

        return std::make_pair(this->count, n);
    }

    /**
     * \brief perform computation operation with non-blocking communication routines
     * \param [in] name of computational phase (opname) of string datatype
     * \param [in] time of computational phase (bufSize) of DisCosTiC_Timetype datatype
     * \param [in] node number of multinode system (node) of integer datatype
     */
    inline DisCosTiC::Event Iexec(std::string opname, UserInterface::YAMLParser YAML_args, DisCosTiC::Event depOP1, DisCosTiC_Datatype process_Rank, DisCosTiC_Datatype N_size_Of_Cluster, MPI_Comm comm)
    {
        DisCosTiC_Timetype runTime = 0;
        execNodeLVL(opname, YAML_args, &runTime, process_Rank, N_size_Of_Cluster, comm);
        // TODO What shall I do with this drunken Sailor!
        // if(this->func  != opname)
        //	    throw std::runtime_error("computation operation NOT YET IMPLEMENTED\n");
        this->compCount++;
        this->count++;
        this->execsize = runTime;

        std::stringstream ss;
        ss << "l" << this->count << ": calc " << execsize * timeunit_conv /*bufSize*/ << " node " << node << std::endl;
        AppendString(ss.str(), content);

        // return this->count;
        DisCosTiC::AST_OP *n = addNode();

        n->type = DisCosTiC::Operation_t::COMP;
        n->target = 0;
        n->tag = 0;
        n->node = 0; // node;
        n->network = 0 /*network*/;
        n->bufSize = execsize * timeunit_conv; // bufSize;

        MaxCPU(0 /*node*/);
        Maxnetwork(0 /*network*/);

        this->mode = DisCosTiC::Mode_t::NONBLOCKING;
        nonBlockingDep(std::make_pair(this->count, n), depOP1);
        //  nonBlockingDep(std::make_pair (this->count,n), depOP2);

        return std::make_pair(this->count, n);
    }

    inline void execNodeLVL(std::string &opname, UserInterface::YAMLParser YAML_args, DisCosTiC_Timetype *rt, DisCosTiC_Datatype process_Rank, DisCosTiC_Datatype N_size_Of_Cluster, MPI_Comm comm)
    {
        DisCosTiC_Timetype perf_est;
        DisCosTiC_Timetype runtime = 0;
        std::string::size_type n = opname.find(":");
        if (opname.substr(0, n) == "LBL")
        {
            opname.erase(0, n + 1);
            std::string::size_type opos = opname.find(":");
            std::string::size_type extrasPos = opname.find("//");

            std::string extras = "-extras" + opname.substr(extrasPos + 2, opname.length());

            std::string ECM_core = extras.erase(0, 7);
            ECM_core = extras.substr(0, extras.find("+"));

            if (this->func != opname.substr(opos + 1, extrasPos - opos - 1))
                throw std::runtime_error("computation operation NOT YET IMPLEMENTED\n");

            if (process_Rank == 0 && !barrier)
            {
                system(("for num in {0.." + std::to_string(N_size_Of_Cluster) + "}; do cp config.cfg config$num.cfg; done").c_str());
                system(("for num in {0.." + std::to_string(N_size_Of_Cluster) + "}; do cp ./nodelevel/configs/" + opname.substr(opos + 1, extrasPos - opos - 1) + ".cfg ./nodelevel/configs/" + opname.substr(opos + 1, extrasPos - opos - 1) + "$num.cfg; done").c_str());
            }

            if (!barrier)
            {
                MPI_Barrier(comm);
                barrier = true;
            }

            UserInterface::ConfigParser NodeCFG("./nodelevel/configs/" + opname.substr(opos + 1, extrasPos - opos - 1) + std::to_string(process_Rank) + ".cfg");
            UserInterface::ConfigParser config("./config" + std::to_string(process_Rank) + ".cfg");

            NodeModel NM(&config, YAML_args, &NodeCFG, ECM_core);

            estimation(NM, &perf_est, &runtime);
        }
        else if (opname.substr(0, n) == "COMP")
        {

            opname = opname.substr(n + 1, opname.length());

            std::string extras = "-extras" + opname.substr(opname.find("//") + 2, opname.length());

            std::string ECM_core = extras.erase(0, 7);
            ECM_core = extras.substr(0, extras.find("+"));

            if (process_Rank == 0 && !barrier)
            {
                system(("for num in {0.." + std::to_string(N_size_Of_Cluster) + "}; do cp config.cfg config$num.cfg; done").c_str());
            }
            if (!barrier)
            {
                MPI_Barrier(comm);
                barrier = true;
            }

            UserInterface::ConfigParser NodeCFG("./config" + std::to_string(process_Rank) + ".cfg");

            std::string::size_type begin = system_number == 0 ? opname.find("TOL") + 4 : opname.find("TOLSecondary") + 13;
            std::string::size_type end = opname.find("||", begin);
            DisCosTiC_Timetype T_OL = std::stod(opname.substr(begin, end - begin));

            begin = system_number == 0 ? opname.find("TnOL") + 5 : opname.find("TnOLSecondary") + 14;
            end = opname.find("|", begin);
            DisCosTiC_Timetype T_nOL = std::stod(opname.substr(begin, end - begin));

            begin = system_number == 0 ? opname.find("TL1L2") + 6 : opname.find("TL1L2Secondary") + 15;
            end = opname.find("|", begin);
            DisCosTiC_Timetype T_L1L2 = std::stod(opname.substr(begin, end - begin));

            begin = system_number == 0 ? opname.find("TL2L3") + 6 : opname.find("TL2L3Secondary") + 15;
            end = opname.find("|", begin);
            DisCosTiC_Timetype T_L2L3 = std::stod(opname.substr(begin, end - begin));

            begin = system_number == 0 ? opname.find("TL3Mem") + 7 : opname.find("TL3MemSecondary") + 16;
            end = opname.find("|", begin);
            DisCosTiC_Timetype T_L3Mem = std::stod(opname.substr(begin, end - begin));

            DisCosTiC_Timetype T_ECM = std::max(T_OL, T_nOL + T_L1L2 + T_L2L3 + T_L3Mem);
            NodeModel NM(&NodeCFG, YAML_args, T_OL, T_nOL, T_L1L2, T_L2L3, T_L3Mem, T_ECM, std::stod(ECM_core));

            DisCosTiC_Timetype *scaling_performance = (double *)malloc(NM.getMachine().cores_per_socket_ * sizeof(double));
            DisCosTiC_Timetype *scaling_numa = (double *)malloc(NM.getMachine().cores_per_socket_ * sizeof(double));

            scaling(NM, scaling_performance, scaling_numa);

            NM.setMultiCore(scaling_performance[std::stoi(ECM_core) - 1]);

            estimation(NM, &perf_est, &runtime);
        }
        else if (opname.substr(0, n) == "SRC")
        {
            opname.erase(0, n + 1);
            std::string::size_type opos = opname.find(":");
            std::string::size_type machinePos = opname.find("//", opos);

            if (this->func != opname.substr(opos + 1, machinePos - opos - 1))
                throw std::runtime_error("computation operation NOT YET IMPLEMENTED\n");

            std::string output = "nodelevel/configs/" + opname.substr(opos + 1, machinePos - opos - 1) + std::to_string(process_Rank) + ".cfg";

            std::string::size_type corePos = opname.find("//", machinePos + 2);
            std::string::size_type definePos = opname.find("//", corePos + 2);

            std::string extras = "-extras" + opname.substr(corePos + 2, definePos - corePos - 2);

            std::string ECM_core = extras.erase(0, 7);
            ECM_core = extras.substr(0, extras.find("+"));

            if (!kerncraftExecuted && process_Rank == 0)
            {
                kerncraftExecuted = true;
                std::string output = "nodelevel/configs/" + opname.substr(opos + 1, machinePos - opos - 1) + ".cfg";

                system(("rm " + output).c_str());

                std::string::size_type pos = opname.find("//BREAK:");
                std::string source = "nodelevel/kernels/" + opname.substr(opos + 1, machinePos - opos - 1) + ".c";
                std::transform(source.begin(), source.end(), source.begin(), ::tolower);

                std::string::size_type opos = opname.find(":");
                std::string::size_type machinePos = opname.find("//", opos);
                std::string::size_type corePos = opname.find("//", machinePos + 2);
                std::string::size_type definePos = opname.find("//", corePos + 2);

                std::string script("./kerncraftintegration/diskern.py");
                std::string model("-pECM");
                std::string outputPath = "-o" + output;
                std::string machine = "-m" + opname.substr(machinePos + 2, corePos - machinePos - 2);
                std::string extras = "-extras" + opname.substr(corePos + 2, definePos - corePos - 2);
                std::string cores = "-c" + std::to_string(cores_per_socket);
                std::string defines = opname.substr(definePos + 2, opname.length());
                std::string system_no = "-systemNo" + std::to_string(system_number);

                std::ofstream sF;
                sF.open(source);
                sF << opname.substr(0, pos) << std::endl;
                sF.close();
                char scriptPath[script.length() + 1];
                char modelType[model.length() + 1];
                char outputFile[outputPath.length() + 1];
                char machineFile[machine.length() + 1];
                char coreNumber[cores.length() + 1];
                char keyValues[defines.length() + 1];
                char sourceFile[source.length() + 1];
                char extrasVal[extras.length() + 1];
                char system_number_val[system_no.length() + 1];

                std::strcpy(scriptPath, script.c_str());
                std::strcpy(modelType, model.c_str());
                std::strcpy(outputFile, outputPath.c_str());
                std::strcpy(machineFile, machine.c_str());
                std::strcpy(coreNumber, cores.c_str());
                std::strcpy(keyValues, defines.c_str());
                std::strcpy(sourceFile, source.c_str());
                std::strcpy(extrasVal, extras.c_str());
                std::strcpy(system_number_val, system_no.c_str());

                std::vector<char *> argVek{scriptPath, modelType, machineFile, coreNumber, outputFile, extrasVal, system_number_val};

                char *token = std::strtok(keyValues, " ");
                while (token != NULL)
                {
                    argVek.push_back(token);
                    token = std::strtok(NULL, " ");
                }
                argVek.push_back(sourceFile);
                char *argV[argVek.size() + 1];
                for (std::size_t i = 0; i < argVek.size(); ++i)
                {
                    argV[i] = argVek[i];
                }
                argV[argVek.size()] = NULL;
                executeKerncraft(argV, argVek.size());

                system(("for num in {0.." + std::to_string(N_size_Of_Cluster) + "}; do cp config.cfg config$num.cfg; done").c_str());
                system(("for num in {0.." + std::to_string(N_size_Of_Cluster) + "}; do cp ./nodelevel/configs/" + opname.substr(opos + 1, machinePos - opos - 1) + ".cfg ./nodelevel/configs/" + opname.substr(opos + 1, machinePos - opos - 1) + "$num.cfg; done").c_str());
            }

            if (!barrier)
            {
                MPI_Barrier(comm);
            }

            if (!kerncraftExecuted &&
                process_Rank == (CFG_args.getValue<DisCosTiC_Datatype>("secondary_number_of_processes") + 2) &&
                (CFG_args.getValue<DisCosTiC_Datatype>("heteregeneous") == 1))
            {
                kerncraftExecuted = true;
                std::string output = "nodelevel/configs/" + opname.substr(opos + 1, machinePos - opos - 1) + ".cfg";

                system(("rm " + output).c_str());

                std::string::size_type pos = opname.find("//BREAK:");
                std::string source = "nodelevel/kernels/" + opname.substr(opos + 1, machinePos - opos - 1) + ".c";
                std::transform(source.begin(), source.end(), source.begin(), ::tolower);

                std::string::size_type opos = opname.find(":");
                std::string::size_type machinePos = opname.find("//", opos);
                std::string::size_type corePos = opname.find("//", machinePos + 2);
                std::string::size_type definePos = opname.find("//", corePos + 2);

                std::string script("./kerncraftintegration/diskern.py");
                std::string model("-pECM");
                std::string outputPath = "-o" + output;
                std::string machine = "-m" + opname.substr(machinePos + 2, corePos - machinePos - 2);
                std::string extras = "-extras" + opname.substr(corePos + 2, definePos - corePos - 2);
                std::string cores = "-c" + std::to_string(cores_per_socket);
                std::string defines = opname.substr(definePos + 2, opname.length());
                std::string system_no = "-systemNo" + std::to_string(system_number);

                std::ofstream sF;
                sF.open(source);
                sF << opname.substr(0, pos) << std::endl;
                sF.close();
                char scriptPath[script.length() + 1];
                char modelType[model.length() + 1];
                char outputFile[outputPath.length() + 1];
                char machineFile[machine.length() + 1];
                char coreNumber[cores.length() + 1];
                char keyValues[defines.length() + 1];
                char sourceFile[source.length() + 1];
                char extrasVal[extras.length() + 1];
                char system_number_val[system_no.length() + 1];

                std::strcpy(scriptPath, script.c_str());
                std::strcpy(modelType, model.c_str());
                std::strcpy(outputFile, outputPath.c_str());
                std::strcpy(machineFile, machine.c_str());
                std::strcpy(coreNumber, cores.c_str());
                std::strcpy(keyValues, defines.c_str());
                std::strcpy(sourceFile, source.c_str());
                std::strcpy(extrasVal, extras.c_str());
                std::strcpy(system_number_val, system_no.c_str());

                std::vector<char *> argVek{scriptPath, modelType, machineFile, coreNumber, outputFile, extrasVal, system_number_val};

                char *token = std::strtok(keyValues, " ");
                while (token != NULL)
                {
                    argVek.push_back(token);
                    token = std::strtok(NULL, " ");
                }
                argVek.push_back(sourceFile);
                char *argV[argVek.size() + 1];
                for (std::size_t i = 0; i < argVek.size(); ++i)
                {
                    argV[i] = argVek[i];
                }
                argV[argVek.size()] = NULL;
                executeKerncraft(argV, argVek.size());

                system(("for num in {0.." + std::to_string(N_size_Of_Cluster) + "}; do cp config.cfg config$num.cfg; done").c_str());
                system(("for num in {0.." + std::to_string(N_size_Of_Cluster) + "}; do cp ./nodelevel/configs/" + opname.substr(opos + 1, machinePos - opos - 1) + ".cfg ./nodelevel/configs/" + opname.substr(opos + 1, machinePos - opos - 1) + "$num.cfg; done").c_str());
            }

            if (!barrier_hetero && heteregeneous_mode == 1)
            {
                MPI_Barrier(comm);
                barrier_hetero = true;
            }

            UserInterface::ConfigParser config("./config" + std::to_string(process_Rank) + ".cfg");
            UserInterface::ConfigParser NodeCFG(output);

            // scaling_cores = NodeCFG.getValue<DisCosTiC_Datatype>("Scaling_cores");

            NodeModel NM(&config, YAML_args, &NodeCFG, ECM_core);
            // std::cout<<process_Rank<<" "<<NM.getECM().T_MECM_<<std::endl;
            estimation(NM, &perf_est, &runtime);
        }
        else if (opname.substr(0, n) == "FILE")
        {

            opname.erase(0, n + 1);
            std::string::size_type opos = opname.find(":");
            std::string::size_type machinePos = opname.find("//", opos);

            if (this->func != opname.substr(opos + 1, machinePos - opos - 1))
                throw std::runtime_error("computation operation NOT YET IMPLEMENTED\n");

            std::string output = "nodelevel/configs/" + opname.substr(opos + 1, machinePos - opos - 1) + std::to_string(process_Rank) + ".cfg";

            std::string::size_type corePos = opname.find("//", machinePos + 2);
            std::string::size_type definePos = opname.find("//", corePos + 2);

            std::string extras = "-extras" + opname.substr(corePos + 2, definePos - corePos - 2);

            std::string ECM_core = extras.erase(0, 7);
            ECM_core = extras.substr(0, extras.find("+"));

            std::string::size_type pos = opname.find("//BREAK:");

            std::string source = "nodelevel/kernels/" + opname.substr(0, pos);

            std::ifstream file;

            file.open(source);

            if (!file)
            {
                if (process_Rank == 0)
                    std::cout << "File does not exists ! " << source;

                file.close();
                MPI_Finalize();
                exit(0);
            }
            file.close();

            if (!kerncraftExecuted && process_Rank == 0)
            {
                kerncraftExecuted = true;
                std::string output = "nodelevel/configs/" + opname.substr(opos + 1, machinePos - opos - 1) + ".cfg";

                system(("rm nodelevel/configs/" + opname.substr(opos + 1, machinePos - opos - 1) + ".cfg").c_str());

                std::string::size_type opos = opname.find(":");
                std::string::size_type machinePos = opname.find("//", opos);
                std::string::size_type corePos = opname.find("//", machinePos + 2);
                std::string::size_type definePos = opname.find("//", corePos + 2);

                std::string script("./kerncraftintegration/diskern.py");
                std::string model("-pECM");
                std::string outputPath = "-o" + output;
                std::string machine = "-m" + opname.substr(machinePos + 2, corePos - machinePos - 2);
                std::string extras = "-extras" + opname.substr(corePos + 2, definePos - corePos - 2);
                std::string cores = "-c" + std::to_string(cores_per_socket);
                std::string defines = opname.substr(definePos + 2, opname.length());
                std::string system_no = "-systemNo" + std::to_string(system_number);

                char scriptPath[script.length() + 1];
                char modelType[model.length() + 1];
                char outputFile[outputPath.length() + 1];
                char machineFile[machine.length() + 1];
                char coreNumber[cores.length() + 1];
                char keyValues[defines.length() + 1];
                char sourceFile[source.length() + 1];
                char extrasVal[extras.length() + 1];
                char system_number_val[system_no.length() + 1];

                std::strcpy(scriptPath, script.c_str());
                std::strcpy(modelType, model.c_str());
                std::strcpy(outputFile, outputPath.c_str());
                std::strcpy(machineFile, machine.c_str());
                std::strcpy(coreNumber, cores.c_str());
                std::strcpy(keyValues, defines.c_str());
                std::strcpy(sourceFile, source.c_str());
                std::strcpy(extrasVal, extras.c_str());
                std::strcpy(system_number_val, system_no.c_str());

                std::vector<char *> argVek{scriptPath, modelType, machineFile, coreNumber, outputFile, extrasVal, system_number_val};
                char *token = std::strtok(keyValues, " ");
                while (token != NULL)
                {
                    argVek.push_back(token);
                    token = std::strtok(NULL, " ");
                }
                argVek.push_back(sourceFile);
                char *argV[argVek.size() + 1];
                for (std::size_t i = 0; i < argVek.size(); ++i)
                {
                    argV[i] = argVek[i];
                }
                argV[argVek.size()] = NULL;

                executeKerncraft(argV, argVek.size());

                system(("for num in {0.." + std::to_string(N_size_Of_Cluster) + "}; do cp config.cfg config$num.cfg; done").c_str());
                system(("for num in {0.." + std::to_string(N_size_Of_Cluster) + "}; do cp ./nodelevel/configs/" + opname.substr(opos + 1, machinePos - opos - 1) + ".cfg ./nodelevel/configs/" + opname.substr(opos + 1, machinePos - opos - 1) + "$num.cfg; done").c_str());
            }

            if (!barrier)
            {
                MPI_Barrier(comm);
                barrier = true;
            }

            if (!kerncraftExecuted &&
                process_Rank == (CFG_args.getValue<DisCosTiC_Datatype>("secondary_number_of_processes") + 2) &&
                (CFG_args.getValue<DisCosTiC_Datatype>("heteregeneous") == 1))
            {
                kerncraftExecuted = true;
                std::string output = "nodelevel/configs/" + opname.substr(opos + 1, machinePos - opos - 1) + ".cfg";

                std::string::size_type pos = opname.find("//BREAK:");
                std::string source = "nodelevel/kernels/" + opname.substr(0, pos);

                std::string::size_type opos = opname.find(":");
                std::string::size_type machinePos = opname.find("//", opos);
                std::string::size_type corePos = opname.find("//", machinePos + 2);
                std::string::size_type definePos = opname.find("//", corePos + 2);

                std::string script("./kerncraftintegration/diskern.py");
                std::string model("-pECM");
                std::string outputPath = "-o" + output;
                std::string machine = "-m" + opname.substr(machinePos + 2, corePos - machinePos - 2);
                std::string extras = "-extras" + opname.substr(corePos + 2, definePos - corePos - 2);
                std::string cores = "-c" + std::to_string(cores_per_socket);
                std::string defines = opname.substr(definePos + 2, opname.length());
                std::string system_no = "-systemNo" + std::to_string(system_number);

                char scriptPath[script.length() + 1];
                char modelType[model.length() + 1];
                char outputFile[outputPath.length() + 1];
                char machineFile[machine.length() + 1];
                char coreNumber[cores.length() + 1];
                char keyValues[defines.length() + 1];
                char sourceFile[source.length() + 1];
                char extrasVal[extras.length() + 1];
                char system_number_val[system_no.length() + 1];

                std::strcpy(scriptPath, script.c_str());
                std::strcpy(modelType, model.c_str());
                std::strcpy(outputFile, outputPath.c_str());
                std::strcpy(machineFile, machine.c_str());
                std::strcpy(coreNumber, cores.c_str());
                std::strcpy(keyValues, defines.c_str());
                std::strcpy(sourceFile, source.c_str());
                std::strcpy(extrasVal, extras.c_str());
                std::strcpy(system_number_val, system_no.c_str());

                std::vector<char *> argVek{scriptPath, modelType, machineFile, coreNumber, outputFile, extrasVal, system_number_val};
                char *token = std::strtok(keyValues, " ");
                while (token != NULL)
                {
                    argVek.push_back(token);
                    token = std::strtok(NULL, " ");
                }
                argVek.push_back(sourceFile);
                char *argV[argVek.size() + 1];
                for (std::size_t i = 0; i < argVek.size(); ++i)
                {
                    argV[i] = argVek[i];
                }
                argV[argVek.size()] = NULL;

                executeKerncraft(argV, argVek.size());

                system(("for num in {0.." + std::to_string(N_size_Of_Cluster) + "}; do cp config.cfg config$num.cfg; done").c_str());
                system(("for num in {0.." + std::to_string(N_size_Of_Cluster) + "}; do cp ./nodelevel/configs/" + opname.substr(opos + 1, machinePos - opos - 1) + ".cfg ./nodelevel/configs/" + opname.substr(opos + 1, machinePos - opos - 1) + "$num.cfg; done").c_str());
            }

            if (!barrier_hetero && heteregeneous_mode == 1)
            {
                MPI_Barrier(comm);
                barrier_hetero = true;
            }

            UserInterface::ConfigParser config("./config" + std::to_string(process_Rank) + ".cfg");
            UserInterface::ConfigParser NodeCFG(output);

            // scaling_cores = NodeCFG.getValue<DisCosTiC_Datatype>("Scaling_cores");

            NodeModel NM(&config, YAML_args, &NodeCFG, ECM_core);

            estimation(NM, &perf_est, &runtime);
        }
        else
        {
            // TODO Add error message
            abort();
        }
        *rt = runtime * 1e9;
        return;
    }
    /**
     * \brief perform computation operation
     * \param [in] name of computational phase (opname) of string datatype
     * \param [in] time of computational phase (bufSize) of DisCosTiC_Timetype datatype
     */
    /* inline DisCosTiC_Indextype Exec(std::string opname, DisCosTiC_Timetype bufSize) {
         std::vector<vec3T> buf;
         vec3T elem = vec3T(1, 1, bufSize);
         buf.push_back(elem);

         this->compCount++;
         this->count++;

         std::stringstream ss;
         ss << "l" << this->count << ": calc ";
         DisCosTiC_Datatype bufSize=0;
         for (unsigned DisCosTiC_Datatype i = 0; i < buf.bufSize(); i++) {
             bufSize += buf[i].bufSize;
         }
         ss << bufSize << std::endl;
         AppendString(ss.str(),content);

         return this->count;
     }*/

    /**
     * \brief dependency on blocking send/recv operation that satisfied with start of an operation.
     * Dependency means that a can not be executed before b is finished
     * \param [in] src of DisCosTiC_Indextype datatype
     * \param [in] dest of DisCosTiC_Indextype datatype
     */
    inline void blocking(DisCosTiC_Indextype src, DisCosTiC_Indextype dest)
    {
        this->depCount++;

        std::stringstream ss;
        ss << "l" << src << " requires l" << dest << std::endl;

        EraseSrcDest(src, dest);
        AppendString(ss.str(), content);

        // std::cout << dest << "\t" << +toCharPointer(dest) << std::endl;
        this->insertDep(depTable, toCharPointer(src), toCharPointer(dest));
        DisCosTiC::AST_OP *a = this->retrieveID(indicesTable, toCharPointer(src));
        DisCosTiC::AST_OP *b = this->retrieveID(indicesTable, toCharPointer(dest));
        /*std::cout << "src: " ;
        print_DeserialNodeT(src);
        std::cout << "dst: " ;
        print_DeserialNodeT(dest);*/
        b->DepOperations.push_back(a);
        a->depCount++;
        edgesCount++;
    }

    inline void blockingDep(DisCosTiC::Event a, DisCosTiC::Event b)
    {

        // std::cout << "this->count : " << this->count << " a.first = " << a.first << " b.first = " << b.first << std::endl;

        if (a.first != INVALID_ID)
        {
            this->insertID(indicesTable, toCharPointer(a.first), a.second);
        }
        if (b.first != INVALID_ID)
        {
            this->blocking(a.first, b.first); // if(DisCosTiC::Mode_t::NONBLOCKING)
        }
    }

    /**
     * \brief dependency on non-blocking send/recv operation that satisfied with start of an operation while link to wait/test loop
     * StartDependency means that a can not be executed before b is started
     * \param [in] src of DisCosTiC_Indextype datatype
     * \param [in] dest of DisCosTiC_Indextype datatype
     */
    inline void nonBlocking(DisCosTiC_Indextype src, DisCosTiC_Indextype dest)
    {
        this->depCount++;

        std::stringstream ss;
        ss << "l" << src << " irequires l" << dest << std::endl;

        EraseSrcDest(src, dest);
        AppendString(ss.str(), content);

        // std::cout << dest << "\t" << +toCharPointer(dest) << std::endl;
        this->insertDep(depTable, toCharPointer(src), toCharPointer(dest));
        DisCosTiC::AST_OP *a = this->retrieveID(indicesTable, toCharPointer(src));
        DisCosTiC::AST_OP *b = this->retrieveID(indicesTable, toCharPointer(dest));
        /*std::cout << "src: " ;
        print_DeserialNodeT(src);
        std::cout << "dst: " ;
        print_DeserialNodeT(dest);*/
        b->IdepOperations.push_back(a);
        a->depCount++;
        edgesCount++;
    }

    inline void nonBlockingDep(DisCosTiC::Event a, DisCosTiC::Event b)
    {
        if (a.first != INVALID_ID)
        {
            this->insertID(indicesTable, toCharPointer(a.first), a.second);
        }
        if (b.first != INVALID_ID)
        {
            this->nonBlocking(a.first, b.first); // if(DisCosTiC::Mode_t::NONBLOCKING)
        }
    }

    /**
     *\brief DEBUG: printing table that consists of node id with node type
     */
    inline void print_indicesTable()
    {
        for (auto rank : DisCosTiC::getRange(rankCount))
        {
            for (std::map<std::string, DisCosTiC::AST_OP *>::iterator itr = indicesTable->begin(); itr != indicesTable->end(); ++itr)
            {
                std::cout << '\t' << itr->first << '\t';
                print_DeserialNodeT(+itr->second);
            }
        }
    }

    inline void print_indicesDeserializedTable()
    {
        for (auto rank : DisCosTiC::getRange(rankCount))
        {
            for (std::map<std::string, DisCosTiC::AST_OP_TYPE>::iterator itr = indicesDeserializedTable->begin(); itr != indicesDeserializedTable->end(); ++itr)
            {
                std::cout << '\t' << itr->first << '\t';
                print_DeserialNodeNonPointerT(+itr->second);
            }
        }
    }

    /**
     *\brief DEBUG: printing table that consist of respective node dependencies
     */
    inline void print_depTable()
    {
        for (auto rank : DisCosTiC::getRange(rankCount))
        {
            for (std::map<std::string, std::string>::iterator itr = depTable->begin(); itr != depTable->end(); ++itr)
            {
                std::cout << '\t' << itr->first << '\t' << itr->second << '\n';
                // char *cstr1 = new char[itr->first.length() + 1];
                // strcpy(cstr1, itr->first.c_str());
                // char *cstr2 = new char[itr->second.length() + 1];
                // strcpy(cstr2, itr->second.c_str());
                // std::map<std::string, DisCosTiC::AST_OP*>::iterator srcit = indicesTable->find(itr->first);
                // std::map<std::string, DisCosTiC::AST_OP*>::iterator dstit = indicesTable->find(itr->second);
                // this->Dependency( this->retrieveID(indicesTable, itr->first ), this->retrieveID(indicesTable, itr->second) );
                // delete [] cstr1;
                // delete [] cstr2;
            }
        }
    }

    /**
     * \brief end of each process runtime with closing loop
     */
    inline void Rank_Finalize()
    {
        for (std::vector<DisCosTiC::AST_OP *>::iterator it = this->allNodes.begin(); it != this->allNodes.end(); it++)
        { // TODO: added to AST destructor
            delete *it;
        }
        AppendString("}\n", content);
    }

    /**
     * \brief write to output file
     */
    inline void File_Write()
    {
        if (myfile.is_open())
        {
            myfile << this->content;
            this->content.clear();
            myfile.sync();
        }
        else
        {
            std::cerr << "ERROR: missing result file definition of " << this->filename << " for writing" << std::endl;
        }
    }
};

#endif //!< end AST_HPP
