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

				comp = DisCosTiC->Exec("FILE:heat.c//BREAK:" + CFG_args.getValue<std::string>("benchmark_kernel") + "//./nodelevel/machine-files/" + arch_name + ".yml//" + std::to_string(ECM_core) + "+" + std::to_string(remainder_cores) + "//-D dim_x " + std::to_string(dim_x) + " -D  dim_y " + std::to_string(dim_y_local), recv, YAML_args, process_Rank, N_size_Of_Cluster, comm);

				comp.second->bufSize = (comp.second->bufSize * dim_x * dim_y_local);
				// std::cout<<"Runtime : "<<comp.second->bufSize<<std::endl;
				if (rank > 0)
				{
					int bottom = rank - 1;
					// std::cout<<"Who is sending : "<<rank<<" to whom : "<<bottom<<std::endl;
					send = DisCosTiC->Isend(&src[0], dim_x * 8, MPI_DOUBLE, bottom, 2, MPI_COMM_WORLD, &requests[2], comp); // [TODO: replace dim_x by dim_x*8B]
					// std::cout<<"Who is receiving : "<<rank<<" from whom : "<<bottom<<std::endl;

					recv = DisCosTiC->Irecv(&src[0], dim_x * 8, MPI_DOUBLE, bottom, 1, MPI_COMM_WORLD, &requests[3], comp); // [TODO: replace dim_x by dim_x*8B]
				}
				if (rank + 1 < systemsize)
				{
					int top = rank + 1;
					// std::cout<<"Who is sending : "<<rank<<" to whom : "<<top<<std::endl;
					send = DisCosTiC->Isend(&src[(dim_y - 1) * dim_x], dim_x * 8, MPI_DOUBLE, top, 1, MPI_COMM_WORLD, &requests[0], comp); // [TODO: replace dim_x by dim_x*8B]
					// std::cout<<"Who is receiving : "<<rank<<" from whom : "<<top<<std::endl;
					recv = DisCosTiC->Irecv(&src[(dim_y - 1) * dim_x], dim_x * 8, MPI_DOUBLE, top, 2, MPI_COMM_WORLD, &requests[1], comp); // [TODO: replace dim_x by dim_x*8B]
				}
				// Simulating dummy send and recv for allgather
				if (rank + 1 == systemsize)
				{
					int top = rank + 1;

					send = DisCosTiC->Isend(&rank, 0, MPI_DOUBLE, top, 1, MPI_COMM_WORLD, &requests[0], comp);
					recv = DisCosTiC->Irecv(&rank, 0, MPI_DOUBLE, top, 2, MPI_COMM_WORLD, &requests[1], comp);
				}
				if (rank == 0)
				{
					int bottom = rank - 1;

					send = DisCosTiC->Isend(&rank, 0, MPI_DOUBLE, bottom, 2, MPI_COMM_WORLD, &requests[2], comp);
					recv = DisCosTiC->Irecv(&rank, 0, MPI_DOUBLE, bottom, 1, MPI_COMM_WORLD, &requests[3], comp);
				}

				//!< end of loop over time step
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
		}			   //!< end of Oneway_PositiveDisplacement

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
