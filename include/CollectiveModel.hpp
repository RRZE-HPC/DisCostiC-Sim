///    \file
///    COLLECTIVEMODEL_HPP

#ifndef COLLECTIVEMODEL_HPP
#define COLLECTIVEMODEL_HPP

#pragma once

#include <cmath>
#include <algorithm>
#include "DataType.hpp"
#include "enum.hpp"

namespace DisCosTiC {
    
    class CollectiveModel {
    public:
        //bcast time using binomial tree
        static DisCosTiC_Timetype calculateBinomialTreeBroadcast(
            DisCosTiC_Datatype rank,
            DisCosTiC_Datatype root,
            DisCosTiC_Datatype numRanks,
            DisCosTiC_Datatype messageSize,
            DisCosTiC_Timetype L,
            DisCosTiC_Timetype o,
            DisCosTiC_Timetype g,
            DisCosTiC_Timetype G,
            DisCosTiC_Timetype inverse_bandwidth = 0,
            DisCosTiC_Datatype eagerLimit = 65535)
        {
            if (numRanks <= 1) return 0;

            DisCosTiC_Timetype stageTime = L + o;
            if (messageSize > 0) {
                if (messageSize <= eagerLimit) {
                    //eager protocol bottleneck is max of sender/receiver gaps
                    stageTime += messageSize * std::max(g, G);
                } else {
                    //rdvz protocol bandwidth limited
                    stageTime += messageSize * inverse_bandwidth;
                }
            }

            //calculate relative rank from root
            DisCosTiC_Datatype relRank = (rank + numRanks - root) % numRanks;

            //root is special case
            if (relRank == 0) {
                //root sends at multiple stages
                DisCosTiC_Datatype totalStages = static_cast<DisCosTiC_Datatype>(
                    std::ceil(std::log2(numRanks))
                );
                //root completes after last send
                return totalStages * stageTime;
            }

            //for non root find when receive
            //rank receives at log stage
            DisCosTiC_Datatype receiveStage = static_cast<DisCosTiC_Datatype>(
                std::ceil(std::log2(relRank + 1)) - 1
            );

            //time when rank receives
            DisCosTiC_Timetype receiveTime = receiveStage * stageTime;

            //find last stage where rank sends
            DisCosTiC_Datatype lastSendStage = receiveStage;
            DisCosTiC_Datatype totalStages = static_cast<DisCosTiC_Datatype>(
                std::ceil(std::log2(numRanks))
            );

            //check each subsequent stage to see if send
            for (DisCosTiC_Datatype stage = receiveStage + 1; stage < totalStages; stage++) {
                DisCosTiC_Datatype distance = 1 << stage;
                if (relRank + distance < numRanks) {
                    //send at this stage
                    lastSendStage = stage;
                }
            }

            //never send after receiving = leaf
            if (lastSendStage == receiveStage) {
                //leafs completes immediately after receiving
                return receiveTime;
            }

            //complete after last send
            return (lastSendStage + 1) * stageTime;
        }
        
        //bcast time using ring algo
        static DisCosTiC_Timetype calculateRingBroadcast(
            DisCosTiC_Datatype rank,
            DisCosTiC_Datatype root,
            DisCosTiC_Datatype numRanks,
            DisCosTiC_Datatype messageSize,
            DisCosTiC_Timetype L,
            DisCosTiC_Timetype o,
            DisCosTiC_Timetype g,
            DisCosTiC_Timetype G,
            DisCosTiC_Timetype inverse_bandwidth = 0,
            DisCosTiC_Datatype eagerLimit = 65535)
        {
            DisCosTiC_Timetype hopTime = L + o;
            if (messageSize > 0) {
                if (messageSize <= eagerLimit) {
                    //eager protocol
                    hopTime += messageSize * std::max(g, G);
                } else {
                    //rdvz protocol bandwidth limited
                    hopTime += messageSize * inverse_bandwidth;
                }
            }

            //message is sent sequentially through all ranks
            //(numRanks-1) hops
            DisCosTiC_Timetype totalTime = (numRanks - 1) * hopTime;

            DisCosTiC_Datatype distance = (rank + numRanks - root) % numRanks;
            DisCosTiC_Timetype receiveTime = distance * hopTime;
            return totalTime;
        }

