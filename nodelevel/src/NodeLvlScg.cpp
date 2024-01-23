
///    \file
///    NODELVLSCG_CPP
///
///    \Copyright © 2019 HPC, FAU Erlangen-Nuremberg. All rights reserved.
///

#include "../include/NodeLvlScg.hpp"

int bound = 0;

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

enum bound_type
{
	COMPUTE = 0,
	MEMORY = 1
};

void estimation(NodeModel &NM, DisCosTiC_Timetype *perf_est, DisCosTiC_Timetype *runtime)
{
	Machine machine = NM.getMachine();
	ECM ecm = NM.getECM();
	DisCosTiC_Datatype flops = NM.getFlops();
	*perf_est = (flops / ecm.T_MECM_) * (machine.f_core_ * 1000); // This gives me the MFlop/s

	if (ecm.T_OL_ == ecm.T_ECM_ || NM.benchmark_kernel == "POISSONNS")
	{
		*runtime = ecm.T_ECM_ / (machine.f_core_ * 8 * 1e9);
		bound = COMPUTE;
	}
	else
	{
		*runtime = ecm.T_MECM_ / (machine.f_core_ * 8 * 1e9);
		bound = MEMORY;
	}
}

// TODO Add scalability functions from kerncraft

__declspec(noalias) void scaling(NodeModel &NM, DisCosTiC_Timetype *scaling_performance, DisCosTiC_Timetype *scaling_numa)
{
	ECM ecm = NM.getECM();
	Machine m = NM.getMachine();

	if (ecm.T_L3Mem_ != 0.0)
	{

		DisCosTiC_Timetype util[m.cores_per_numa_domain_ + 1];
		unsigned int scaling_cores_local;

		// Calculation of Utilization
		util[0] = 0.0;
		scaling_cores_local = m.cores_per_socket_ * m.sockets_ + 5000;
		for (unsigned int c = 1; c < m.cores_per_numa_domain_ + 1; ++c)
		{
			if (c * ecm.T_L3Mem_ > (ecm.T_ECM_ + util[c - 1] * (c - 1) * ecm.T_L3Mem_ * 0.5))
			{
				util[c] = 1.0;
				scaling_cores_local = std::min(scaling_cores_local, c);
			}
			else
			{
				util[c] = (c * ecm.T_L3Mem_) / (ecm.T_ECM_ + util[c - 1] * (c - 1) * (ecm.T_L3Mem_ * 0.5));
			}
		}

		scaling_cores = scaling_cores_local;
		for (unsigned int c = 1; c < m.cores_per_socket_ + 1; ++c)
		{

			DisCosTiC_Timetype numa = 0.0;
			if (c <= scaling_cores_local)
			{
				numa = ecm.T_ECM_ / (ecm.T_ECM_ / ecm.T_L3Mem_);
			}
			else
			{
				numa = ecm.T_L3Mem_;
			}
			scaling_numa[c - 1] = numa;

			if (0 < c && c <= m.cores_per_numa_domain_)
			{
				scaling_performance[c - 1] = (numa / util[c]);
			}
			else if (m.cores_per_numa_domain_ < c && c <= m.cores_per_socket_)
			{
				scaling_performance[c - 1] = (numa / util[m.cores_per_numa_domain_]) * ((double)(m.cores_per_numa_domain_) / (c));
			}
			else
			{
				std::cerr << "The number of cores must be greater than zero and only upto the max number available as defined by cores per socket and sockets in the machine file." << std::endl;
				exit(EXIT_FAILURE);
			}
		}
	}
	else
	{
		for (unsigned int c = 1; c < m.cores_per_socket_ + 1; ++c)
		{
			scaling_performance[c - 1] = ecm.T_ECM_ / static_cast<DisCosTiC_Timetype>(c);
			scaling_numa[c - 1] = ecm.T_ECM_ / static_cast<DisCosTiC_Timetype>(m.cores_per_numa_domain_);
		}
	}
}

void executeKerncraft(char *argV[], int size)
{

	std::string execCommand;

	for (int i = 0; i < size; ++i)
	{
		execCommand.append(argV[i]);
		execCommand.append(" ");
	}

	system(execCommand.c_str());

	return;
}
