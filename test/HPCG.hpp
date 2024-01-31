#ifndef P2P_HPP
#define P2P_HPP
#include <tuple>
#include <mpi.h>
#include <string>

#include "../include/CompModel.hpp"
#include "../include/AST.hpp"
#include "../include/DataStruct.hpp"

extern int scaling_cores;
extern int bytes_to_send;

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

namespace DisCosTiC
{

    class Benchmark
    {

    public:
        /**
         * \brief constructor that initializes the coordinates
         * \param CFG_args
         */
        Benchmark(UserInterface::ConfigParser *CFG_args)
        {
            systemsize = heteregeneous_mode == 0 ? primary_processes : primary_processes + secondary_processes;
            numTimesteps = CFG_args->getValue<DisCosTiC_Datatype>("number_of_iterations");
            nodesCount = 0;
            networksCount = 0;
        }

        /*DisCosTiC::tupleIdNodePair*/
        DisCosTiC::idNodeTypePair Oneway_PositiveDisplacement(DisCosTiC_Datatype process_Rank, UserInterface::YAMLParser YAML_args, DisCosTiC_Datatype N_size_Of_Cluster, MPI_Comm comm)
        {
            DisCosTiC = new AST(&CFG_args, systemsize);
            DisCosTiC::Event recv, send, comp;
            DisCosTiC::idNodeTypePair ID(systemsize);

            long int dim_x = system_number == 0 ? CFG_args.getValue<long int>("dim_x") : CFG_args.getValue<long int>("secondary_dim_x");
            long int dim_y = system_number == 0 ? CFG_args.getValue<long int>("dim_y") : CFG_args.getValue<long int>("secondary_dim_y");

            int local_system_size = system_number == 0 ? primary_processes : secondary_processes;

            int dim_y_local = dim_y / local_system_size;

            int task_per_node_ = task_per_node;
            int core = process_Rank;

            int remainder_cores = local_system_size > task_per_node_ ? local_system_size % task_per_node_ : 0;
            int ECM_core = 1;

            assert(task_per_node_ <= (YAML_args.cores_per_chip * YAML_args.chips_per_node));

            if (task_per_node_ != 1)
            {
                if (local_system_size <= task_per_node_)
                {
                    dim_y_local = dim_y_local * std::min(task_per_node_, cores_per_socket);
                    ECM_core = std::min(task_per_node_, cores_per_socket);
                }
                else if (remainder_cores == 0)
                {
                    dim_y_local = dim_y_local * std::min(task_per_node_, cores_per_socket);
                    ECM_core = std::min(task_per_node_, cores_per_socket);
                }
                else
                {
                    if (core <= local_system_size - remainder_cores)
                    {
                        dim_y_local = dim_y_local * std::min(task_per_node_, cores_per_socket);
                        ECM_core = std::min(task_per_node_, cores_per_socket);
                    }
                    else
                    {
                        dim_y_local = dim_y_local * remainder_cores;
                        ECM_core = remainder_cores;
                    }
                }
            }

            for (auto rank : DisCosTiC::getRange(systemsize))
            {
                DisCosTiC->Rank_Init(rank);
                DisCosTiC->SetNumRanks(systemsize); // std::cout <<DisCosTiC->ranks<< std::endl; //18
                recv.first = INVALID_ID;
                send.first = INVALID_ID;
                comp.first = INVALID_ID;
                /*compIDs[rank].resize(numTimesteps);
                sendIDs[rank].resize(numTimesteps);
                recvIDs[rank].resize(numTimesteps);*/

                double src[dim_x][dim_y_local];
                MPI_Request requests[4] = {MPI_REQUEST_NULL, MPI_REQUEST_NULL, MPI_REQUEST_NULL, MPI_REQUEST_NULL};
					for (int i = 0; i < numberOfCalls; ++i)
					{
						ierr = CG_ref(A, data, b, x, refMaxIters, tolerance, niters, normr, normr0, &ref_times[0], true);
						const SparseMatrix &A = A;
						CGData &data = data;
						const Vector &b = b;
						Vector &x = x;
						const int max_iter = refMaxIters;
						const double tolerance = tolerance;
						int &niters = niters;
						double &normr = normr;
						double &normr0 = normr0;
						double *times = &ref_times[0];
						bool doPreconditioning = true;
						normr = 0.0;
						double rtz = 0.0, oldrtz = 0.0, alpha = 0.0, beta = 0.0, pAp = 0.0;
						double t0 = 0.0, t1 = 0.0, t2 = 0.0, t3 = 0.0, t4 = 0.0, t5 = 0.0;
						local_int_t nrow = A.localNumberOfRows;
						Vector &r = data.r;
						Vector &z = data.z;
						Vector &p = data.p;
						Vector &Ap = data.Ap;
                        DisCosTiC->Exec("FILE:WAXPBY.c//BREAK:" + CFG_args.getValue<std::string>("benchmark_kernel") + "//./nodelevel/machine-files/" + arch_name + ".yml//" + std::to_string(ECM_core) + "+" + std::to_string(remainder_cores) "//-D nrow " + 100 + " -D  b " + 100 "//-D Ap " + 100 + " -D  r " + 100, recv, YAML_args, process_Rank, N_size_Of_Cluster, comm);
                        
						DisCosTiC->Exec("FILE:SCALAR-PRODUCT.c//BREAK:" + CFG_args.getValue<std::string>("benchmark_kernel") + "//./nodelevel/machine-files/" + arch_name + ".yml//" + std::to_string(ECM_core) + "+" + std::to_string(remainder_cores) "//-D nrow " + 100 + " -D  normr " + 100 "//-D t4 " + 100 + " -D  r " + 100, recv, YAML_args, process_Rank, N_size_Of_Cluster, comm);
						normr0 = normr;
						for (int k = 1; k <= max_iter && normr / normr0 > tolerance; k++)
						{
							if (doPreconditioning)

								const SparseMatrix &A = A;
							const Vector &r = r;
							Vector &x = z;
							int ierr = 0;
							if (A.mgData != 0)
							{
								int numberOfPresmootherSteps = A.mgData->numberOfPresmootherSteps;
								DisCosTiC->Exec("FILE:SOR.c//BREAK:" + CFG_args.getValue<std::string>("benchmark_kernel") + "//./nodelevel/machine-files/" + arch_name + ".yml//" + std::to_string(ECM_core) + "+" + std::to_string(remainder_cores) "//-D " + 100, recv, YAML_args, process_Rank, N_size_Of_Cluster, comm);
                                
								DisCosTiC->Exec("FILE:Restriction.c//BREAK:" + CFG_args.getValue<std::string>("benchmark_kernel") + "//./nodelevel/machine-files/" + arch_name + ".yml//" + std::to_string(ECM_core) + "+" + std::to_string(remainder_cores) "//-D A " + 100 + " -D  r " + 100, recv, YAML_args, process_Rank, N_size_Of_Cluster, comm);

								DisCosTiC->Exec("FILE:Prolongation.c//BREAK:" + CFG_args.getValue<std::string>("benchmark_kernel") + "//./nodelevel/machine-files/" + arch_name + ".yml//" + std::to_string(ECM_core) + "+" + std::to_string(remainder_cores) "//-D A " + 100 + " -D  r " + 100, recv, YAML_args, process_Rank, N_size_Of_Cluster, comm);
                                
								int numberOfPostsmootherSteps = A.mgData->numberOfPostsmootherSteps;
								DisCosTiC->Exec("FILE:SOR.c//BREAK:" + CFG_args.getValue<std::string>("benchmark_kernel") + "//./nodelevel/machine-files/" + arch_name + ".yml//" + std::to_string(ECM_core) + "+" + std::to_string(remainder_cores) "//-D " + 100, recv, YAML_args, process_Rank, N_size_Of_Cluster, comm);
							}
							else
							{
								DisCosTiC->Exec("FILE:SOR.c//BREAK:" + CFG_args.getValue<std::string>("benchmark_kernel") + "//./nodelevel/machine-files/" + arch_name + ".yml//" + std::to_string(ECM_core) + "+" + std::to_string(remainder_cores) "//-D A " + 100 + " -D  r " + 100 + " -D  x " + 100, recv, YAML_args, process_Rank, N_size_Of_Cluster, comm);
							}

							else DisCosTiC->Exec("FILE:WAXPBY.c//BREAK:" + CFG_args.getValue<std::string>("benchmark_kernel") + "//./nodelevel/machine-files/" + arch_name + ".yml//" + std::to_string(ECM_core) + "+" + std::to_string(remainder_cores) "//-D nrow " + 100 + "//-D z " + 100 + " -D  r " + 100, recv, YAML_args, process_Rank, N_size_Of_Cluster, comm);
							if (k == 1)
							{
								DisCosTiC->Exec("FILE:SCALAR-PRODUCT.c//BREAK:" + CFG_args.getValue<std::string>("benchmark_kernel") + "//./nodelevel/machine-files/" + arch_name + ".yml//" + std::to_string(ECM_core) + "+" + std::to_string(remainder_cores) "//-D nrow " + 100 + " -D  rtz " + 100 "//-D z " + 100 + " -D  r " + 100 + " -D  t4 " + 100, recv, YAML_args, process_Rank, N_size_Of_Cluster, comm);
							}
							else
							{
								oldrtz = rtz;
								DisCosTiC->Exec("FILE:SCALAR-PRODUCT.c//BREAK:" + CFG_args.getValue<std::string>("benchmark_kernel") + "//./nodelevel/machine-files/" + arch_name + ".yml//" + std::to_string(ECM_core) + "+" + std::to_string(remainder_cores) "//-D nrow " + 100 + " -D  rtz " + 100 "//-D z " + 100 + " -D  r " + 100 + " -D  t4 " + 100, recv, YAML_args, process_Rank, N_size_Of_Cluster, comm);
								
                                beta = rtz / oldrtz;
								DisCosTiC->Exec("FILE:WAXPBY.c//BREAK:" + CFG_args.getValue<std::string>("benchmark_kernel") + "//./nodelevel/machine-files/" + arch_name + ".yml//" + std::to_string(ECM_core) + "+" + std::to_string(remainder_cores) "//-D nrow " + 100 + "//-D z " + 100 + " -D  beta " + 100 + " -D  p " + 100, recv, YAML_args, process_Rank, N_size_Of_Cluster, comm);
							}
							
                            DisCosTiC->Exec("FILE:SCALAR-PRODUCT.c//BREAK:" + CFG_args.getValue<std::string>("benchmark_kernel") + "//./nodelevel/machine-files/" + arch_name + ".yml//" + std::to_string(ECM_core) + "+" + std::to_string(remainder_cores) "//-D nrow " + 100 + " -D  p " + 100 "//-D Ap " + 100 + " -D  pAp " + 100 + " -D  t4 " + 100, recv, YAML_args, process_Rank, N_size_Of_Cluster, comm);
							alpha = rtz / pAp;
							
                            DisCosTiC->Exec("FILE:WAXPBY.c//BREAK:" + CFG_args.getValue<std::string>("benchmark_kernel") + "//./nodelevel/machine-files/" + arch_name + ".yml//" + std::to_string(ECM_core) + "+" + std::to_string(remainder_cores) "//-D nrow " + 100 + "//-D x " + 100 + " -D  alpha " + 100 + " -D  p " + 100, recv, YAML_args, process_Rank, N_size_Of_Cluster, comm);
							
                            DisCosTiC->Exec("FILE:WAXPBY.c//BREAK:" + CFG_args.getValue<std::string>("benchmark_kernel") + "//./nodelevel/machine-files/" + arch_name + ".yml//" + std::to_string(ECM_core) + "+" + std::to_string(remainder_cores) "//-D nrow " + 100 + "//-D r " + 100 + " -D  alpha " + 100 + " -D  Ap " + 100 + " -D  r " + 100, recv, YAML_args, process_Rank, N_size_Of_Cluster, comm);
                            
							DisCosTiC->Exec("FILE:SCALAR-PRODUCT.c//BREAK:" + CFG_args.getValue<std::string>("benchmark_kernel") + "//./nodelevel/machine-files/" + arch_name + ".yml//" + std::to_string(ECM_core) + "+" + std::to_string(remainder_cores) "//-D nrow " + 100 + " -D r " + 100 "//-D normr " + 100 + " -D  t4 " + 100, recv, YAML_args, process_Rank, N_size_Of_Cluster, comm);
							niters = k;
						}
						if (ierr)
							++err_count;
						totalNiters_ref += niters;
					}

				}										//!< end of loop over time step
            numOperations = DisCosTiC->getNumOps(); // DisCosTiC->allNodes.size(); //15
            ID[rank].resize(numOperations);

            /**
             * \brief find the root nodes (private)
             */
            for (std::vector<DisCosTiC::AST_OP *>::iterator it = DisCosTiC->allNodes.begin(); it != DisCosTiC->allNodes.end(); it++)
            { // DisCosTiC->allNodes.size():15
                if ((**it).depCount == 0)
                    DisCosTiC->RootNodes.push_back(*it); // DisCosTiC->RootNodes.size():1
            }

            DisCosTiC_Datatype count = 0;
            DisCosTiC::AST_OP_ N;

            for (std::vector<DisCosTiC::AST_OP *>::iterator it = DisCosTiC->allNodes.begin(); it != DisCosTiC->allNodes.end(); it++)
            {
                N.depCount = (**it).depCount;
                N.type = (**it).type;
                N.target = (**it).target;
                N.bufSize = (**it).bufSize;
                N.tag = (**it).tag;
                N.node = (**it).node;
                N.network = (**it).network;
                N.label = (**it).label;
                N.depsCount = (**it).DepOperations.size();
                N.depApdxStartLabel = count;
                count += (**it).DepOperations.size();
                N.idepsCount = (**it).IdepOperations.size();
                N.idepApdxStartLabel = count;
                count += (**it).IdepOperations.size();
                // DisCosTiC->insertdeserialID(indicesDeserializedTable, toCharPointer(rank) , N);

                if (process_Rank == 0)
                {
                    std::cout << "";
                }
                ID[rank][(**it).label] = std::make_pair(rank, N);
            }

            DisCosTiC->Rank_Finalize();
        } //!< end of loop over rank

