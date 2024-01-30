
///    \file
///    NODELVLSCG_HPP
///
///    \Copyright © 2024 HPC, FAU Erlangen-Nuremberg. All rights reserved.
///

#ifndef NodeLvlScg_HPP
#define NodeLvlScg_HPP

#pragma once

#include <algorithm>
#include <fstream>
#include <string>
#include <iostream>
#include <cstring>

#include <unistd.h>
#include <sys/wait.h>

#include "NodeModel.hpp"

void estimation(NodeModel& NM, DisCosTiC_Timetype* perf_est, DisCosTiC_Timetype* runtime);
void scaling(NodeModel& NM, DisCosTiC_Timetype* scaling_performance, DisCosTiC_Timetype* scaling_numa);
void executeKerncraft(char* argV[], int size);
//static double memory_bus_utilization(NodeModel& NM, double n_core, double oldUtilization);
//void ecm_est(NodeModel& NM, double* perf_est, double* pareff, double* runtime);
//void printOutput(NodeModel& NM, double* perf_est, double* pareff);

#endif //!< end NodeLvlScg_HPP

