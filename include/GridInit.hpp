#ifndef GRIDINIT_HPP
#define GRIDINIT_HPP

#pragma once

#include <vector>
#include <string>

#include "Grid.hpp"

namespace DisCosTiC {
/**
 * \brief this class exposes all P graphVec and manages dependencies and execution order.
 * It returns a list of executable operations and offers an interface to mark operations as executed.
 */
class Grid_Init {
    ///private variables
private:
    DisCosTiC_Datatype num_ranks;
    DisCosTiC_Datatype num_operations;
    
    ///public variables
public:
    DisCosTiC::VecSeqGraph_t graphVec;
    /**
     * \brief constructor that initializes the variables.
     * \param binary file name 
     */
    Grid_Init(DisCosTiC_Datatype num_ranks_, DisCosTiC_Datatype num_operations_, Benchmark benchmark, DisCosTiC::idNodeTypePair ID):
    		num_ranks(num_ranks_),
    		num_operations(num_operations_) 
    	{
	        	
        /// get root nodes of type DisCosTiC::AST_OP_TYPE (i.e., exec in our case ) and other info of all ranks
        for (auto rank : DisCosTiC::getRange(num_ranks)) {                              
        	DisCosTiC::Grid G;
            G.myRank = rank; 
            G.numRanks = num_ranks; 
            G.numOps = num_operations;
            G.Nodes.clear(); 
			G.Nodes.push_back(G.getOp(benchmark, ID, rank, 0)); 
			graphVec.push_back(G);    
            // std::cout<<"In rank : "<<org_rank<<" for Rank : "<<rank<<" and its numOps in GraphVec : "<<num_operations<<std::endl;
        }
          // std::cout << "In rank : "<<org_rank<<"graphVec stores " << int(graphVec.size()) << " numbers.\n"; // number of ranks
    }
    
    /**
     * \brief destructor
     */
    ~Grid_Init() {
    }
    
};

} // end namespace DisCosTiC

#endif //!< end GRIDINIT_HPP
