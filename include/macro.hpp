#ifndef MACRO_HPP
#define MACRO_HPP

#pragma once

#include "DataType.hpp" //!< data types
#include <iostream>
static const DisCosTiC_Indextype INVALID_ID = -1;
static const DisCosTiC_Indextype MPI_ANY_SOURC = ~0;
static const DisCosTiC_Indextype MPI_ANY_TA = ~0;

/*
 * \brief return the print of the 1 dimensional vector
 * \return print 1D vector
 */
#define print_vec1T(vec)                               \
	{                                                  \
		std::cout << "{";                              \
		for (auto i : DisCosTiC::getRange(vec.size())) \
			std::cout << (i ? " , " : "") << vec[i];   \
		std::cout << " }" << std::endl;                \
	}

/*
 * \brief return the print of the 2 dimensional vector
 * \return print 2D vector
 */
#define print_vec2T(vec)                                                              \
	{                                                                                 \
		for (auto i : DisCosTiC::getRange(vec[0].size()))                             \
		{                                                                             \
			for (auto j : DisCosTiC::getRange(vec[i].size()))                         \
			{                                                                         \
				std::cout << "[" << i << "][" << j << "]=" << vec[i][j] << std::endl; \
			}                                                                         \
		}                                                                             \
	}

/*
 * \brief return the print of the 2 dimensional paired vector
 * \return print 2D paired vector
 */
#define print_pairedVec2T(vec)                                                        \
	{                                                                                 \
		for (auto i : DisCosTiC::getRange(vec.size()))                                \
		{                                                                             \
			for (auto j : DisCosTiC::getRange(vec[i].size()))                         \
			{                                                                         \
				std::cout << "[" << i << "][" << j << "]:"                            \
						  << "\t label = " << vec[i][j].first << "\t AST_OP info : "; \
				print_DeserialNodeT(+vec[i][j].second);                               \
			}                                                                         \
		}                                                                             \
	}

#define print_pairedVecNonPointer2T(vec)                                              \
	{                                                                                 \
		for (auto i : DisCosTiC::getRange(vec.size()))                                \
			for (auto j : DisCosTiC::getRange(vec[i].size()))                         \
				std::cout << "[" << i << "][" << j << "]:"                            \
						  << "\t label = " << vec[i][j].first << "\t AST_OP info : "; \
		print_DeserialNodeNonPointerT(+vec[i][j].second);                             \
	}                                                                                 \
	}                                                                                 \
	}

#define print_pairedVec_NonPointer2T(vec)                                             \
	{                                                                                 \
		for (auto i : DisCosTiC::getRange(vec.size()))                                \
		{                                                                             \
			for (auto j : DisCosTiC::getRange(vec[i].size()))                         \
			{                                                                         \
				std::cout << "[" << i << "][" << j << "]:"                            \
						  << "\t label = " << vec[i][j].first << "\t AST_OP info : "; \
				print_AST_OP_NonPointerT(+vec[i][j].second);                          \
			}                                                                         \
		}                                                                             \
	}

/*
 * \brief the print of the first value of 2 dimensional paired vector
 * \return print/read second value of 2D paired vector
 */
#define itFirst_Vec2T(vec)                           \
	{                                                \
		for (auto &row : vec)                        \
		{                                            \
			for (auto &col : row)                    \
				std::cout << col.first << std::endl; \
		}                                            \
	}
/*for(vector< vector<int> >::iterator row = vec.begin(); row != vec.end(); ++row) {
	for(vector<int>::iterator col = row->begin(); col != row->end(); ++col) {
		std::cout << *col.first << std::endl;    }
}*/

/*
 * \brief the print of the second value of 2 dimensional paired vector
 * \return read second value of 2D paired vector
 */
#define iqueueOpecond_Vec2T(vec)  \
	{                             \
		for (auto &row : vec)     \
		{                         \
			for (auto &col : row) \
				col.second;       \
		}                         \
	}

/*
 * \brief Return the print of the 3 dimensional vector
 * \return print 3D vector
 */
