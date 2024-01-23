
///    \file
///    NODEMODEL_HPP
///
///    \Copyright © 2019 HPC, FAU Erlangen-Nuremberg. All rights reserved.
///

#ifndef NodeModel_HPP
#define NodeModel_HPP

#pragma once

#include "../../include/ConfigParser.hpp"
#include "../../include/YAMLParser.hpp"

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

//===Machine===//
struct Machine
{
	DisCosTiC_Datatype n_cores_;
	DisCosTiC_Datatype p0_nom_;
	DisCosTiC_Datatype cores_per_numa_domain_;
	DisCosTiC_Datatype cores_per_socket_;
	DisCosTiC_Datatype sockets_;
	DisCosTiC_Timetype f_core_;
	DisCosTiC_Timetype f_uncore_;
	DisCosTiC_Timetype f_core_nom_;
	DisCosTiC_Timetype alpha_;
	DisCosTiC_Timetype task_;
};

//===ECM Model===//
struct ECM
{
	DisCosTiC_Timetype T_OL_;
	DisCosTiC_Timetype T_nOL_;
	DisCosTiC_Timetype T_L1L2_;
	DisCosTiC_Timetype T_L2L3_;
	DisCosTiC_Timetype T_L3Mem_;
	DisCosTiC_Timetype T_ECM_;
	DisCosTiC_Timetype T_MECM_;
};

class NodeModel
{
private:
	Machine machine_;
	ECM ecm_;
	DisCosTiC_Datatype flops_;
	std::string filename_;

public:
	//===Constructor===//
	NodeModel() = default;
	std::string benchmark_kernel;

	NodeModel(Machine &machine, ECM &ecm, DisCosTiC_Datatype flops, std::string &filename) : machine_(machine), ecm_(ecm), flops_(flops), filename_(filename) {}

	NodeModel(UserInterface::ConfigParser *CFG_args, UserInterface::YAMLParser YAML_args, UserInterface::ConfigParser *CFG_args2, std::string ECM_core)
	{
		machine_.n_cores_ = YAML_args.cores_per_chip * YAML_args.chips_per_node;

		machine_.cores_per_numa_domain_ = YAML_args.cores_per_numa_domain;
		machine_.cores_per_socket_ = YAML_args.cores_per_chip;
		machine_.sockets_ = YAML_args.chips_per_node;
		machine_.task_ = task_per_node;

		machine_.f_core_ = YAML_args.clk_freq_in_GHz;

		if (system_number == 0)
		{
			ecm_.T_OL_ = CFG_args2->getValue<DisCosTiC_Timetype>("T_OL");
			ecm_.T_nOL_ = CFG_args2->getValue<DisCosTiC_Timetype>("T_nOL");
			ecm_.T_L1L2_ = CFG_args2->getValue<DisCosTiC_Timetype>("T_L1L2");
			ecm_.T_L2L3_ = CFG_args2->getValue<DisCosTiC_Timetype>("T_L2L3");
			ecm_.T_L3Mem_ = CFG_args2->getValue<DisCosTiC_Timetype>("T_L3Mem");
			ecm_.T_ECM_ = CFG_args2->getValue<DisCosTiC_Timetype>("T_ECM");
			ecm_.T_MECM_ = CFG_args2->getValue<DisCosTiC_Timetype>("T_" + ECM_core + "ECM");

			// std::cout << "Scaling cores before conversion : " << CFG_args2->getValue<std::string>("Scaling_cores_Secondary") << std::endl;
			scaling_cores = std::strcmp(CFG_args2->getValue<std::string>("Scaling_cores").c_str(), "inf") == 0 ? -1 : CFG_args2->getValue<DisCosTiC_Timetype>("Scaling_cores");
			// std::cout << "Scaling cores after conversion : " << scaling_cores << std::endl;
		}
		else
		{
			ecm_.T_OL_ = CFG_args2->getValue<DisCosTiC_Timetype>("T_OL_Secondary");
			ecm_.T_nOL_ = CFG_args2->getValue<DisCosTiC_Timetype>("T_nOL_Secondary");
			ecm_.T_L1L2_ = CFG_args2->getValue<DisCosTiC_Timetype>("T_L1L2_Secondary");
			ecm_.T_L2L3_ = CFG_args2->getValue<DisCosTiC_Timetype>("T_L2L3_Secondary");
			ecm_.T_L3Mem_ = CFG_args2->getValue<DisCosTiC_Timetype>("T_L3Mem_Secondary");
			ecm_.T_ECM_ = CFG_args2->getValue<DisCosTiC_Timetype>("T_ECM_Secondary");
			ecm_.T_MECM_ = CFG_args2->getValue<DisCosTiC_Timetype>("T_" + ECM_core + "ECM_Secondary");
			// std::cout << "Scaling cores before conversion : " << CFG_args2->getValue<std::string>("Scaling_cores_Secondary") << std::endl;
			scaling_cores = std::strcmp(CFG_args2->getValue<std::string>("Scaling_cores_Secondary").c_str(), "inf") == 0 ? -1 : CFG_args2->getValue<DisCosTiC_Timetype>("Scaling_cores_Secondary");
			// std::cout << "Scaling cores after conversion : " << scaling_cores << std::endl;
		}
		filename_ = CFG_args->getValue<std::string>("filename");
		flops_ = CFG_args->getValue<DisCosTiC_Datatype>("FLOPs_per_iteration");
		benchmark_kernel = CFG_args->getValue<std::string>("benchmark_kernel");
	}
	NodeModel(UserInterface::ConfigParser *CFG_args, UserInterface::YAMLParser YAML_args, DisCosTiC_Timetype T_OL, DisCosTiC_Timetype T_nOL, DisCosTiC_Timetype T_L1L2, DisCosTiC_Timetype T_L2L3, DisCosTiC_Timetype T_L3Mem, DisCosTiC_Timetype T_ECM)
	{
		machine_.n_cores_ = YAML_args.cores_per_chip * YAML_args.chips_per_node;

		machine_.cores_per_numa_domain_ = YAML_args.cores_per_numa_domain;
		machine_.cores_per_socket_ = YAML_args.cores_per_chip;
		machine_.sockets_ = YAML_args.chips_per_node;
		machine_.task_ = task_per_node;

		machine_.f_core_ = YAML_args.clk_freq_in_GHz;

		ecm_.T_OL_ = T_OL;
		ecm_.T_nOL_ = T_nOL;
		ecm_.T_L1L2_ = T_L1L2;
		ecm_.T_L2L3_ = T_L2L3;
		ecm_.T_L3Mem_ = T_L3Mem;
		ecm_.T_ECM_ = T_ECM;
		ecm_.T_MECM_ = 0.0;

		filename_ = CFG_args->getValue<std::string>("filename");
		flops_ = CFG_args->getValue<DisCosTiC_Datatype>("FLOPs_per_iteration");
	}
	//===Destructor===//
	~NodeModel() = default;

	//===getter===//
	Machine &getMachine()
	{
		return machine_;
	}

	ECM &getECM()
	{
		return ecm_;
	}

	DisCosTiC_Datatype getFlops()
	{
		return flops_;
	}

	std::string &getFileName()
	{
		return filename_;
	}

	//===setter===//
	void setMultiCore(DisCosTiC_Timetype T_MECM)
	{
		ecm_.T_MECM_ = T_MECM;
	}
};

#endif //!< end NodeModel_HPP
