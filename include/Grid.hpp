
#ifndef Grid_HPP
#define Grid_HPP

#pragma once

#include <map>
#include <vector>
#include <assert.h>

#include "enum.hpp"        //!<  enumerated types
#include "DataStruct.hpp"  //!< data structures
#include "../test/P2P.hpp" //!< benchmark test cases

namespace DisCosTiC
{
    // Benchmark benchmark(&CFG_args); 

    class Grid
    {

        /// private members of Grid class
        // private:
    public:
        DisCosTiC_Datatype numOps;
        DisCosTiC_Datatype numRanks;
        DisCosTiC_Datatype myRank;
        DisCosTiC::VecDeserialNode Nodes;

        /*class Indices
        {
        public:
        std::map<std::string, DisCosTiC::AST_OP*>* indices(DisCosTiC::GraphInterface::Benchmark & benchmark)
        {
                std::cout << "working";
            for (std::map<std::string, DisCosTiC::AST_OP*>::iterator itr = benchmark.indicesTable->begin(); itr != benchmark.indicesTable->end(); ++itr)
            {
                std::cout << '\t' << itr->first << '\t' << +itr->second->type << std::endl;
            }
            return benchmark.indicesTable;
        }
        };

        Indices set()
        {
            return Indices{};
        }*/

        /**
         * \brief get the nodes
         */
        DisCosTiC::AST_OP_TYPE getOp(Benchmark benchmark, DisCosTiC::idNodeTypePair ID, DisCosTiC_Datatype rank, DisCosTiC_Datatype label)
        {
            // label: 0...0(x18) 0012..0012(x18) 112..112(x18) 2323..2323(x18) 3345...3345(x18) 445..445(x18) 5656..5656(x18) 6678..6678(x18) 778...778(x18) 8989...8989 991011..991011... 101011....11121112...12121314..131314...14
            // set().indices(benchmark);
            // DisCosTiC::AST_OP* Nodeee= DAG->retrieveID(indicesTable, toCharPointer(label+1));

            DisCosTiC::AST_OP_ Nodeee = ID[rank][label].second; // DAG->retrievedeserialID(indicesDeserializedTable, toCharPointer(label+1));
            // DisCosTiC::AST_OP_ Nodeee = benchmark.Oneway_PositiveDisplacement()[rank][label].second;
            DisCosTiC::AST_OP_TYPE N;
            N.depCount = Nodeee.depCount;
            N.type = Nodeee.type;
            N.target = Nodeee.target;
            N.bufSize = Nodeee.bufSize;
            N.tag = Nodeee.tag;
            N.node = Nodeee.node;
            N.network = Nodeee.network;
            N.label = Nodeee.label;
            for (auto i : DisCosTiC::getRange(Nodeee.depsCount)) // DepOperations.size()/depsCount: 2 for comp, 0 for send, 1 for recv (2..2 2201..2201 001..001 1212..1212 2201..2201 001..001 1212..1212 2201..2201  001..001 1212..1212)
            {
                DisCosTiC_Datatype depnode = Nodeee.depApdxStartLabel + i + 1; // 12,45,78,1011,1314 for comp 3,6,9,12 for recv (depApdxStartLabel: 0,3,6,9,12 for comp, 2,5,8,11,14 for send, 1 for recv)
                assert(depnode < numOps);
                N.DepOperations.push_back(depnode);
            }

            // always push_back(0) i.e., idepsCount=0 for blocking calls
            for (auto i : DisCosTiC::getRange(Nodeee.idepsCount))
            {
                DisCosTiC_Datatype depnode = Nodeee.idepApdxStartLabel + i + 1; // 12,45,78,1011,1314 for comp 3,6,9,12 for recv
                assert(depnode < numOps);
                N.IdepOperations.push_back(depnode);
            }
            // std::cout<< "actual" ;
            // print_DeserialNodeNonPointerT(N); // DepOperations.size(): 2 for comp, 0 for send, 1 for recv
            return N;
        }

    public:
        /**
         * \brief default constructor
         */
        /*Grid(){
        }*/

        /**
         * \brief get the number of operations for each rank
         */
        DisCosTiC_Datatype getNumOps()
        {
            return this->numOps; // it->getnumOps()=15 (x18)
        }