#define print_vec3T(vec)                                                                                  \
	{                                                                                                     \
		for (auto i : DisCosTiC::getRange(vec[0].size()))                                                 \
		{                                                                                                 \
			for (auto j : DisCosTiC::getRange(vec[i].size()))                                             \
			{                                                                                             \
				for (auto k : DisCosTiC::getRange(vec[j].size()))                                         \
				{                                                                                         \
					std::cout << "[" << i << "][" << j << "][" << k << "]=" << vec[i][j][k] << std::endl; \
				}                                                                                         \
			}                                                                                             \
		}                                                                                                 \
	}

/*
 * \brief print of the Operation struct type
 */
// for(DisCosTiC::VecOperation::iterator op=operations.begin(); op != operations.end(); ++op) {
#define print_OpPropertiesT(op)                               \
	{                                                         \
		std::cout << "time: " << op->time                     \
				  << "	starttime: " << op->starttime         \
				  << "	syncstart: " << op->syncstart         \
				  << "	numOpsInQueue: " << op->numOpsInQueue \
				  << "	bufSize: " << op->bufSize             \
				  << "	target: " << op->target               \
				  << "	rank: " << op->rank                   \
				  << "	label: " << op->label                 \
				  << "	tag: " << op->tag                     \
				  << "	node: " << +op->node                  \
				  << "	network: " << +op->network            \
				  << "	type: " << +op->type << std::endl;    \
	}

/*
 * \brief print of the non-pointer Operation struct type
 */
#define print_OpPropertiesNonPointerT(op)                                \
	{                                                                    \
		std::cout << "	rank: " << op.rank                               \
				  << "    time: " << op.time                             \
				  << "	starttime: " << op.starttime                     \
				  << "	syncstart: " << op.syncstart                     \
				  << "	numOpsInQueue: " << op.numOpsInQueue             \
				  << "	bufSize: " << op.bufSize                         \
				  << "	target: " << op.target << "	label: " << op.label \
				  << "	tag: " << op.tag                                 \
				  << "	node: " << +op.node                              \
				  << "	network: " << +op.network                        \
				  << "	type: " << +op.type                              \
				  << " mode : " << +op.mode << std::endl;                \
	}

/*
 * \brief print of the DeserialNodeT struct type
 */
#define print_DeserialNodeT(op)                                        \
	{                                                                  \
		std::cout << "DepOperations: " << op->DepOperations.size()     \
				  << "	IdepOperations: " << op->IdepOperations.size() \
				  << "	depCount: " << op->depCount                    \
				  << "	target: " << op->target                        \
				  << "	label: " << op->label                          \
				  << "	tag: " << op->tag                              \
				  << "	node: " << +op->node                           \
				  << "	network: " << +op->network                     \
				  << "	type: " << +op->type                           \
				  << "	bufSize: " << op->bufSize                      \
				  << std::endl;                                        \
	}

/*
 * \brief print of the non-pointer DeserialNodeT struct type
 */
#define print_DeserialNodeNonPointerT(op)                                    \
	{                                                                        \
		std::cout << "DepOperations.size(): " << op.DepOperations.size()     \
				  << "	IdepOperations.size(): " << op.IdepOperations.size() \
				  << "	depCount: " << op.depCount                           \
				  << "	target: " << op.target                               \
				  << "	label: " << op.label                                 \
				  << "	tag: " << op.tag                                     \
				  << "	node: " << +op.node                                  \
				  << "	network: " << +op.network                            \
				  << "	type: " << +op.type                                  \
				  << "	bufSize: " << op.bufSize                             \
				  << std::endl;                                              \
	}

/*
 * \brief print of the non-pointer DeserialNodeT struct type
 */
#define print_AST_OP_NonPointerT(op)                                                       \
	{                                                                                      \
		std::cout << "num_deps: " << op.num_deps                                           \
				  << "	deps_startlabel_in_apdx: " << op.deps_startlabel_in_apdx           \
				  << "	num_startdeps.bufSize(): " << op.num_startdeps                     \
				  << "	startdeps_startlabel_in_apdx: " << op.startdeps_startlabel_in_apdx \
				  << "	depCount: " << op.depCount                                         \
				  << "	target: " << op.target                                             \
				  << "	label: " << op.label                                               \
				  << "	tag: " << op.tag                                                   \
				  << "	node: " << +op.node                                                \
				  << "	network: " << +op.network                                          \
				  << "	type: " << +op.type                                                \
				  << "	bufSize: " << op.bufSize                                           \
				  << std::endl;                                                            \
	}

