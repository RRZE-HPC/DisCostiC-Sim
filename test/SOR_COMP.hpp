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
			systemsize = CFG_args->getValue<DisCosTiC_Datatype>("number_of_processes");
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
			// DisCosTiC::AST_OP root_Node;
			// DisCosTiC::idNodePair compIDs(systemsize), sendIDs(systemsize), recvIDs(systemsize);

			long int imax = system_number == 0 ? CFG_args.getValue<long int>("dim_x") : CFG_args.getValue<long int>("secondary_dim_x");
			long int jmax = system_number == 0 ? CFG_args.getValue<long int>("dim_y") : CFG_args.getValue<long int>("secondary_dim_y");

			int local_system_size = system_number == 0 ? primary_processes : secondary_processes;

			int jmaxLocal = imax / local_system_size;

			int task_per_node_ = task_per_node;
			int core = process_Rank;

			int remainder_cores = local_system_size > task_per_node_ ? local_system_size % task_per_node_ : 0;
			int ECM_core = 1;

			double src[imax + 2][jmaxLocal + 2];
			double dst[imax + 2][jmaxLocal + 2];

            assert(task_per_node_ <= (YAML_args.cores_per_chip * YAML_args.chips_per_node));

            if (task_per_node_ != 1)
            {
                if (local_system_size <= task_per_node_)
                {
                    jmaxLocal = jmaxLocal * std::min(task_per_node_, cores_per_socket);
                    ECM_core = std::min(task_per_node_, cores_per_socket);
                }
                else if (remainder_cores == 0)
                {
                    jmaxLocal = jmaxLocal * std::min(task_per_node_, cores_per_socket);
                    ECM_core = std::min(task_per_node_, cores_per_socket);
                }
                else
                {
                    if (core <= local_system_size - remainder_cores)
                    {
                        jmaxLocal = jmaxLocal * std::min(task_per_node_, cores_per_socket);
                        ECM_core = std::min(task_per_node_, cores_per_socket);
                    }
                    else
                    {
                        jmaxLocal = jmaxLocal * remainder_cores;
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

				// std::cout << "core : " << core << " task per node : " << task_per_node << " cores per socket : " << YAML_args.cores_per_chip << " imax : " << imax << " jmaxLocal : " << jmaxLocal << " ECM_core : "<<ECM_core<<std::endl;

				MPI_Request requests[4] = {MPI_REQUEST_NULL, MPI_REQUEST_NULL, MPI_REQUEST_NULL, MPI_REQUEST_NULL};

				comp = DisCosTiC->Exec("COMP:TOL=96.0||TnOL=1.0|TL1L2=4.0|TL2L3=12.0|TL3Mem=5.5|TOLSecondary=92.0||TnOLSecondary=1.0|TL1L2Secondary=4.0|TL2L3Secondary=12.0|TL3MemSecondary=5.5//" + std::to_string(ECM_core) + "+" + std::to_string(remainder_cores), recv, YAML_args, process_Rank, N_size_Of_Cluster, comm);

				// std::cout<<"Offered cycles : "<<comp.second->bufSize<<std::endl;

				comp.second->bufSize = (comp.second->bufSize * imax * jmaxLocal) + CFG_args.getValue<DisCosTiC_Datatype>("penalty");
				// std::cout<<"Time : "<<comp.second->bufSize<<std::endl;
				int top = (rank == systemsize - 1 ? 0 : rank + 1);

				if (rank + 1 < systemsize)
				{
					int top = rank + 1;
					send = DisCosTiC->Isend(&src[(jmaxLocal) * (imax + 2) + 1], imax * 8, MPI_DOUBLE, top, 1, MPI_COMM_WORLD, &requests[0], comp);
					recv = DisCosTiC->Irecv(&dst[(jmaxLocal + 1) * (imax + 2) + 1], imax * 8, MPI_DOUBLE, top, 2, MPI_COMM_WORLD, &requests[1], comp);
				}
				int bottom = (rank == 0 ? systemsize - 1 : rank - 1);

				if (rank > 0)
				{
					int bottom = rank - 1;
					send = DisCosTiC->Isend(&src[(imax + 2) + 1], imax * 8, MPI_DOUBLE, bottom, 2, MPI_COMM_WORLD, &requests[2], comp);
					recv = DisCosTiC->Irecv(&dst[1], imax * 8, MPI_DOUBLE, bottom, 1, MPI_COMM_WORLD, &requests[3], comp);
				}

				// Simulating dummy send and recv for allgather
				if (rank + 1 == systemsize)
				{

					send = DisCosTiC->Isend(&rank, 0, MPI_DOUBLE, top, 1, MPI_COMM_WORLD, &requests[0], comp);
					recv = DisCosTiC->Irecv(&rank, 0, MPI_DOUBLE, top, 2, MPI_COMM_WORLD, &requests[1], comp);
				}
				if (rank == 0)
				{
					send = DisCosTiC->Isend(&rank, 0, MPI_DOUBLE, bottom, 2, MPI_COMM_WORLD, &requests[2], comp);
					recv = DisCosTiC->Irecv(&rank, 0, MPI_DOUBLE, bottom, 1, MPI_COMM_WORLD, &requests[3], comp);
				}

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
					ID[rank][(**it).label] = std::make_pair(rank, N);
				}

				DisCosTiC->Rank_Finalize();
			} //!< end of loop over rank
			// DisCosTiC->File_Write();
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
