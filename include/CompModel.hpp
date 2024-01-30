
#ifndef COMPMODEL_HPP
#define COMPMODEL_HPP

#pragma once

#include <numeric> 		//!< std::iota
#include <functional> 	//!< std::placeholders
#include <chrono>       //!< time measurement
//#include <string>
#include <vector>
#include <algorithm>
using namespace std::chrono;

#include "macro.hpp"
#include "YAMLParser.hpp"

namespace DisCosTiC {
    
    class CompModel {
    private:
        DisCosTiC_Timetype unit_converter; //!< multiplier - relative to microseconds
        
    public:
        DisCosTiC_Datatype node; //!< core to execute on
        DisCosTiC_Timetype start_time; //!< start time for this local operation
        
        /**
         * \brief constructor that initializes the coordinates for unit_converter, node and start time
         * \param _unit_converter and node
         */
        /*CompModel( DisCosTiC_Timetype _unit_converter, DisCosTiC_Datatype node) :
        unit_converter(_unit_converter),
        node(node),
        start_time(0)
        {}*/
        
        
        
	CompModel(UserInterface::ConfigParser *CFG_args, UserInterface::YAMLParser YAML_args, int process_Rank)
    {
        /**
         * \brief reading system properties
         */
        std::string micro_architecture = CFG_args->getValue<std::string>("micro_architecture");
        DisCosTiC_Datatype FP_instructions_per_cycle = CFG_args->getValue<DisCosTiC_Datatype>("FP_instructions_per_cycle");
        DisCosTiC_Datatype FP_ops_per_instruction_SP = YAML_args.FP_ops_per_instruction_SP;
        DisCosTiC_Datatype FP_ops_per_instruction_DP = YAML_args.FP_ops_per_instruction_DP;
        real_t clk_freq_in_GHz = YAML_args.clk_freq_in_GHz;
        DisCosTiC_Datatype cores_per_chip = YAML_args.cores_per_chip;
        static bool streaming_stores = CFG_args->getValue<bool>("streaming_stores");
        
        std::vector<DisCosTiC_Datatype> cores(cores_per_chip) ;
        std::iota (std::begin(cores), std::end(cores), 1);

        /**
         * \brief maximum performance for core-bound workloads
         */
        DisCosTiC_Datatype FLOPs_per_cycle_SP = FP_instructions_per_cycle * FP_ops_per_instruction_SP;
        DisCosTiC_Datatype FLOPs_per_cycle_DP = FP_instructions_per_cycle * FP_ops_per_instruction_DP;
        
        real_t max_perf_per_core_SP = clk_freq_in_GHz * FLOPs_per_cycle_SP;
        real_t max_perf_per_core_DP = clk_freq_in_GHz * FLOPs_per_cycle_DP;
        
        /// multiplication with cores
        std::vector<real_t> chip_max_perf_SP(cores_per_chip);
        std::transform( cores.begin(),
        				cores.end(),
        				chip_max_perf_SP.begin(),
                        std::bind(  std::multiplies<real_t>(),
                        			std::placeholders::_1,
                        			max_perf_per_core_SP ));
        
        std::vector<real_t> chip_max_perf_DP(cores_per_chip);
        std::transform(cores.begin(), cores.end(), chip_max_perf_DP.begin(),
        std::bind(std::multiplies<real_t>(), std::placeholders::_1, max_perf_per_core_DP ));

if (process_Rank == 0){
        std::cout << "\n----------------------------------------------------------------"<< std::endl;
        std::cout << "COMPUTATIONAL PERFORMANCE MODEL" << std::endl;
        std::cout << "----------------------------------------------------------------"<< std::endl;
        std::cout << "Code simulating on " << micro_architecture <<" chip of cores: "; //<< std::endl;
        //std::cout << "cores: " ;
        print_vec1T(cores);
        std::cout << "Single-precision maximum chip performance [GF/s]: ";
        print_vec1T(chip_max_perf_SP);
        std::cout << "Double-precision maximum chip performance [GF/s]: ";
        print_vec1T(chip_max_perf_DP);
}
        /**
         * \brief reading properties for memory-bound STREAM TRIAD benchmark
         */
//        std::string benchmark_kernel = CFG_args->getValue<std::string>("benchmark_kernel");
        DisCosTiC_Datatype FLOPs_per_iteration = CFG_args->getValue<DisCosTiC_Datatype>("FLOPs_per_iteration");
        DisCosTiC_Datatype R_streams = CFG_args->getValue<DisCosTiC_Datatype>("R_streams");
        //DisCosTiC_Datatype R_streams_in_bytes = CFG_args->getValue<DisCosTiC_Datatype>("R_streams_in_bytes");
        DisCosTiC_Datatype W_streams = CFG_args->getValue<DisCosTiC_Datatype>("W_streams");
        //DisCosTiC_Datatype W_streams_in_bytes = CFG_args->getValue<DisCosTiC_Datatype>("W_streams_in_bytes");
        real_t datasize_in_GB = CFG_args->getValue<real_t>("datasize_in_GB");
        
        /**
         * \brief performance for memory-bound workloads due to transfer bottleneck
         * performance in Flop/s with roofline model: arithmetic intensity (= flops / bytes_transfer) * bandwidth
         */
        real_t WA_factor;
        if (streaming_stores){ //!< data at the missed-write location is not loaded to cache, and is written directly to the backing store
            R_streams = R_streams;
            WA_factor = 1.0;
        }
        else{ //!< data at the missed-write location is loaded to cache, followed by a write-hit operation
            real_t Read_streams = R_streams;
            R_streams = R_streams + W_streams;
            WA_factor = (R_streams  + W_streams)/(Read_streams + W_streams) ; // Normal stores are performed (disables generation of streaming stores for optimization)
        }
        
        real_t MEMBw_1C = CFG_args->getValue<real_t>("MEMBw_1C_in_GBpers");
        real_t MEMBw_2C = CFG_args->getValue<real_t>("MEMBw_2C_in_GBpers");
        real_t MEMBw_3C = CFG_args->getValue<real_t>("MEMBw_3C_in_GBpers");
        real_t MEMBw_4C = CFG_args->getValue<real_t>("MEMBw_4C_in_GBpers");
        real_t MEMBw_5C = CFG_args->getValue<real_t>("MEMBw_5C_in_GBpers");
        real_t MEMBw_6C = CFG_args->getValue<real_t>("MEMBw_6C_in_GBpers");
        real_t MEMBw_7C = CFG_args->getValue<real_t>("MEMBw_7C_in_GBpers");
        real_t MEMBw_8C = CFG_args->getValue<real_t>("MEMBw_8C_in_GBpers");
        real_t MEMBw_9C = CFG_args->getValue<real_t>("MEMBw_9C_in_GBpers");
        real_t MEMBw_10C = CFG_args->getValue<real_t>("MEMBw_10C_in_GBpers");
        
        std::vector<real_t> MEM_bandwidth = YAML_args.MEM_bandwidth;

        
        /// multiplication with WA_factor
        std::transform( MEM_bandwidth.begin(),
        				MEM_bandwidth.end(),
        				MEM_bandwidth.begin(),
                        std::bind(  std::multiplies<real_t>(),
                        			std::placeholders::_1,
                        			WA_factor));
        
        real_t bytes_transfered = (R_streams + W_streams) * sizeof(real_t);
        
        real_t arith_intens_in_FLOPperB = FLOPs_per_iteration/bytes_transfered;
        
        std::vector<real_t> perf_in_FLOPpers(cores_per_chip);
        std::transform( MEM_bandwidth.begin(), 
        				MEM_bandwidth.end(), 
        				perf_in_FLOPpers.begin(),
                        std::bind(  std::multiplies<real_t>(), 
                        			std::placeholders::_1,
                        			arith_intens_in_FLOPperB));
        
        std::vector<real_t> time_in_s(cores_per_chip);
        std::transform (MEM_bandwidth.begin(), MEM_bandwidth.end(), time_in_s.begin(),
                   [&datasize_in_GB](double v) { return datasize_in_GB / v; });
        
        // std::vector<DisCosTiC_Timetype> time_in_s = { datasize_in_GB/ MEMBw_1C, datasize_in_GB/ MEMBw_2C,
        //     datasize_in_GB / MEMBw_3C, datasize_in_GB / MEMBw_4C, datasize_in_GB / MEMBw_5C,
        //     datasize_in_GB  / MEMBw_6C, datasize_in_GB / MEMBw_7C, datasize_in_GB / MEMBw_8C,
        //     datasize_in_GB / MEMBw_9C, datasize_in_GB / MEMBw_10C};

 		for (auto const& i : time_in_s)
 		{
	        if(i < 0)
                throw std::runtime_error("negative operation time\n"); 
        }
        
        std::vector<DisCosTiC_Timetype> time_in_ns(cores_per_chip);
        std::transform( time_in_s.begin(),
        				time_in_s.end(),
        				time_in_ns.begin(),
                        std::bind(  std::multiplies<real_t>(),
                        			std::placeholders::_1,
                        			1000000000));
/*if (process_Rank == 0){
        std::cout << "bytes transfer [B]: " << bytes_transfered << std::endl;
        std::cout << "arithmetic intensity [F/B]: " << arith_intens_in_FLOPperB <<  std::endl;
        std::cout << "memory bandwidth [GB/s]: " << std::endl;
        print_vec1T(MEM_bandwidth);
        std::cout << "performnace [GF/s]: " << std::endl;
        print_vec1T(perf_in_FLOPpers);
        std::cout << "runtime [ns]: " << std::endl;
        print_vec1T(time_in_ns);
}*/
	}
    };
    
} // end namespace DisCosTiC

#endif //!< end COMPMODEL_HPP