/**
 * \brief open an open file
 */
#define fileopen(myfile, filename)                                        \
	{                                                                     \
		try                                                               \
		{                                                                 \
			myfile.open(filename);                                        \
		}                                                                 \
		catch (std::ifstream::failure const &e)                           \
		{                                                                 \
			std::cerr << "error: opening file definition of "             \
					  << this->filename << " for reading, with message '" \
					  << e.what() << std::endl;                           \
			std::exit(EXIT_FAILURE);                                      \
		}                                                                 \
	}

/**
 * \brief close an open file
 */
#define fileclose(myfile, filename)                                                                                 \
	{                                                                                                               \
		if (myfile.is_open())                                                                                       \
		{                                                                                                           \
			try                                                                                                     \
			{                                                                                                       \
				myfile.close();                                                                                     \
			}                                                                                                       \
			catch (std::ofstream::failure const &e)                                                                 \
			{                                                                                                       \
				std::cerr << "error: closing file definition of " << this->filename << " for writing" << std::endl; \
				std::exit(EXIT_FAILURE);                                                                            \
			}                                                                                                       \
		}                                                                                                           \
	}

/**
 * \brief append a string
 * \param string
 */
#define AppendString(str, content) \
	{                              \
		this->content.append(str); \
	}

/**
 * \brief append content to a string with label l
 * \param DisCosTiC_Indextype
 */
#define toCharPointer(content)                                      \
	{                                                               \
		const_cast<char *>(("l" + std::to_string(content)).c_str()) \
	}

/**
 * \brief find max value of a vector
 */
#define max_vec1T(vec)                                 \
	{                                                  \
		DisCosTiC_Datatype max = 0;                    \
		for (auto i : DisCosTiC::getRange(vec.size())) \
			max = std::max(max, vec[i].size());        \
		std::cout << max;                              \
	}

/**
 * \brief print progress
 */
#define progessPrint(progress, qCounter, q1, q2, q3)                                    \
	{                                                                                   \
		std::cout << "Discostic Progress: " << progress                                 \
				  << " % (queue counter: " << qCounter << ") ";                         \
		std::cout << "[Queue bufSize: " << q1.size() << " Max Receive Queue bufSize: "; \
		max_vec1T(q2);                                                                  \
		std::cout << " Max Unexpected Message Queue bufSize: ";                         \
		max_vec1T(q3);                                                                  \
		std::cout << "]" << std::endl;                                                  \
	}

/**
 * \brief check if all queues are empty
 */
#define queues_empty_check(check, q)                                                                     \
	for (auto rank : DisCosTiC::getRange(numRanks))                                                      \
	{                                                                                                    \
		if (!q[rank].empty())                                                                            \
		{                                                                                                \
			std::cerr << " queue on rank " << rank                                                       \
					  << " contains " << q[rank].size()                                                  \
					  << " operations!";                                                                 \
			for (DisCosTiC::ListqueueOp::iterator iter = q[rank].begin(); iter != q[rank].end(); ++iter) \
				std::cerr << "\t [ source ==> " << iter->src                                             \
						  << ", tag ==> " << iter->tag                                                   \
						  << "]" << std::endl;                                                           \
			check = false;                                                                               \
		}                                                                                                \
	} //!< end of for loop

/**
 * \brief print final time for all ranks
 */
#define allRanksTime(numRanks, nexto)                                                                 \
	std::cout << "\nFULL APPLICATION PERFORMANCE (for all ranks):" << std::endl;                      \
	std::cout << "----------------------------------------------------------------" << std::endl;     \
	std::cout << "rank            runtime [ns]            runtime [s]" << std::endl;                  \
	std::cout << "----------------------------------------------------------------" << std::endl;     \
	for (auto rank : DisCosTiC::getRange(numRanks))                                                   \
	{                                                                                                 \
		DisCosTiC_Timetype maxo = *(std::max_element(nexto[rank].begin(), nexto[rank].end()));        \
		std::cout << rank << "            	" << maxo << "          	" << maxo / 1e9 << std::endl; \
	}

