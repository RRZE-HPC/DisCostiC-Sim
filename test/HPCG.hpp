#ifndef P2P_HPP
#define P2P_HPP
#include <tuple>
#include "../include/CompModel.hpp"
#include "../include/AST.hpp"
#include "../include/DataStruct.hpp"

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

		/*DisCosTiC::tupleIdNodePair*/ DisCosTiC::idNodeTypePair Oneway_PositiveDisplacement()
		{
			DisCosTiC = new AST(&CFG_args, systemsize);
			DisCosTiC::Event recv, send, comp;
			DisCosTiC::idNodeTypePair ID(systemsize);
			// DisCosTiC::AST_OP root_Node;
			// DisCosTiC::idNodePair compIDs(systemsize), sendIDs(systemsize), recvIDs(systemsize);

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

				for (auto timestep : DisCosTiC::getRange(numTimesteps))
				{
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
						DisCosTiC->Exec("FILE:ComputeWAXPBY_ref.c//BREAK:COPY//./nodelevel/machine-files/BroadwellEP_E5-2697v4_CoD.yml//18//-D nrow 100 -D  1.0 100 -D  b 100 -D  -1.0 100 -D  Ap 100 -D  r 100 ", recv);
						DisCosTiC->Exec("FILE:ComputeDotProduct_ref.c//BREAK:COPY//./nodelevel/machine-files/BroadwellEP_E5-2697v4_CoD.yml//18//-D nrow 100 -D  r 100 -D  r 100 -D  normr 100 -D  t4 100 ", recv);
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
								DisCosTiC->Exec("FILE:ComputeSYMGS_ref.c//BREAK:COPY//./nodelevel/machine-files/BroadwellEP_E5-2697v4_CoD.yml//18//-D  100 ", recv);
								DisCosTiC->Exec("FILE:ComputeRestriction_ref.c//BREAK:COPY//./nodelevel/machine-files/BroadwellEP_E5-2697v4_CoD.yml//18//-D A 100 -D  r 100 ", recv);

								DisCosTiC->Exec("FILE:ComputeProlongation_ref.c//BREAK:COPY//./nodelevel/machine-files/BroadwellEP_E5-2697v4_CoD.yml//18//-D A 100 -D  x 100 ", recv);
								int numberOfPostsmootherSteps = A.mgData->numberOfPostsmootherSteps;
								DisCosTiC->Exec("FILE:ComputeSYMGS_ref.c//BREAK:COPY//./nodelevel/machine-files/BroadwellEP_E5-2697v4_CoD.yml//18//-D  100 ", recv);
							}
							else
							{
								DisCosTiC->Exec("FILE:ComputeSYMGS_ref.c//BREAK:COPY//./nodelevel/machine-files/BroadwellEP_E5-2697v4_CoD.yml//18//-D A 100 -D  r 100 -D  x 100 ", recv);
							}

							else DisCosTiC->Exec("FILE:ComputeWAXPBY_ref.c//BREAK:COPY//./nodelevel/machine-files/BroadwellEP_E5-2697v4_CoD.yml//18//-D nrow 100 -D  1.0 100 -D  r 100 -D  0.0 100 -D  r 100 -D  z 100 ", recv);
							DisCosTiC->Exec("FILE:ComputeWAXPBY_ref.c//BREAK:COPY//./nodelevel/machine-files/BroadwellEP_E5-2697v4_CoD.yml//18//-D nrow 100 -D  1.0 100 -D  r 100 -D  0.0 100 -D  r 100 -D  z 100 ", recv);
							if (k == 1)
							{
								DisCosTiC->Exec("FILE:ComputeDotProduct_ref.c//BREAK:COPY//./nodelevel/machine-files/BroadwellEP_E5-2697v4_CoD.yml//18//-D nrow 100 -D  r 100 -D  z 100 -D  rtz 100 -D  t4 100 ", recv);
							}
							else
							{
								oldrtz = rtz;
								DisCosTiC->Exec("FILE:ComputeDotProduct_ref.c//BREAK:COPY//./nodelevel/machine-files/BroadwellEP_E5-2697v4_CoD.yml//18//-D nrow 100 -D  r 100 -D  z 100 -D  rtz 100 -D  t4 100 ", recv);
								beta = rtz / oldrtz;
								DisCosTiC->Exec("FILE:ComputeWAXPBY_ref.c//BREAK:COPY//./nodelevel/machine-files/BroadwellEP_E5-2697v4_CoD.yml//18//-D nrow 100 -D  1.0 100 -D  z 100 -D  beta 100 -D  p 100 -D  p 100 ", recv);
							}
							DisCosTiC->Exec("FILE:ComputeDotProduct_ref.c//BREAK:COPY//./nodelevel/machine-files/BroadwellEP_E5-2697v4_CoD.yml//18//-D nrow 100 -D  p 100 -D  Ap 100 -D  pAp 100 -D  t4 100 ", recv);
							alpha = rtz / pAp;
							DisCosTiC->Exec("FILE:ComputeWAXPBY_ref.c//BREAK:COPY//./nodelevel/machine-files/BroadwellEP_E5-2697v4_CoD.yml//18//-D nrow 100 -D  1.0 100 -D  x 100 -D  alpha 100 -D  p 100 -D  x 100 ", recv);
							DisCosTiC->Exec("FILE:ComputeWAXPBY_ref.c//BREAK:COPY//./nodelevel/machine-files/BroadwellEP_E5-2697v4_CoD.yml//18//-D nrow 100 -D  1.0 100 -D  r 100 -D  -alpha 100 -D  Ap 100 -D  r 100 ", recv);
							DisCosTiC->Exec("FILE:ComputeDotProduct_ref.c//BREAK:COPY//./nodelevel/machine-files/BroadwellEP_E5-2697v4_CoD.yml//18//-D nrow 100 -D  r 100 -D  r 100 -D  normr 100 -D  t4 100 ", recv);
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