        DisCosTiC->File_Write();
        delete DisCosTiC;
        /*print_pairedVec_NonPointer2T(ID);
        print_pairedVec2T(compIDs);
        print_pairedVec2T(sendIDs);
        print_pairedVec2T(recvIDs);
        DisCosTiC->print_indicesTable();
        DisCosTiC->print_depTable();
        DisCosTiC->print_indicesDeserializedTable();*/
        return ID; // std::make_tuple(compIDs,sendIDs,recvIDs);  //(root_Node,*send.second,*recv.second);
    }               //!< end of Oneway_PositiveDisplacement

    /**
     * \brief the maximum number of the nodes
     */
    uint8_t GetNumCores()
    {
        return nodesCount + 1;
    }

    /**
     * \brief the maximum number of the network interface controller
     */
    uint8_t GetNumNetworks()
    {
        return networksCount + 1;
    }

    /**
     * \brief destructor
     */
    ~Benchmark()
    {
    }

public:
    DisCosTiC_Datatype nodesCount;
    DisCosTiC_Datatype networksCount;
    DisCosTiC_Datatype systemsize;
    DisCosTiC_Datatype numOperations;

private:
    AST *DisCosTiC;
    DisCosTiC::VecDeserialNode Nodes;

    DisCosTiC_Datatype datasize;
    DisCosTiC_Datatype numTimesteps;
}; //!< end of Benchmark class

using VecGraph_t = std::vector<Benchmark>;

} //!< end namespace DisCosTiC

#endif //!< end P2P_HPP