/**
 * \brief print final maximum time for only rank taking maximum time
 */
#define slowRankTime(numRanks, nexto)                                                             \
	std::cout << "\nFULL APPLICATION PERFORMANCE (for slowest rank):" << std::endl;               \
	DisCosTiC_Timetype max = 0;                                                                   \
	DisCosTiC_Datatype rank = 0;                                                                  \
	for (auto rank : DisCosTiC::getRange(numRanks))                                               \
	{                                                                                             \
		DisCosTiC_Timetype maxo = *(std::max_element(nexto[rank].begin(), nexto[rank].end()));    \
		DisCosTiC_Timetype cur = maxo;                                                            \
		if (cur > max)                                                                            \
		{                                                                                         \
			rank = rank;                                                                          \
			max = cur;                                                                            \
		}                                                                                         \
	}                                                                                             \
	std::cout << "----------------------------------------------------------------" << std::endl; \
	std::cout << "rank            runtime [ns]            runtime [s]" << std::endl;              \
	std::cout << "----------------------------------------------------------------" << std::endl; \
	std::cout << rank << " \t\t " << max << " \t\t " << max / 1e9 << std::endl;

/**
 * \brief print verbose output for computational phases
 */
#define verboseCompInitPrint(rank, operation)        \
	std::cout << "initialize rank: " << rank         \
			  << " [ node: " << +operation.node      \
			  << ", network: " << +operation.network \
			  << "] computation: " << operation.bufSize << std::endl;

#define verboseCompFinalPrint(rank, operation)          \
	std::cout << "rank: " << rank                       \
			  << " [ node: " << +operation.node         \
			  << ", network: " << +operation.network    \
			  << "] computation: " << operation.bufSize \
			  << ", time: " << operation.time           \
			  << ", label: " << operation.label << std::endl;

#define verboseSendInitPrint(rank, operation)        \
	std::cout << "initialize rank: " << rank         \
			  << " [ node: " << +operation.node      \
			  << ", network: " << +operation.network \
			  << "] send to: " << operation.target   \
			  << "tag: " << operation.tag            \
			  << " bufSize: " << operation.bufSize << std::endl;

#define verboseSendFinalPrint(rank, operation)       \
	std::cout << "rank: " << rank                    \
			  << " [ node: " << +operation.node      \
			  << ", network: " << +operation.network \
			  << "] send to: " << operation.target   \
			  << ", tag: " << operation.tag          \
			  << ", bufSize: " << operation.bufSize  \
			  << ", time: " << operation.time        \
			  << ", label: " << operation.label << std::endl;

#define verboseRecvInitPrint(rank, operation)           \
	std::cout << "initialize rank: " << rank            \
			  << " [ node: " << +operation.node         \
			  << ", network: " << +operation.network    \
			  << "] receive from: " << operation.target \
			  << "tag: " << operation.tag               \
			  << " bufSize: " << operation.bufSize << std::endl;

#define verboseRecvFinalPrint(rank, operation)          \
	std::cout << "rank: " << rank                       \
			  << " [ node: " << +operation.node         \
			  << ", network: " << +operation.network    \
			  << "] receive from: " << operation.target \
			  << ", tag: " << operation.tag             \
			  << ", bufSize: " << operation.bufSize     \
			  << ", time: " << operation.time           \
			  << ", label: " << operation.label << std::endl;

#define verboseCompPrint(operation)                                   \
	{                                                                 \
		std::cout << "\n[rank: " << operation.rank                    \
				  << ", node: " << +operation.node                    \
				  << "]:" << std::endl;                               \
		std::cout << "local operation bufSize= " << operation.bufSize \
				  << " ns, total time= " << operation.time            \
				  << " ns" << std::endl;                              \
	}

#define verboseSendPrint(operation)                      \
	{                                                    \
		std::cout << "\n[rank: " << operation.rank       \
				  << ", node: " << +operation.node       \
				  << "]:" << std::endl;                  \
		std::cout << "send to rank " << operation.target \
				  << ", total time= " << operation.time  \
				  << " ns" << std::endl;                 \
	}