        //reduce time using binomial tree algo (inverse of bcast)
        static DisCosTiC_Timetype calculateBinomialTreeReduce(
            DisCosTiC_Datatype rank,
            DisCosTiC_Datatype root,
            DisCosTiC_Datatype numRanks,
            DisCosTiC_Datatype messageSize,
            DisCosTiC_Timetype L,
            DisCosTiC_Timetype o,
            DisCosTiC_Timetype g,
            DisCosTiC_Timetype G,
            DisCosTiC_Timetype gamma = 0,
            DisCosTiC_Timetype inverse_bandwidth = 0,
            DisCosTiC_Datatype eagerLimit = 65535)
        {
            if (numRanks <= 1) return 0;

            DisCosTiC_Datatype numStages =
                static_cast<DisCosTiC_Datatype>(std::ceil(std::log2(numRanks)));

            DisCosTiC_Timetype stageTime = L + o + gamma;
            if (messageSize > 0) {
                if (messageSize <= eagerLimit) {
                    //eager protocol bottleneck is max of sender/receiver gaps
                    stageTime += messageSize * std::max(g, G);
                } else {
                    //rdvz protocol bandwidth limited
                    stageTime += messageSize * inverse_bandwidth;
                }
            }

            if (rank == root) {
                return numStages * stageTime; //root waits all stages
            }

            DisCosTiC_Datatype distance = (rank + numRanks - root) % numRanks;
            DisCosTiC_Datatype rankStage =
                static_cast<DisCosTiC_Datatype>(std::ceil(std::log2(distance + 1)));

            return (rankStage + 1) * stageTime; //shift by 1 for non root
        }
        
        //allreduce time same for all ranks
        static DisCosTiC_Timetype calculateAllreduce(
            DisCosTiC_Datatype rank,
            DisCosTiC_Datatype numRanks,
            DisCosTiC_Datatype messageSize,
            DisCosTiC_Timetype L,
            DisCosTiC_Timetype o,
            DisCosTiC_Timetype g,
            DisCosTiC_Timetype G,
            DisCosTiC_Timetype gamma = 0,
            DisCosTiC_Timetype inverse_bandwidth = 0,
            DisCosTiC_Datatype eagerLimit = 65535)
        {
            if (numRanks <= 1) return 0;

            DisCosTiC_Datatype numStages =
                static_cast<DisCosTiC_Datatype>(std::ceil(std::log2(numRanks)));
            DisCosTiC_Datatype totalStages = 2 * numStages;

            DisCosTiC_Timetype stageTime = L + o;
            if (messageSize > 0) {
                if (messageSize <= eagerLimit) {
                    //eager protocol bottleneck is max of sender/receiver gaps
                    stageTime += messageSize * std::max(g, G);
                } else {
                    //rdvz protocol bandwidth limited
                    stageTime += messageSize * inverse_bandwidth;
                }
            }
            
            DisCosTiC_Datatype rankOffset = rank % numStages;
            DisCosTiC_Timetype rankDelay = rankOffset * (stageTime * 0.1); //small variation/delay depending on rank

            return totalStages * stageTime + numStages * gamma + rankDelay;
        }
        
        //calculating collective time with algo selection
        static DisCosTiC_Timetype calculateCollectiveTime(
            CollectiveType collectiveType,
            DisCosTiC_Datatype rank,
            DisCosTiC_Datatype root,
            DisCosTiC_Datatype numRanks,
            DisCosTiC_Datatype messageSize,
            DisCosTiC_Timetype L,
            DisCosTiC_Timetype o,
            DisCosTiC_Timetype g,
            DisCosTiC_Timetype G,
            uint8_t algorithm = 0,
            DisCosTiC_Timetype inverse_bandwidth = 0,
            DisCosTiC_Datatype eagerLimit = 65535)
        {
            switch(collectiveType) {
                case COLL_BCAST: {
                    BcastAlgorithm algo = static_cast<BcastAlgorithm>(algorithm);
                    if (algo == BCAST_RING) {
                        return calculateRingBroadcast(rank, root, numRanks, messageSize, L, o, g, G, inverse_bandwidth, eagerLimit);
                    } else {
                        //all tree variants use binomial tree
                        return calculateBinomialTreeBroadcast(rank, root, numRanks, messageSize, L, o, g, G, inverse_bandwidth, eagerLimit);
                    }
                }
                case COLL_REDUCE: {
                    DisCosTiC_Timetype gamma = messageSize * 0.001; //computation overhead per byte for reduction ops
                    return calculateBinomialTreeReduce(rank, root, numRanks, messageSize, L, o, g, G, gamma, inverse_bandwidth, eagerLimit);
                }
                case COLL_ALLREDUCE: {
                    DisCosTiC_Timetype gamma = 0; //computation overhead
                    return calculateAllreduce(rank, numRanks, messageSize, L, o, g, G, gamma, inverse_bandwidth, eagerLimit);
                }


                default:
                    return calculateBinomialTreeBroadcast(rank, root, numRanks, messageSize, L, o, g, G);
            }
        }
    };

}

#endif