
#ifndef Node_t_HPP
#define Node_t_HPP

#pragma once

#include "DataType.hpp"
#include "macro.hpp"
#include <list>
#include <queue>
#include <array>

namespace DisCosTiC
{

    struct AST_OP
    {
        DisCosTiC_Timetype bufSize;           //!< number of bytes (data size) of this operation
        std::vector<AST_OP *> DepOperations;  //!< dependencies for blocking routines, i.e., other operations that depend on this current operation
        std::vector<AST_OP *> IdepOperations; //!< dependencies for non-blocking routines, i.e., other operations that depend on current operation
        DisCosTiC_Indextype depCount;
        DisCosTiC_Indextype label;   //!< index/identifier of this operation for each rack
        DisCosTiC_Indextype target;  //!< rank of target/partner (source for recv / dest for send / no real target for comp, just added for completeness)
        DisCosTiC_Indextype tag;     //!< tag of AST_OP	(no real tag for comp, just added for completeness)
        DisCosTiC_Indextype node;    //!< node or proceessing element for this operation
        DisCosTiC_Indextype network; //!< type of network for this operation
        char type;                   //!< type of AST_OP send, receive or computation
        char mode;                   //!< TODO: add blocking/non-blocking mode
    };

    struct AST_OP_
    {
        DisCosTiC_Datatype bufSize;
        DisCosTiC_Indextype depCount;
        DisCosTiC_Indextype label;
        DisCosTiC_Indextype target;
        DisCosTiC_Indextype tag;
        DisCosTiC_Indextype node;
        DisCosTiC_Indextype network;
        char type;
        DisCosTiC_Indextype depsCount;
        DisCosTiC_Indextype depApdxStartLabel;
        DisCosTiC_Indextype idepsCount;
        DisCosTiC_Indextype idepApdxStartLabel;
        char mode; //!< TODO: add blocking/non-blocking mode
    };

    struct AST_OP_TYPE
    {
        DisCosTiC_Datatype bufSize;
        std::vector<DisCosTiC_Indextype> DepOperations;
        std::vector<DisCosTiC_Indextype> IdepOperations;
        DisCosTiC_Indextype depCount;
        DisCosTiC_Indextype label;
        DisCosTiC_Indextype target;
        DisCosTiC_Indextype tag;
        DisCosTiC_Indextype node;
        DisCosTiC_Indextype network;
        char type;
        char mode; //!< TODO: add blocking/non-blocking mode
    };

    struct DisCosTiC_OP
    {
        DisCosTiC_Timetype time;
        DisCosTiC_Timetype starttime;     //!< only used for MSGs to identify start times
        DisCosTiC_Timetype syncstart;     //!< ifdef RANKSYNC
        DisCosTiC_Timetype numOpsInQueue; //!< STRICT_ORDER a timestamp that determines the (original) insertion order of elemenqueueOp in the queue, it is increased for every new element, not for re-insertions! Needed for correctness. Keep order between Send/Recv and exec in NB case */
        DisCosTiC_Datatype bufSize;
        DisCosTiC_Indextype target;
        DisCosTiC_Indextype rank; //!< owning rank of this operation
        DisCosTiC_Indextype label;
        DisCosTiC_Indextype tag;
        DisCosTiC_Indextype node;
        DisCosTiC_Indextype network;
        char type;
        char mode; //!< TODO: add blocking/non-blocking mode
    };

    struct DisCosTiC_queueOP
    {
        DisCosTiC_Timetype starttime; //!< for visualization
        DisCosTiC_Datatype bufSize;
        DisCosTiC_Indextype src; //!< TODO: src can go, if matching
        DisCosTiC_Indextype tag; //!< TODO: tag can go, if matching
        DisCosTiC_Indextype label;
    };

    /**
     * \brief this is a comparison functor that can be used to compare and sort DisCosTiC_OP by time
     */
    struct OpTimeComparator
    {
        inline bool operator()(DisCosTiC_OP a, DisCosTiC_OP b)
        {
            if (a.time > b.time)
                return true;
            if (a.time == b.time && a.numOpsInQueue > b.numOpsInQueue) //!< STRICT_ORDER
                return true;
            return false;
        }
    };

    using VecDeserialNode = std::vector<AST_OP_TYPE>;
    using Operations = std::vector<DisCosTiC_OP>;
    using ListqueueOp = std::list<DisCosTiC_queueOP>;
    using VecListqueueOp = std::vector<ListqueueOp>;
    using PriorityQueue_t = std::priority_queue<DisCosTiC_OP, Operations, OpTimeComparator>;
    using Event = std::pair<DisCosTiC_Indextype, DisCosTiC::AST_OP *>;
    using idNodeTypePairT = std::pair<DisCosTiC_Indextype, DisCosTiC::AST_OP_>;
    using idNodePair = std::vector<std::vector<Event>>;
    using idNodeTypePair = std::vector<std::vector<idNodeTypePairT>>;
    using tupleIdNodePair = std::tuple<idNodePair, idNodePair, idNodePair>;
    // using Networktype			= std::tuple<DisCosTiC_Timetype,DisCosTiC_Timetype,DisCosTiC_Timetype>;
    using Networktype = std::array<DisCosTiC_Timetype, 4>;

    /**
     * \brief this matches and removes operations from list if found, otherwise returns false
     */
    struct OpMatcher
    {
        static inline bool listmatch(const DisCosTiC::DisCosTiC_OP &OP,
                                     DisCosTiC::ListqueueOp *Q,
                                     DisCosTiC::DisCosTiC_queueOP *matchedOP_ = NULL)
        {
            for (DisCosTiC::ListqueueOp::iterator it = Q->begin(); it != Q->end(); ++it)
            {
                if ((OP.target == MPI_ANY_SOURC || it->src == MPI_ANY_SOURC || it->src == OP.target) && (OP.tag == MPI_ANY_TA || it->tag == MPI_ANY_TA || it->tag == OP.tag))
                {
                    if (matchedOP_)
                        *matchedOP_ = *it;
                    Q->erase(it);
                    return true;
                }
            }
            return false;
        }
    };

    template <typename... T>
    auto make_vector(T &&...args)
    {
        using first_type = typename std::tuple_element<0, std::tuple<T...>>::type;
        return std::vector<first_type>{std::forward<T>(args)...};
    }

    /*struct Bench {
        DisCosTiC_Datatype systemsize;
        DisCosTiC_Datatype datasize;
        DisCosTiC_Datatype execsize;
        DisCosTiC_Datatype timeunit_conv;
        DisCosTiC_Datatype numTimesteps;
        DisCosTiC_Datatype numOperations;
    };*/

} //!< end namespace DisCosTiC

#endif //!<!< end Node_t_HPP