#define verboseSendIrequiresPrint(operation)                                   \
	{                                                                          \
		std::cout << "satisfy local non-blocking dependencies" << std::endl;   \
		std::cout << "send inserting msg to rank " << operation.rank           \
				  << ", total time= " << operation.time << " ns" << std::endl; \
	}

#define verboseEagerSendPrint(o, operation)                                                 \
	std::cout << "eager satisfy local requires at total time = " << operation.starttime + o \
			  << " ns" << std::endl;

#define verboseRendezvousSendPrint(operation)                      \
	std::cout << "start rendezvous message to: " << operation.rank \
			  << " (label:  " << operation.label                   \
			  << ")" << std::endl;

#define verboseRecvPrint(operation)                           \
	{                                                         \
		std::cout << "\n[rank: " << operation.rank            \
				  << ", node: " << +operation.node            \
				  << "]:" << std::endl;                       \
		std::cout << "receive from rank " << operation.target \
				  << " , total time= " << operation.time      \
				  << " ns" << std::endl;                      \
	}

#define verboseRendezvousRecvPrint(operation)                           \
	std::cout << "satisfy remote requires at rank " << operation.target \
			  << " (label: " << operation.label                         \
			  << ")" << std::endl;

#define verboseMsgPrint(operation, oSuccessor, grSuccessor)                                  \
	{                                                                                        \
		std::cout << "\n[rank: " << operation.rank                                           \
				  << ", node: " << +operation.node                                           \
				  << "]:" << std::endl;                                                      \
		std::cout << "receive msg from rank " << operation.target                            \
				  << ", total time= " << operation.time                                      \
				  << " ns" << std::endl;                                                     \
		std::cout << "o= " << oSuccessor[operation.rank][operation.node]                     \
				  << " ns, successor gr= " << grSuccessor[operation.rank][operation.network] \
				  << " ns" << std::endl;                                                     \
	}