        /**
         * \brief get the all operations ordered by their type for all ranks and time steps
         */
        void getTypeSortedOps(DisCosTiC::Operations &opVec)
        {
            // Nodes.size(): 1...1 (x18) 02 002 0002 ... 0..0(x18)2 0...01 0..01
            for (auto i : DisCosTiC::getRange(Nodes.size()))
            {
                DisCosTiC::DisCosTiC_OP op;
                op.target = Nodes[i].target;
                op.bufSize = Nodes[i].bufSize;
                op.tag = Nodes[i].tag;
                op.node = Nodes[i].node;
                op.network = Nodes[i].network;
                op.type = Nodes[i].type;
                op.label = Nodes[i].label;
                // op.rank = myRank;
                // op.time = 0;
                opVec.push_back(op);
            }

            Nodes.clear();

            /**
             * \brief this is a sort operation that can be used to compare DisCosTiC_OP by type and sort them with respect with that respect
             */ 
            std::sort(opVec.begin(), opVec.end(), [](DisCosTiC::DisCosTiC_OP a, DisCosTiC::DisCosTiC_OP b)
                      { 
        			if(a.type < b.type) 
        				return true; 
        			return false; });
        }

        /**
         * \brief get the initial root operations ordered by their type for all ranks and time steps
         */
        void getSortedRootOps(DisCosTiC::Operations &opVec)
        {
            DisCosTiC::DisCosTiC_OP op;
            op.target = Nodes[0].target;
            op.bufSize = Nodes[0].bufSize;
            op.tag = Nodes[0].tag;
            op.node = Nodes[0].node;
            op.network = Nodes[0].network;
            op.type = Nodes[0].type;
            op.label = Nodes[0].label;
            op.rank = myRank;
            op.time = 0;
            opVec.push_back(op);

            Nodes.clear();

            std::sort(opVec.begin(), opVec.end(), [](DisCosTiC::DisCosTiC_OP a, DisCosTiC::DisCosTiC_OP b)
                      { 
        			if(a.type < b.type) 
        				return true; 
        			return false; });
        }

        /**
         * \brief set the operation, if started
         */
        void setOp(DisCosTiC_Datatype rank, DisCosTiC_Datatype label, Benchmark benchmark, DisCosTiC::idNodeTypePair ID)
        {
            // label (time-local l-1): 0..0(x18) 12...12 3...3 45..45 6...6 ... 1314...1314 (this->id_counter first for all ranks and then increase iterations)
            //  +getOp(label).type: 3...3(x18) 12...12 3...3 12...12 3...3 12...12
            //  +getOp(label).IdepOperations.size(): 0...0
            DisCosTiC::AST_OP_TYPE N1 = getOp(benchmark, ID, rank, label);
            // std::cout<<"Rank "<<rank<<" in setOp  and label "<<label<<" and its type : "<<int(N1.type)<<" and "<<N1.IdepOperations.size()<<"independent operations : "<<std::endl;

            for (auto i : DisCosTiC::getRange(N1.IdepOperations.size()))
            {
                DisCosTiC::AST_OP_TYPE N2 = getOp(benchmark, ID, rank, N1.IdepOperations[i]);

                // std::cout<<"Rank : "<<rank<<" Independent Label : "<<N2.label<<" and its type : "<<int(N2.type)<<std::endl;
                Nodes.push_back(N2);
            }
        }

        /**
         * \brief un set the operation, if completed
         */
        void unsetOp(DisCosTiC_Datatype rank, DisCosTiC_Datatype label, Benchmark benchmark, DisCosTiC::idNodeTypePair ID)
        {
            // label: 0..0 1...1 2...2 3...3 4...4 ... 14...14 (this->id_counter first for all ranks and then increase iterations)
            //+getOp(label).DepOperations.size(): 2...2(x18) 0...0 1...1 ....
            DisCosTiC::AST_OP_TYPE N1 = getOp(benchmark, ID, rank, label);
            // std::cout<<"Rank "<<rank<<" in unsetOp  and label "<<label<<" and its type : "<<int(N1.type)<<" and "<<N1.DepOperations.size()<<"dependent operations : "<<std::endl;

            for (auto i : DisCosTiC::getRange(N1.DepOperations.size()))
            {
                DisCosTiC::AST_OP_TYPE N2 = getOp(benchmark, ID, rank, N1.DepOperations[i]);
                // std::cout<<"Rank : "<<rank<<" dependent Label : "<<N2.label<<" and its type : "<<int(N2.type)<<std::endl;

                Nodes.push_back(N2);
                // N.DepOperations[i]: 12...12 (x18) 3...3 (x18) 45..45 (x18) 6...6(x18) ... 1314...1314(x18)
            }
        }
    };

    using VecSeqGraph_t = std::vector<Grid>;

}

#endif //!< end Grid_HPP
