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
	{                                                                                                                                                                   \
		version()                                                                                                                                                       \
				std::cout                                                                                                                                               \
			<< "\n==================================================" << std::endl                                                                                        \
			<< " Parameter for config.cfg" << std::endl                                                                                                                 \
			<< "==================================================" << std::endl                                                                                        \
			<< std::endl;                                                                                                                                               \
		std::cout                                                                                                                                                       \
			<< "\t task_per_node"                                                                                                                                       \
			<< "\t\t\t number of runing processes on one node" << std::endl;                                                                                            \
		std::cout                                                                                                                                                       \
			<< "\t number_of_processes"                                                                                                                                 \
			<< "\t\t number of runing processes on all nodes" << std::endl;                                                                                           \
		std::cout                                                                                                                                                       \
			<< "\t idlewave"                                                                                                                                            \
			<< "\t\t\t  1 to insert delay and create idlewave." << std::endl;                                                                                           \
		std::cout                                                                                                                                                       \
			<< "\t which_rank"                                                                                                                                          \
			<< "\t\t\t which rank should introduce the disturbance" << std::endl;                                                                                       \
		std::cout                                                                                                                                                       \
			<< "\t at_timestep"                                                                                                                                         \
			<< "\t\t\t at which timestep the given rank should introduce the disturbance" << std::endl;                                                                 \
		std::cout                                                                                                                                                       \
			<< "\t desync"                                                                                                                                              \
			<< "\t\t\t\t 0 to turn off the desynchronization feature and 1 to turn on." << std::endl;                                                                     \
		std::cout                                                                                                                                                       \
			<< "\t comm_model"                                                                                                                                          \
			<< "\t\t\t 0 for LogGP model and 1 for HOCKNEY model" << std::endl;                                                                                         \
		std::cout                                                                                                                                                       \
			<< "\t filename"                                                                                                                                            \
			<< "\t\t\t [optional: for debug puspose] output file name that contain details of the generated graph" << std::endl;                                        \
		std::cout                                                                                                                                                       \
			<< "\t timeunit_conv"                                                                                                                                       \
			<< "\t\t\t time multiplier for conversation from baseline unit of nanoseconds to some other unit" << std::endl;                                             \
		std::cout                                                                                                                                                       \
			<< "\t chromevizfilename"                                                                                                                                   \
			<< "\t\t the name of the output file contains all time rank tracing data for visulization with Chrome tracing browser" << std::endl;                      \
		std::cout                                                                                                                                                       \
			<< "\t Verbose"                                                                                                                                             \
			<< "\t\t\t enable/disable more verbose output (0=disable, 1=enable)" << std::endl;                                                                          \
		std::cout                                                                                                                                                       \
			<< "\t batchmode"                                                                                                                                           \
			<< "\t\t\t enable/disable batchmode (0=disable i.e., printing performance for each rank, 1=enable i.e., never print detailed all rank info)" << std::endl;  \
		std::cout                                                                                                                                                       \
			<< "\t micro_architecture"                                                                                                                                  \
			<< "\t\t\t " << std::endl;                                                                                                                                  \
		std::cout                                                                                                                                                       \
			<< "\t FP_instructions_per_cycle"                                                                                                                           \
			<< "\t\t\t " << std::endl;                                                                                                                                  \
		std::cout                                                                                                                                                       \
			<< "\t benchmark_kernel"                                                                                                                                    \
			<< "\t\t\t " << std::endl;                                                                                                                                  \
		std::cout                                                                                                                                                       \
			<< "\t kernel_mode"                                                                                                                                            \
			<< "\t\t\t choose either of FILE, SRC, LBL, SRC modes. Concatenated with benchmark kernel during runtime to determine which P2P file to use." << std::endl; \
		std::cout                                                                                                                                                       \
			<< "\t FLOPs_per_iteration"                                                                                                                                 \
			<< "\t\t number of flops in one iteration" << std::endl;                                                                                                  \
		std::cout                                                                                                                                                       \
			<< "\t R_streams"                                                                                                                                           \
			<< "\t\t\t number of read streams" << std::endl;                                                                                                            \
		std::cout                                                                                                                                                       \
			<< "\t W_streams"                                                                                                                                           \
			<< "\t\t\t number of write streams" << std::endl;                                                                                                           \
		std::cout                                                                                                                                                       \
			<< "\t datasize_in_GB"                                                                                                                                      \
			<< "\t\t\t number of array elements * (R_streams + W_streams) * size of datatype" << std::endl;                                                             \
		std::cout                                                                                                                                                       \
			<< "\t penalty"                                                                                                                                             \
			<< "\t\t\t penalty in nanoseconds. Used only in LBL and COMP mode." << std::endl;                                                                           \
		std::cout                                                                                                                                                       \
			<< "\t compiler-flags"                                                                                                                                      \
			<< "\t\t\t " << std::endl;                                                                                                                                  \
		std::cout                                                                                                                                                       \
			<< "\t pmodel"                                                                                                                                              \
			<< "\t\t\t " << std::endl;                                                                                                                                  \
		std::cout                                                                                                                                                       \
			<< "\t vvv"                                                                                                                                                 \
			<< "\t\t\t " << std::endl;                                                                                                                                  \
		std::cout                                                                                                                                                       \
			<< "\t cache-predictor"                                                                                                                                     \
			<< "\t\t\t " << std::endl;                                                                                                                                  \
		std::cout                                                                                                                                                       \
			<< "\n\t Other user defined P2P file parameters for domain size etc. etc."                                                                                  \
			<< std::endl;                                                                                                                                               \
	}
#define version()                                                                                      \
	{                                                                                                  \
		std::cout << "\nDistributed CosT in Cluster ( DisCosTiC )" << std::endl;                       \
		std::cout << "Version : v1.0 (initial release)" << std::endl;                                  \
		std::cout << "Author : Ayesha Afzal <ayesha.afzal@fau.de>" << std::endl;                       \
		std::cout << "Copyright : 2023 HPC, FAU Erlangen-Nuremberg. All rights reserved" << std::endl; \
	}
#endif //!< end P2P_HPP