#define help()                                                                                                                                                          \
	{                                             \
		version()                                                                                                                                                       \
std::cout << "\n====================================" << std::endl << " Arguments for ./discostic" << std::endl << "====================================" << std::endl; \
std::cout << "\t --version, -v" << "\t\t show simulator's version number and exit" << std::endl; \
std::cout << "\t --help, -h" << "\t\t show this help message and exit" << std::endl; \
std::cout                                                                                                                                               \
    << "\n====================================" << std::endl                                                                                        \
    << " Application model for config.cfg" << std::endl                                                                                                                 \
    << "====================================" << std::endl;                                                                                                                                               \
std::cout << "\t benchmark_kernel" << "\t name of the kernel used in the program" << std::endl; \
std::cout << "\t kernel_mode" << "\t\t mode of the kernel (FILE, SRC, LBL, COMP)" << std::endl; \
        std::cout                                                                                                                                               \
			<< "\n====================================" << std::endl                                                                                        \
			<< " Cluster model for config.cfg" << std::endl                                                                                                                 \
			<< "====================================" << std::endl;                                                                                                                                               \
std::cout << "\t heteregeneous" << "\t\t a bool flag to enable or disable the second system (1: enabled, 0: disabled)" << std::endl; \
std::cout << "\t number_of_iterations" << "\t number of iterations of the program" << std::endl; \
std::cout << "\t dim_x, dim_y, dim_z" << "\t problem size; high-dimensional parameters will be disregarded for low-dimensional problems." << std::endl; \
std::cout << "\t task_per_node" << "\t\t number of running processes on one node" << std::endl; \
std::cout << "\t number_of_processes" << "\t\t total number of running processes" << std::endl; \
std::cout                                                                                                                                               \
    << "\n====================================" << std::endl                                                                                        \
    << " Interconnect model for config.cfg" << std::endl                                                                                                                 \
    << "====================================" << std::endl;                                                                                                                                               \
std::cout << "\t interconnect" << "\t\t name of the interconnect" << std::endl; \
std::cout << "\t MPI_library" << "\t\t name of the MPI library for the first system (IntelMPI, WaitIO)" << std::endl; \
std::cout << "\t comm_model" << "\t\t performance model for communication (0: LogGP, 1: HOCKNEY)" << std::endl; \
std::cout << "\t waitio_mode" << "\t\t mode of the WaitIO MPI library (socket, file or hybrid)" << std::endl; \
std::cout                                                                                                                                               \
    << "\n====================================" << std::endl                                                                                        \
    << " Node model for config.cfg" << std::endl                                                                                                                 \
    << "====================================" << std::endl;                                                                                                                                               \
std::cout << "\t micro_architecture" << "\t name of the YAML machine file" << std::endl; \
std::cout << "\t compiler-flags" << "\t\t STD and SIMD optimizations for the first system (-03 -xCORE-AVX512 -qopt-zmm-usage=high, -03 -xHost -xAVX, -Kfast -DTOFU); If not set, flags are taken from the YAML formatted machine file." << std::endl; \
std::cout << "\t pmodel" << "\t\t\t performance model for computation for the first system (ECM, Roofline)" << std::endl; \
std::cout << "\t vvv" << "\t\t\t a bool flag to enable or disable the verbose node output for the first system (0: disabled, 1: enabled)" << std::endl; \
std::cout << "\t cache-predictor" << "\t cache prediction with layer conditions or ache simulation with pycachesim for the first system (LC, SIM)" << std::endl; \
std::cout << "\t penalty" << "\t\t penalty for the computation model in nanoseconds, used only in LBL or COMP mode" << std::endl; \
std::cout                                                                                                                                               \
    << "\n====================================" << std::endl                                                                                        \
    << " Delay injection mode for config.cfg" << std::endl                                                                                                                 \
    << "====================================" << std::endl;                                                                                                                                               \
std::cout << "\t delay" << "\t\t\t a bool flag to enable or disable the delay injection (0: disabled, 1: enabled)" << std::endl; \
std::cout << "\t delay_intensity" << "\t intensity of delay as a multiple of computation time of one iteration" << std::endl; \
std::cout << "\t delay_rank" << "\t\t process rank of the injected delay" << std::endl; \
std::cout << "\t delay_timestep" << "\t\t iteration for the occurrence of the injected delay" << std::endl; \
std::cout                                                                                                                                               \
    << "\n====================================" << std::endl                                                                                        \
    << " Noise injection mode for config.cfg" << std::endl                                                                                                                 \
    << "====================================" << std::endl;                                                                                                                                               \
std::cout << "\t noise" << "\t\t\t a bool flag to enable or disable the noise injection (0: disabled, 1: enabled)" << std::endl; \
std::cout << "\t noise_intensity" << "\t intensity of random noise, i.e., rand() % noise_intensity " << std::endl; \
std::cout << "\t noise_start_timestep" << "\t starting iteration for the injected noise" << std::endl; \
std::cout << "\t noise_stop_timestep" << "\t ending iteration for the injected noise" << std::endl; \
std::cout                                                                                                                                               \
    << "\n====================================" << std::endl                                                                                        \
    << " Output for config.cfg" << std::endl                                                                                                                 \
    << "====================================" << std::endl;                                                                                                                                               \
std::cout << "\t filename" << "\t\t output file name that contains details for the debug purpose" << std::endl; \
std::cout << "\t chromevizfilename" << "\t output file name that contains all time-rank tracing data for visualization with Chrome tracing browser" << std::endl; \
std::cout << "\t Verbose" << "\t\t a bool flag to enable or disable the verbose output (0: disabled, 1: enabled)" << std::endl; \
	}
#define version()                                                                                      \
	{                                                                                                  \
		std::cout << "\nDistributed Cost in Cluster (DisCostiC)" << std::endl;                       \
		std::cout << "Version : v1.0.0 (initial release)" << std::endl;                                  \
		std::cout << "Author : Ayesha Afzal <ayesha.afzal@fau.de>" << std::endl;                       \
		std::cout << "Copyright : 2023 HPC, FAU Erlangen-Nuremberg. All rights reserved" << std::endl; \
	}
#endif //!< end P2P_HPP
