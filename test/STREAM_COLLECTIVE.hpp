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
			//for collective mode, force single timestep
			DisCosTiC_Datatype collective_mode = CFG_args->getValue<DisCosTiC_Datatype>("collective_mode");
			if (collective_mode == 1) {
				numTimesteps = 1;
			} else {
				numTimesteps = CFG_args->getValue<DisCosTiC_Datatype>("number_of_timesteps");
			}
			nodesCount = 0;
			networksCount = 0;
		}

		/*DisCosTiC::tupleIdNodePair*/
		DisCosTiC::idNodeTypePair Oneway_PositiveDisplacement(AST *DisCosTiC, DisCosTiC_Datatype process_Rank, UserInterface::YAMLParser YAML_args, DisCosTiC_Datatype N_size_Of_Cluster, MPI_Comm comm)
		{
			DisCosTiC::Event recv, send, comp, collective;
			DisCosTiC::idNodeTypePair ID(systemsize);
			// DisCosTiC::AST_OP root_Node;
			// DisCosTiC::idNodePair compIDs(systemsize), sendIDs(systemsize), recvIDs(systemsize);

			long int N_ = system_number == 0 ? CFG_args.getValue<long int>("N") : CFG_args.getValue<long int>("secondary_N");

			int Nlocal = system_number == 0 ? N_ / primary_processes : N_ / secondary_processes;

			int task_per_node_ = task_per_node;
			int core = process_Rank;
			int remainder_cores = systemsize > task_per_node_ ? systemsize % task_per_node_ : 0;
			int ECM_core = 1;

			assert(task_per_node_ <= (YAML_args.cores_per_chip * YAML_args.chips_per_node));

			if (task_per_node_ != 1)
			{
				if (systemsize <= task_per_node_)
				{
					Nlocal = Nlocal * std::min(task_per_node_, cores_per_socket);
					ECM_core = std::min(task_per_node_, cores_per_socket);
				}
				else if (remainder_cores == 0)
				{
					Nlocal = Nlocal * std::min(task_per_node_, cores_per_socket);
					ECM_core = std::min(task_per_node_, cores_per_socket);
				}
				else
				{
					if (core <= systemsize - remainder_cores)
					{
						Nlocal = Nlocal * std::min(task_per_node_, cores_per_socket);
						ECM_core = std::min(task_per_node_, cores_per_socket);
					}
					else
					{
						Nlocal = Nlocal * remainder_cores;
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
				collective.first = INVALID_ID;

				//for collectives: computation (STREAM) followed by collective
				//only single timestep (collective_mode = 1)
				
				comp = DisCosTiC->Exec("FILE:stream-triad.c//BREAK:" + CFG_args.getValue<std::string>("benchmark_kernel") + 
				                       "//./nodelevel/machine-files/" + arch_name + ".yml//" + 
				                       std::to_string(ECM_core) + "+" + std::to_string(remainder_cores) + 
				                       "//-D N " + std::to_string(Nlocal) + " -D  s " + std::to_string(55), 
				                       recv, YAML_args, process_Rank, N_size_Of_Cluster, comm);
				
				comp.second->bufSize = (comp.second->bufSize * Nlocal);

				DisCosTiC_Datatype root = 0;
				DisCosTiC_Datatype coll_type_id = CFG_args.getValue<DisCosTiC_Datatype>("collective_type");
				DisCosTiC_Datatype coll_algo = CFG_args.getValue<DisCosTiC_Datatype>("collective_algorithm");
				DisCosTiC_Datatype coll_bytes = CFG_args.getValue<DisCosTiC_Datatype>("collective_bytes");
				DisCosTiC::CollectiveType coll_type = static_cast<DisCosTiC::CollectiveType>(coll_type_id);

				//collective DSEL — type/algorithm/bytes from config.cfg
				collective = DisCosTiC->Collective
							(
								coll_type,
								root, coll_bytes,
								rank, systemsize, comp,
								static_cast<uint8_t>(coll_algo)
							);

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
					N.mode = (**it).mode;
					N.algorithm = (**it).algorithm;
					N.collectiveType = (**it).collectiveType;
					N.collectiveRoot = (**it).collectiveRoot;
					N.collectiveNumRanks = (**it).collectiveNumRanks;
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
			DisCosTiC->File_Write();
			// delete DisCosTiC;
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
		uint8_t
		GetNumCores()
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
		// AST *DisCosTiC;
		DisCosTiC::VecDeserialNode Nodes;

		DisCosTiC_Datatype datasize;
		DisCosTiC_Datatype numTimesteps;
	}; //!< end of Benchmark class

	using VecGraph_t = std::vector<Benchmark>;

} //!< end namespace DisCosTiC

#endif //!< end P2P_HPP
