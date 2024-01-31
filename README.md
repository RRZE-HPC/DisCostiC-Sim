# DisCostiC: Distributed Cost in Cluster

  </a><a href="https://DisCostiC-Sim.readthedocs.io/">
  <img alt="Docs" src="https://img.shields.io/badge/read-the_docs-blue" />
  </a><a href="https://matrix.to/#/!OeNPngEkrqRRkBJjwz:gwaaf.rrze.fau.de?via=gwaaf.rrze.fau.de">
  <img alt="Join the discussion on Matrix" src="https://img.shields.io/badge/Discussions%20%26%20Support-Chat%20now!-blue" />
  </a><a href="https://www.linkedin.com/in/ayeshaafzal-/">
    <img alt="linkedin" src="https://i.stack.imgur.com/gVE0j.png" />
  <a href="https://github.com/AyeshaAfzal91" rel="nofollow noreferrer">
    <img alt="github" src="https://i.stack.imgur.com/tskMh.png">
  </a>
  </a><a href="https://matrix.to/#/!OeNPngEkrqRRkBJjwz:gwaaf.rrze.fau.de?via=gwaaf.rrze.fau.de" target="\_parent">
  <img alt="" src="https://img.shields.io/github/stars/tanstack/react-table.svg?style=social&label=Star" />
  </a><a href="https://twitter.com/AyeshaHamad4" target="\_parent">
  <img alt="" src="https://img.shields.io/twitter/follow/AyeshaHamad4.svg?style=social&label=Follow" />

## Table of Contents

  * [💡Description](#description)
    * [⚙️ Framework workflow](#workflow)
    * [🖍 Advantages over existing tools](#advantages)
  * [:octocat: Compilation and build](#compilation-and-build)
    * [⚡ Installation](#installation)
    * [⏱️ Configuration settings](#configuration)
    * [🥅 Compilation](#compilation)
    * [:green_circle: Run](#run)
    * [🔁 Clean and uninstall](#clean)
  * [:signal_strength: DisCostiC output](#output)
    * [🌐 1. Web interface with Google Chrome browser (DMS file format)](#chrome)
    * [📊 2. Graphical user interface with Vampir (OTF2 file format)](#vampir)
    * [📊 3. Graphical user interface with ITAC (STF file format)](#itac)
    * [🗄️ 4. Statistical or log data](#stat)
  * [💻 System model](#cluster-configuration)
    * [🔌 Cluster model](#cluster)
    * [🔌 Network model](#network)
    * [🔌 Node model](#node)
  * [👩‍💻 Automating DSEL generation through static analysis](#automating-dsel-generation-through-static-analysis)
    * [📝 Essential DisCostiC routines](#essential-routines)
    * [📝 Code blueprint as DisCostiC input](#DSEL)
    * [📝 Code documentation: suite of C++ data structures and enumerated types](#documentation)
  * [🧐 MPI-parallelized DisCostiC implementation](#mpi-parallelized-implementation)
  * [🚀 Planned features for further development](#library-limitations)
  * [📚 References about the theory of potential application scenarios for DisCostiC](#references)
  * [🔒 License](#license)
  * [:warning: Disclaimer](#disclaimer)
  * [🔗 Acknowledgement](#acknowledgement)
  * [📱Contact](#contact)


<a name="description"></a>
## 💡 Description

A `cross-architecture resource-based parallel simulation framework` that can efficiently predict the performance of real or hypothetical massively parallel MPI programs on current and future highly parallel heterogeneous systems.✨

The runtime predictions are carried out in a controlled environment through _`straightforward, portable, scalable, and efficient simulations`_.⭐

The simulation framework is an automated version of `analytical first-principle performance models at a full-scale scope`, including cores, chips, nodes, networks, clusters, and their mutual interactions plus inherent bottlenecks such as memory bandwidth.🌟

The application blueprint created using `Domain Specific Embedded Language (DSEL)` enables key concepts and elements embedded in the modern C++ language, enhancing `readability for domain experts and accurately encoding inter-process dependencies without introducing unknown consequences from the actual systems`. 💥


<a name="workflow"></a>
⚙️ **Framework workflow**💬

```diff
- Application model

    * Domain knowledge of the application expressed in the DisCostiC DSEL language provides information on call dependencies and properties for computation (data access pattern, flop count) and communication (volume, mode and protocol)

- Analytic first-principle performance models
    
    * Computation models: Fundamental analytic Roofline and refined ECM models including memory bandwidth bottleneck concept at the system level
    * Communication models: Fundamental latency-bandwidth and refined LogGP models

- System models
    
    * Cluster, network and node models for the full topology of the target system, including multiple chips, nodes, network interfaces and clusters
 
+ DisCostiC combines all of them to simulate the runtime cost of distributed applications.
```

<a name="advantages"></a>
🖍 **Advantages over existing tools**💬

```diff
- DSEL approach for creating application blueprint 

    * Accurately encoding inter-process dependencies without introducing unknown consequences from the actual systems 

- A full-scale first-principle-model-based simulator 

    * Automating analytical first-principle performance models on the entire hierarchy of parallel systems

- Efficient speed 

    * No intermediate tracing files requirement like any offline, trace-driven tools 
    * No high memory requirement like any online tools that use the host architectures to execute code
	
+ Last but not least, an open-source low-entry cost lightweight simulator enabling model-based design-space exploration 
```

<a name="compilation-and-build"></a>
## :octocat: Compilation and build

First, use the following command to clone the git repository:

```
git clone git@github.com:RRZE-HPC/DisCostiC-Sim.git && cd DisCostiC-Sim
```

<a name="installation"></a>
⚡ **Installation**:thought_balloon:

Before proceeding, make sure the environment is prepared for the compilation.
The installation steps are listed below:
```
module load python git intel intelmpi cmake itac vampir
conda create --name XYZ
conda activate XYZ
conda install pip 
cmake -DCMAKE_INSTALL_PREFIX=~/.local . && make all install
```
One way to check the installation is to print the version and the help of the DisCostiC using `./discostic --version` and `./discostic --help`.

<a name="configuration"></a>
⏱️ **Configuration settings**:thought_balloon:

The [test](test) folder in DisCostiC offers multiple MPI-parallelized programs (`benchmark_kernel`) in distinct functionality for computation (`kernel_mode`) and communication (`exchange_mode`). For illustration, a few examples are given below:

`benchmark_kernel`                 | Description
--------------------- | -------------
`HEAT`          |       Two-dimensional five-point Jacobi kernel with communication
`SOR`          |    Gauss-Seidel Successive Over-Relaxation solver with communication
``DMMM`               | Dense Matrix Matrix Multiplication kernel with communication
``DMVM`               | Dense Matrix Vector Multiplication kernel with communication
``DMVM-TRANSPOSE`     | Dense Matrix Transpose Vector Multiplication kernel with communication
`HEATHEAT`                | Back-to-back two HEAT kernels with communication
`HEATSOR`                | Back-to-back HEAT and SOR kernels with communication
`HEATDIVIDE`                | Back-to-back HEAT and DIVIDE kernels with communication
`HPCG`               | High Performance Conjugate Gradient
`STENCIL-3D-7PT`                | Three-dimensional seven point stencil kernel with communication
`STENCIL-3D-27PT`                | Three-dimensional twenty seven point stencil kernel with communication
`STENCIL-UXX`                | UXX stencil kernel with communication
`STENCIL-3D-LONGRANGE`                | 3D long range stencil kernel with communication
`STENCIL-1D-3PT`                | One-dimensional three point stencil kernel with communication
`STREAM`                | STREAM Triad kernel with communication
`SCHOENAUER`                | SCHOENAUER Triad kernel with communication
`SCHOENAUER-DIV`                | SCHOENAUER divide Triad kernel with communication
`WAXPY`                | WAXPY kernel with communication
`DAXPY`                | DAXPY kernel with communication
`SUM`                | SUM kernel with communication
`VECTOR-SUM`                | Vector SUM kernel with communication
`ADD`                | ADD kernel with communication
`DIVIDE`                | DIVIDE kernel with communication
`SCALE`                | SCALE kernel with communication
`COPY`                | COPY kernel with communication
`KAHAN-DOT`                | KAHAN-DOT kernel with communication
`SCALAR-PRODUCT`                | Scalar Product kernel with communication


The following `kernel_mode` is exclusively offered for the flexibility of the framework; the simulator's prediction remains unaffected by the selection of the `kernel_mode`. Further explanation of this mode can be found in the [Essential DisCostiC routines](#essential-routines) section.

`kernel_mode`                 | Integrated tool | Description
--------------------- | ------------- | -------------
`COMP`          |   no external tool integrated |  This directly embeds the single-core pre-recorded ECM performance model data of the computational kernel into the simulator. 
`LBL`           |   no external tool integrated |  This reads the pre-recorded ECM performance model data for the computational kernel from an external file located at [nodelevel/configs](nodelevel/configs) folder.
`SRC`           |   [Kerncraft](https://github.com/RRZE-HPC/kerncraft) integrated      |  This directly embeds the source code of the computational kernel into the simulator.
`FILE`          |   [Kerncraft](https://github.com/RRZE-HPC/kerncraft) integrated      |  This reads the source code for the computational kernel from an external file located at [nodelevel/kernels](nodelevel/kernels) folder. 

The following `exchange_mode` is provided in LBL mode to enable model-based exploration through experimenting with various communication patterns in MPI applications. 

`exchange_mode`                  | Description
--------------------- | -------------
`message_size`          |       It specifies the size of the message to be exchanged in bytes.
`step_size`          |       It describes the step size of the message exchange as an int (1: distance one communication, 2: distance two communication, ...).
`direction`          |       It specifies the direction of message exchange as an int (1: uni-directional upwards shift in only positive direction, 2: bi-directional upwards and downwards shift in both positive and negative directions).
`periodic`          |       It enables or disables the communication periodicity as a bool (0: false, 1: true).

The `config.cfg` file can be edited to select the `benchmark_kernel` and `kernel_mode`. More information about this `config.cfg` file is available in the [DisCostiC help](#help) documentation.

<a name="compilation"></a>
🥅 **Compilation**:thought_balloon:

The compilation offers the following choices to enable the output report's data format generation and to enable the tracing of the simulator's own implementation. An executable will be generated after compilation. 

Command                  | Description
--------------------- | -------------
`make`          | This enables JSON data format without ITAC profiling of simulator implementation.
`make otf2`          | This enables both JSON and OTF2 data format without ITAC profiling of simulator implementation.
`make trace_MPI`          | This enables JSON data format and the standard ITAC tracing mode of simulator implementation with the information about MPI call functions (enabled flag: `-trace`).
`make trace_all`               | This enables JSON data format and the verbose ITAC tracing mode of simulator implementation with the information on both user-defined and MPI call functions (enabled flag: `-trace -tcollect flag`).


<a name="run"></a>
:green_circle: **Run**:thought_balloon:

In the batch script, the number of the simulator processes is configured as the number of simulated processes plus one. To run the batch script on any system, ITAC profiling can be enabled or disabled, which will or will not dump the simulator's own trace in ITAC:

Command                  | Description
--------------------- | -------------
`sbatch Run_Simulation.sh`          | This performs the simulation without tracing the own implementation of the simulator using ITAC.
`sbatch Run_Simulation_ITAC.sh`          | This dumps the simulator's own trace into ITAC to investigate the implementation of the simulator. 


The `Run_Simulation_ITAC.sh` script only generates a single STF file (`simulation.single.stf`) in the main directory due to the export of the following variables:

```
export VT_LOGFILE_FORMAT=SINGLESTF
export VT_LOGFILE_NAME=simulation
export VT_LOGFILE_PREFIX=$SLURM_SUBMIT_DIR
export VT_FLUSH_PREFIX=/tmp
```
The formats, names and locations of output files in these environmental variables can be adjusted as desired.

<a name="clean"></a>
🔁 **Clean and uninstall**:thought_balloon:

Command                  | Description
--------------------- | -------------
`make clear`            |   This cleans up the working directory by removing all unnecessary DisCostiC files, such as *.dms, *.otf, *.csv files.
`make uninstall`        |   This uninstalls the DisCostiC framework, including installed files and CMake-specific files.


<a name="output"></a>
## :signal_strength: DisCostiC output

<a name="chrome"></a>
🌐 **1. Web interface with Google Chrome browser (DMS file format)**:speech_balloon:

Upon completion of the run, DisCostiC generates a report referred to as `DisCostiC.dms`.
`DisCostiC.dms` is a straightforward [JSON object data format file](https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU/edit) that can be viewed using the [Google Chrome browser](chrome://tracing).
Use the following Google Chrome web browser to load the generated JSON file:

```
chrome://tracing
```

<a name="vampir"></a>
📊 **2. Graphical user interface with Vampir (OTF2 file format)**:speech_balloon:

Upon completion of a run that was compiled with `make otf2`, DisCostiC generates a report called `DisCostiC/traces.otf2`. The `DisCostiC/traces.otf2` file is an [OTF2 object data format](https://perftools.pages.jsc.fz-juelich.de/cicd/otf2/tags/otf2-3.0.2/html/) file and can be viewed using third-party tools like [ITAC](https://www.intel.com/content/www/us/en/developer/tools/oneapi/trace-analyzer.html) and [Vampir](https://vampir.eu).
This format is produced by the [ChromeTrace2Otf2](https://profilerpedia.markhansen.co.nz/converters/otf2-cli-chrome-trace-converter/) converter, which converts the JSON object data format file `(DisCostiC.dms)` to the OTF2 object data format file `(DisCostiC/traces.otf2)`.

Use the following commond to open an OTF2 small trace file (`DisCostiC/traces.ot2`) in [Vampir](https://vampir.eu):

```
vampir DisCostiC/traces.otf2
```

<a name="itac"></a>
📊 **3. Graphical user interface with ITAC (STF file format)**:speech_balloon:

To convert an OTF2 file to a single Structured Trace File (STF) file format `DisCostiC.single.stf` and to open it in [ITAC](https://www.intel.com/content/www/us/en/developer/tools/oneapi/trace-analyzer.html), invoke the Intel Trace Analyzer GUI and follow the below steps:

```
(1) traceanalyzer &
(2) Go to File > Open; from the Files of a type field, select Open Trace Format, navigate to the `DisCostiC/traces.otf2` file, and double-click to open it
(3) The OTF2 to STF conversion dialog appears. Review the available fields and checkboxes, and click Start to start the conversion. As a result, the OTF2 file will be converted to STF (DisCostiC/traces.otf2.single.stf), and you will be able to view it in the Intel Trace Analyzer.
```

<a name="stat"></a>
🗄️ **4. Statistical or log data**:thought_balloon:

	    ----------------------------------------------------------------
        DisCostiC
        ----------------------------------------------------------------
        Full form: Distributed Cost in Cluster
        Version: v1.0.0
        Timestamp: Tue Jan 20 11:16:42 2024
        Author: Ayesha Afzal <ayesha.afzal@fau.de>
        Copyright: © 2024 HPC, FAU Erlangen-Nuremberg. All rights reserved

        HEAT program in FILE mode of kernel is simulated for 100000 iterations and 72 processes

        ----------------------------------------------------------------
        COMPUTATIONAL PERFORMANCE MODEL
        ----------------------------------------------------------------
        Code simulating on IcelakeSP_Platinum-8360Y chip of cores: {1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 , 9 , 10 , 11 , 12 , 13 , 14 , 15 , 16 , 17 , 18 , 19 , 20 , 21 , 22 , 23 , 24 , 25 , 26 , 27 , 28 , 29 , 30 , 31 , 32 , 33 , 34 , 35 , 36 }
        Single-precision maximum chip performance [GF/s]: {614.4 , 1228.8 , 1843.2 , 2457.6 , 3072 , 3686.4 , 4300.8 , 4915.2 , 5529.6 , 6144 , 6758.4 , 7372.8 , 7987.2 , 8601.6 , 9216 , 9830.4 , 10444.8 , 11059.2 , 11673.6 , 12288 , 12902.4 , 13516.8 , 14131.2 , 14745.6 , 15360 , 15974.4 , 16588.8 , 17203.2 , 17817.6 , 18432 , 19046.4 , 19660.8 , 20275.2 , 20889.6 , 21504 , 22118.4 }
        Double-precision maximum chip performance [GF/s]: {307.2 , 614.4 , 921.6 , 1228.8 , 1536 , 1843.2 , 2150.4 , 2457.6 , 2764.8 , 3072 , 3379.2 , 3686.4 , 3993.6 , 4300.8 , 4608 , 4915.2 , 5222.4 , 5529.6 , 5836.8 , 6144 , 6451.2 , 6758.4 , 7065.6 , 7372.8 , 7680 , 7987.2 , 8294.4 , 8601.6 , 8908.8 , 9216 , 9523.2 , 9830.4 , 10137.6 , 10444.8 , 10752 , 11059.2 }

        ----------------------------------------------------------------
        COMMUNICATION PERFORMANCE MODEL
        ----------------------------------------------------------------
        Code simulating on InfiniBand with IntelMPI library
        Hockney model parameters (intra chip): 41185.7 [ns] latency, 0.079065 [s/GB] inverse bandwidth 
        Hockney model parameters (inter chip): 64567.6 [ns] latency, 0.124122 [s/GB] inverse bandwidth 
        Hockney model parameters (inter node): 47628.7 [ns] latency, 0.091516 [s/GB] inverse bandwidth 
        Hockney model parameters (inter cluster): 47628.7 [ns] latency, 0.091516 [s/GB] inverse bandwidth 
        LogGP model additional parameters (intra chip): 24429.9 [ns] o, 199.008 [ns] g, 0.065 [ns] O
        LogGP model additional parameters (inter chip): 39748.3 [ns] o, 338.249 [ns] g, 0.085 [ns] O
        LogGP model additional parameters (inter node): 47938.9 [ns] o, 4980.51 [ns] g, 0.084 [ns] O
        LogGP model additional parameters (inter cluster): 47938.9 [ns] o, 4980.51 [ns] g, 0.084 [ns] O
        Eager threshold: 65535 bytes

        ----------------------------------------------------------------
        Simulation runtime in DisCosTiC: 15.7239 [s]
        ----------------------------------------------------------------

        ----------------------------------------------------------------
        FULL APPLICATION PERFORMANCE (for all MPI processes)
        rank                         runtime [s]
        ----------------------------------------------------------------
        0                              2154.83
        1                              2154.83
        .                              .......
        .                              .......
        .                              .......
        71                             2154.67
        ----------------------------------------------------------------

<a name="cluster-configuration"></a>
## 💻 System model

The system parameters, either hypothetical or actual, can be tuned by editing the [config.cfg](config.cfg) file available in the current directory.
For the detailed documentation of the system model, please take a look at the [config.cfg](config.cfg) file and the `YAML files` available in the [machine files](nodelevel/machine-files) and [network files](interconnectlevel/network-files) directories.

<a name="cluster"></a>    
🔌 **Cluster model**:thought_balloon:

The resource allocation, intercluster characteristics, and runtime modalities are all included in the cluster model.

Metadata information                  | Description 
--------------------- | -------------
`number_of_nodes`             		    | Number of utilizing nodes on a cluster 
`task_per_node`			                | Number of utilizing cores on the node of a cluster 
`number_of_processes`                   | Number of running processes on the cluster 
`inter_cluster or heterogeneous`		    | Communication across clusters (0: inter cluster disabled; 1: inter cluster enabled)
`number_of_iterations`                   | Number of iterations for the program run
`dim_x, dim_y, dim_z`                   | Problem size for the program run; high-dimensional parameters will be disregarded for low-dimensional problems.

<a name="network"></a>    
🔌 **Network model**:thought_balloon:

The YAML formatted network files and the choice of performance model and mode of communication are included in the network model.

Metadata information                  | Description
------------------------------------- | -------------
`name`                                      | Name of the YAML formatted interconnect file
`intra_chip`							    | Communication inside chip
`inter_chip`							    | Communication across chips
`inter_node`							    | Communication across nodes
`latency`							        | Latency in sec for various kinds of interconnects
`bandwidth`							        | Bandwidth in GB/s for various kind of interconnects
`eager_limit`							    | Data size in bytes at which communication mode changes from eager to rendezvous protocol
`waitio_mode`                               | Mode of the WaitIO MPI library (socket, file or hybrid)
`comm_model`                                | Performance model for communication (0: LogGP, 1: HOCKNEY)

For illustration, a few examples of available [network files](interconnectlevel/network-files) are given below:

Network files (YAML format)                 | Description
--------------------- | -------------
`InfiniBand_WaitIO_intercluster`          |       Communication across clusters for WaitIO MPI library and InfiniBand interconnect
`InfiniBand_WaitIO_internode`          |       Communication across nodes for WaitIO MPI library and InfiniBand interconnect
`InfiniBand_WaitIO_interchip`          |       Communication across chips for WaitIO MPI library and InfiniBand interconnect
`InfiniBand_WaitIO_intrachip`          |       Communication inside the chip for WaitIO MPI library and InfiniBand interconnect
`Tofu-D_WaitIO_internode`          |       Communication across nodes for WaitIO MPI library and Tofu-D interconnect
`Tofu-D_WaitIO_interchip`          |       Communication across chips for WaitIO MPI library and Tofu-D interconnect
`Tofu-D_WaitIO_intrachip`          |       Communication inside the chip for WaitIO MPI library and Tofu-D interconnect
`InfiniBand_IntelMPI_internode`          |       Communication across nodes for Intel MPI library and InfiniBand interconnect
`InfiniBand_IntelMPI_interchip`          |       Communication across chips for Intel MPI library and InfiniBand interconnect
`InfiniBand_IntelMPI_intrachip`          |       Communication inside the chip for Intel MPI library and InfiniBand interconnect
`OmniPath_IntelMPI_internode`          |       Communication across nodes for Intel MPI library and Omni-Path interconnect
`OmniPath_IntelMPI_interchip`          |       Communication across chips for Intel MPI library and Omni-Path interconnect
`OmniPath_IntelMPI_intrachip`          |       Communication inside the chip for Intel MPI library and Omni-Path interconnect


<a name="node"></a>    
🔌 **Node model**:thought_balloon:

The machine files in YAML format, the selection of compiler settings and the performance model of computation are included in the node model.

Metadata information                  | Description
--------------------- | -------------
`name`                                      | Name of the YAML formatted processor file 
`sockets_per_node`                          | Number of sockets in one node 
`ccNUMA_domains_per_socket`                 | Number of ccNUMA domains in one socket 
`cores_per_ccNUMA_domain`                   | Number of cores in one ccNUMA domain 
`FP_instructions_per_cycle`					| Floating point instructions (ADD, MUL) per cycle 
`FP_operations_per_instruction_(SP/DP)`		| Single or double precision floating point operations per instruction
`clock_frequency`							| Clock frequency in GHz
`memory_bandwidth`							| Memory bandwidth in GB/s
`compiler-flags`                            | STD and SIMD optimizations of the compiler
`pmodel`                                | Performance model for computation (Roofline, ECM)

For illustration, a few examples of available [machine files](nodelevel/machine-files) are given below:

Machine files (YAML format)                 | Description
--------------------- | -------------
`A64FX`          |       48 core Fujitsu A64FX FX1000 CPU @ 1.8 GHz
`IcelakeSP_Platinum-8360Y`          |      36 core Intel(R) Xeon(R) Icelake SP Platinum 8360Y CPU @ 2.4 GHz 
`CascadelakeSP_Gold-6248_SNC`          |   20 core Intel(R) Xeon(R) Cascadelake SP Gold 6248 CPU @ 2.5 GHz  
`SkylakeSP_Gold-5122`          |       4 core Intel(R) Xeon(R) Skylake SP Gold 5122 CPU @ 3.6 GHz
`SkylakeSP_Gold-6148`          |       20 core Intel(R) Xeon(R) Skylake SP Gold 6148 CPU @ 2.4 GHz
`SkylakeSP_Gold-6148_SNC`          |    20 core Intel(R) Xeon(R) Skylake SP Gold 6148 CPU with SNC enabled @ 2.4 GHz   
`SkylakeSP_Platinum-8147_2.7GHz`          |      24 core Intel(R) Xeon(R) Skylake SP Platinum 8174 CPU @ 3.1 GHz
`BroadwellEP_E5-2630v4`          |       18 core Intel(R) Xeon(R) Broadwell EN/EP/EX E5-2697 v4 CPU @ 2.3 GHz
`BroadwellEP_E5-2697v4_CoD`          |       18 core Intel(R) Xeon(R) Broadwell EN/EP/EX E5-2697 v4 CPU with CoD enabled @ 2.3 GHz
`HaswellEP_E5-2695v3`          |       14 core Intel(R) Xeon(R) Haswell EN/EP/EX E5-2695 v3 CPU @ 2.3 GHz
`HaswellEP_E5-2695v3_CoD`          |       14 core Intel(R) Xeon(R) Haswell EN/EP/EX E5-2695 v3 CPU with CoD enabled @ 2.3 GHz
`IvyBridgeEP_E5-2660v2`          |       10 core Intel(R) Xeon(R) IvyBridge EN/EP/EX E5-2660 v2 CPU @ 2.2 GHz
`IvyBridgeEP_E5-2690v2`          |       10 core Intel(R) Xeon(R) IvyBridge EN/EP/EX E5-2690 v2 CPU @ 3.0 GHz
`SandyBridgeEP_E5-2680`          |    8 core Intel(R) Xeon(R) SandyBridge EN/EP E5-2680 CPU @ 2.7 GHz   

<a name="help"></a>    
**DisCostiC help**:thought_balloon:

The help of DisCostiC (`./discostic --help`) lists as:

```
Distributed Cost in Cluster (DisCostiC)
Version : v1.0.0
Author : Ayesha Afzal <ayesha.afzal@fau.de>
Copyright : 2024 HPC, FAU Erlangen-Nuremberg. All rights reserved

------------------------------------
 Arguments for ./discostic
------------------------------------
     --version, -v          show simulator's version information and exit
     --help, -h             show this help message and exit

------------------------------------
 Application model for config.cfg
------------------------------------
     benchmark_kernel       name of the kernel used in the program
     kernel_mode            mode of the kernel (FILE, SRC, LBL, COMP)

------------------------------------
 Cluster model for config.cfg
------------------------------------
     heteregeneous          a bool flag to enable or disable the second system (1: enabled, 0: disabled)
     number_of_iterations   number of iterations of the program
     dim_x, dim_y, dim_z    problem size; high-dimensional parameters will be disregarded for low-dimensional problems.
     task_per_node          number of running processes on one node
     number_of_processes    total number of running processes

------------------------------------
 Interconnect model for config.cfg
------------------------------------
     interconnect           name of the interconnect
     MPI_library            name of the MPI library for the first system (IntelMPI, WaitIO)
     comm_model             performance model of communication (0: LogGP, 1: HOCKNEY)
     waitio_mode            mode of the WaitIO MPI library (socket, file or hybrid)

------------------------------------
 Node model for config.cfg
------------------------------------
     micro_architecture     name of the YAML machine file
     compiler-flags         STD and SIMD optimizations (-03 -xCORE-AVX512 -qopt-zmm-usage=high, -03 -xHost -xAVX, -Kfast -DTOFU); If not set, flags are taken from the YAML formatted machine file.
     pmodel                 performance model of computation (ECM, Roofline)
     vvv                    a bool flag to enable or disable the verbose node output (0: disabled, 1: enabled)
     cache-predictor        cache prediction with layer conditions or cache simulation with pycachesim (LC, SIM)
     penalty                runtime penalty for the computation model in nanoseconds, used only in LBL or COMP mode

------------------------------------
 Delay injection mode for config.cfg
------------------------------------
     delay                  a bool flag to enable or disable the delay injection (0: disabled, 1: enabled)
     delay_intensity        intensity of delay as a multiple of computation time of one iteration
     delay_rank             process rank of the injected delay
     delay_timestep         iteration for the occurrence of the injected delay

------------------------------------
 Noise injection mode for config.cfg
------------------------------------
     noise                  a bool flag to enable or disable the noise injection (0: disabled, 1: enabled)
     noise_intensity        intensity of random noise, i.e., rand() % noise_intensity 
     noise_start_timestep   starting iteration for the injected noise
     noise_stop_timestep    ending iteration for the injected noise

------------------------------------
 Output for config.cfg
------------------------------------
     filename               output file name that contains details for the debug purpose
     chromevizfilename      output file name that contains all time-rank tracing data for visualization with Chrome tracing browser
     Verbose                a bool flag to enable or disable the verbose output (0: disabled, 1: enabled)
```

<a name="automating-dsel-generation-through-static-analysis"></a>
## 👩‍💻 Automating DSEL generation through static analysis

To perform the static analysis, the following procedure must be followed before running the DisCostiC batch script:
```
pip install -r staticanalysis/requirements.txt
python staticanalysis/Convert-<benchmark_kernel>.py
```

Files                  | Description
--------------------- | -------------
`Convert-<benchmark_kernel>.py`          | A helper script that takes the original code and, through annotation, locates code loops and communication and identifies user-defined variables, such as dim_x and dim_y in the Cartesian stencil heat.c code, and ultimately generates DSEL code.
`requirements.txt`               | Each (sub)dependency is listed and pinned using "==" to specify a particular package version; see [requirements.txt](staticanalysis/requirements.txt). This project makes use of the lightweight `Python tree data structure` [anytree==2.8.0](https://pypi.org/project/anytree) and `type hints for Python 3.7+` [typing_extensions==4.4.0](https://pypi.org/project/typing-extensions). These dependencies are later installed (normally in a virtual environment) through pip using the `pip install -r staticanalysis/requirements.txt` command. The generated tree's syntax is the same as the original C/C++ code.

For the specified program, this will produce the following files:

Files                  | Location       |Description
--------------------- | ------------- | -------------
`<benchmark_kernel>.c`          |  [nodelevel/kernels](nodelevel/kernels)  | It comprises only the generated computational loop kernel of the MPI parallel program.
`<benchmark_kernel>_<kernel_mode>.hpp`   |  [test](test)               | It includes the entire generated code expressed in DisCostiC DSEL language.

<a name="essential-routines"></a>
📝 **Essential DisCostiC routines**:thought_balloon:

The goal is to offer convenient, compact and practically usable application programming interfaces (APIs) with appropriate abstractions. It provides information about the call tree and attributes for communication (volume, mode, and protocol) and computation (data access pattern, flop count).
 

```cpp
1. DisCostiC->Rank_Init(DisCostiC::Indextype rank);

2. DisCostiC->SetNumRanks(DisCosTiC::Datatype numrank);

3. DisCostiC->Exec("LBL:STREAM_TRIAD", 
					DisCostiC::Event depending_operations);
   DisCostiC->Exec("COMP:TOL=2.0||TnOL=1.0|TL1L2=3.0|TL2L3=6.0|TL3Mem=14.2", 
   					DisCostiC::Event depending_operations);
   DisCostiC->Exec("FILE:copy.c//BREAK:COPY//./BroadwellEP_E5-2697v4_CoD.yml//18//-D N 100000", 
   					DisCostiC::Event depending_operations);
   DisCostiC->Exec("SRC:double s, a[N],b[N],c[N];\n\nfor(int i=0; i<N; i++)\n\ta[i]=b[i]+s*c[i];\n//BREAK:DAXPY//./BroadwellEP_E5-2697v4_CoD.yml//18//-D N 100000",
   					DisCostiC::Event depending_operations);

4. DisCostiC->Send(const void *message, 
                    DisCosTiC::Datatype sending_message_size_in_bytes, 
                    DisCostiC::Datatype sending_message_datatype, 
                    DisCostiC::Indextype destination_rank,
                    DisCostiC::Commtype communicator, 
                    DisCosTiC::Event depending_operations);

5. DisCostiC->Isend(const void *message, 
                    DisCosTiC::Datatype sending_message_size_in_bytes, 
                    DisCostiC::Datatype sending_message_datatype, 
                    DisCostiC::Indextype destination_rank,
                    DisCostiC::Commtype communicator,
                    DisCostiC::Request *request,
                    DisCosTiC::Event depending_operations);

6. DisCostiC->Recv(const void *message, 
                    DisCosTiC::Datatype receiving_message_size_in_bytes, 
                    DisCostiC::Datatype receiving_message_datatype, 
                    DisCostiC::Indextype source_rank,
                    DisCostiC::Commtype communicator, 
                    DisCosTiC::Event depending_operations);

7. DisCostiC->Irecv(const void *message,
                    DisCosTiC::Datatype receiving_message_size_in_bytes, 
                    DisCostiC::Datatype receiving_message_datatype, 
                    DisCostiC::Indextype source_rank,
                    DisCostiC::Commtype communicator,
                    DisCostiC::Request *request,
                    DisCosTiC::Event depending_operations);
					
8. DisCostiC->Rank_Finalize(); 

```

<a name="DSEL"></a>
📝 **Code blueprint as DisCostiC input**:thought_balloon:

The following DSEL code example shows the simplest illustration of how the domain knowledge of the applications is expressed in the DisCostiC language:

```cpp
DisCostiC::Event recv, send, comp;  
for (auto rank : DisCostiC::getRange(systemsize))
{
   DisCostiC->Rank_Init(rank);
   DisCosTiC->SetNumRanks(systemsize);
   /* initialization */
   comp = DisCostiC->Exec("STREAM_TRIAD", recv); 
   send = DisCostiC->Send(res, 8, ((rank + 1) % NP), MPI_COMM_WORLD, comp); 
      if(rank != 0)
      {
         recv = DisCostiC->Recv(res, 8, MPI_DOUBLE, rank - 1, MPI_COMM_WORLD, comp);
      } 
      else 
      {
         recv = DisCostiC->Recv(res, 8, MPI_DOUBLE, systemsize - 1, MPI_COMM_WORLD, comp);
      }	
   DisCostiC->Rank_Finalize(); 
} 
```

<a name="documentation"></a>    
📝 **Code documentation: suite of C++ data structures and enumerated types**:thought_balloon:

The HTML documentation can be generated by [doxygen](http://www.doxygen.nl). To build the documentation, navigate to the `Doxyfile` file available in the current directory and run the command:

`doxygen Doxyfile`

To read the documentation, point to a web browser at `html/index.html`.

* **Single Operation**: accessors for local operations and their individual information at a certain grid point

Metadata information                  | Description
------------------------------------- | -------------
`bufSize`						| Number of bytes (data size) transmitted in the communication operation (no real buffer size for comp, just added for completeness)
`DepOperations`				| Dependencies for blocking routines, i.e., other operations that depend on this current operation
`IdepOperations`				| Dependencies for non-blocking routines, i.e., other operations that depend on the current operation
`depCount`					| Number of dependencies for this current operation
`label`						| Index/identifier of this current operation for each rack
`target` 						| Rank of target/partner (source for recv or destination for send or no real target for comp but added for completeness)
`rank`      					| Owning rank of this current operation
`tag`							| Tag of this current operation	(no real tag for comp, just added for completeness)
`node`						| Node or processing element for this current operation						
`network`						| Type of network for this current operation
`time`						|	Ending time of this current operation
`starttime`					|	Starting time of this current operation
`type`								| Type of this current operation

```cpp
enum Operation_t {		| Operation_t defines different operation types of entities
	SEND = 1,         	| Send operation type
        RECV = 2,        	| Recv operation type
        COMP = 3,        	| Computation operation type
        MSG = 4         	| Message operation type
}; 
```

  				
Metadata information | Description 
--------------------- | -------------   
`mode`	|  mode of this current calling operation  

```cpp
enum Mode_t {		| Mode_t defines the operation type of communication
        NONBLOCKING, 	| Routines returning with the start of an operation so the next operation doesnot execute before starting the previous one; such as Isend, Irecv.
        BLOCKING     	| Routines returning only on completion of operation so the next operation doesnot execute before finishing the previous one; such as Send, Recv.
};
```

* **Performance model**

Metadata information     | Description
--------------------- | -------------
`model`				| Analytic first-principle performance model for computation and communication

```cpp
enum Model_t {		| Mode_t defines the used performance model 
        Roofline,	| Simple computation model type
        ECM,		| Advanced computation model type
        LOGGP,		| Simple communication model type
        HOCKNEY		| Advanced communication model type
};
```

* **Custom data types, keywords and high-level classes functionality**:thought_balloon:

    * **accessors** for local vectors and matrices and their individual components without the need for index accesses
    * **abstract base classes** for the AST-generated grid to access the operations and their associated features
    * **solver-specific data types**, e.g., time steps, operations, identifiers, etc., are declared globally (or part of a special global declaration block)
```cpp
    * DisCostiC::Timetype
    * DisCostiC::Datatype
    * DisCostiC::Indextype
    * DisCostiC::Networktype
```	


<a name="mpi-parallelized-implementation"></a>
## 🧐 MPI-parallelized DisCostiC implementation

The underlying principle of parallel simulation is that each operation's entire data is transmitted via blocking or non-blocking MPI routines to the processes it communicates with.

Terms                  | Description
--------------------- | -------------
`simulated processes P_i`    | processes from the application point of view
`simulator processes Q_i`    | processes from the parallel simulation framework point of view
`master simulator process Q_0`  | master process from the parallel simulation framework point of view


**Initialization**:thought_balloon:
In the MPI implementation, only the master process of the default communicator makes any print calls for debug and verbose output reporting, and all other processes of the new communicator initialize root operations only once in each run. 

<table>
<tr>
<th>MPI-parallelized</th>
<th>Serialized</th>
</tr>
<tr>
<td>
  
```cpp
int ierr, process_Rank, size_Of_Cluster, sucesss, N_process_Rank, N_size_Of_Cluster;
MPI_Comm newcomm;
ierr = MPI_Init(NULL, NULL);
ierr = MPI_Comm_rank(MPI_COMM_WORLD, &process_Rank);

MPI_Comm_split(MPI_COMM_WORLD, (process_Rank) < 1 ? 1 : 2, 1, &newcomm);
ierr = MPI_Comm_rank(newcomm, &N_process_Rank);
ierr = MPI_Comm_size(newcomm, &N_size_Of_Cluster);
ierr = MPI_Comm_rank(MPI_COMM_WORLD, &process_Rank);
ierr = MPI_Comm_size(MPI_COMM_WORLD, &size_Of_Cluster);
std::queue<int> test;
MPI_Status status;
MPI_Request request;
    
DisCostiC_Timetype starttimeProcess;
DisCostiC::DisCostiC_OP operation, operation_og;		
int iter = 0;

iter = ((rankLocalIt->getNumOps()) / 3);

if (rank == N_process_Rank)
{
 operation_og = copy(op, operation_og);
 operation = op;
 starttimeProcess = operation.starttime;
}
```
  
</td>
<td>

```cpp
op.numOpsInQueue = numOpsInQueue++;
queue.emplace(op);
```

</td>
</tr>
</table>

**Start of simulation**:thought_balloon:
Do loop over the number of iterations and the number of steps per iteration. Processes connected to the new communicator, or "newcomm," are only used in the main simulation.

<table>
<tr>
<th>MPI-parallelized</th>
<th>Serialized</th>
</tr>
<tr>
<td>
  
```cpp
rank = N_process_Rank;
bool cycle = true;
int iter_num = 1;
do{ //!< over the number of iterations
  do{ //!< over the number of steps per iteration
```
  
</td>
<td>

```cpp
do{
  queue.pop();
```

</td>
</tr>
</table>
Within each ccNUMA domain, runtime corrections are made by monitoring the number of processes that are concurrently engaged in each computation and communication step. 

**SEND operation**:thought_balloon:
The currently active process sends the operation object or array to the process listed as "operation.target," which is specific to the communication pattern simulation program.
 
<table>
<tr>
<th>MPI-parallelized</th>
<th>Serialized</th>
</tr>
<tr>
<td>
  
```cpp
operation.type = DisCostiC::Operation_t::RECV;
operation.starttime = operation.time;

int temp = oSuccessor[operation.rank][operation.node];
                        
double arr[13];                   
*serialize(operation, temp, arr);
if (operation.mode == DisCostiC::Mode_t::NONBLOCKING){
  MPI_Isend(&arr, 13, MPI_DOUBLE, operation.target, operation.tag, newcomm, &request);
}else{
  MPI_Send(&arr, 13, MPI_DOUBLE, operation.target, operation.tag, newcomm);
}

finishedRankList.push_back(operation.rank);
operation.type = DisCostiC::Operation_t::RECV;
cycle = true;
```
  
</td>
<td>

```cpp
std::swap(operation.rank,operation.target);
queue.emplace(operation);
```

</td>
</tr>
</table>

**RECV operation**:thought_balloon:
The currently running process receives the operation object or array from the process listed as "operation.target".
 
<table>
<tr>
<th>MPI-parallelized</th>
<th>Serialized</th>
</tr>
<tr>
<td>
  
```cpp
double brr[13];
if (operation.mode == DisCostiC::Mode_t::NONBLOCKING){
  MPI_Irecv(&brr, 13, MPI_DOUBLE, MPI_ANY_SOURCE, operation.tag, newcomm, &request);
}else{
  MPI_Irecv(&brr, 13, MPI_DOUBLE, MPI_ANY_SOURCE, operation.tag, newcomm, &request);
}
*deserialize(operation, temp, brr);                   
oSuccessor[operation.rank][operation.node] = brr[12];
cycle=true;
operation.numOpsInQueue = numOpsInQueue++;
finishedRankList.push_back(operation.rank);

operation.type = DisCostiC::Operation_t::MSG;
```
  
</td>
<td>

```cpp
finishedRankList.push_back(operation.rank);
DisCostiC::DisCostiC_queueOP matchedOP;
if(M.listmatch(operation, &UMQ[operation.rank], &matchedOP))

else { 
 DisCostiC::DisCostiC_queueOP nOp;
 nOp.bufSize = operation.bufSize;
 nOp.src = operation.target;
 nOp.tag = operation.tag;
 nOp.label = operation.label;
 recvQueue[operation.rank].push_back(nOp);
}
```

</td>
</tr>
</table>

**MSG operation**:thought_balloon:
If MSG is not found, the cycle will continue (cycle = true) until it is found.
 
<table>
<tr>
<th>MPI-parallelized</th>
<th>Serialized</th>
</tr>
<tr>
<td>
  
```cpp
if ((earliestFinish = operation.time) <= ...){
 ...
 operation.numOpsInQueue = numOpsInQueue++;
 ...
 iter_num++;		
 cycle = false;		        		
}else{
 ...
 cycle = true;
}
```
  
</td>
<td>

```cpp
if ((earliestFinish = operation.time) <= ...){
 ...
 DisCostiC::DisCostiC_queueOP matchedOP;
 if(M.listmatch(operation, &recvQueue[operation.rank], &matchedOP)) { 
 ...
 }else { 
 #ifdef USE_VERBOSE 
 std::cout << "[MSG ==> NOT FOUND] in receive queue - add to unexpected queue"<< std::endl;
 #endif
 DisCostiC::DisCostiC_queueOP nOp;
 nOp.bufSize = operation.bufSize;
 nOp.src = operation.target;
 nOp.tag = operation.tag;
 nOp.label = operation.label;
 nOp.starttime = operation.time; // when it was started
 UMQ[operation.rank].push_back(nOp);
 }
}else{
 ...
 queue.emplace(operation);
}
```

</td>
</tr>
</table>

**Adding new sorted operations in queue in order**:thought_balloon:
<table>
<tr>
<th>MPI-parallelized</th>
<th>Serialized</th>
</tr>
<tr>
<td>
  
```cpp

```
  
</td>
<td>

```cpp
newOps=false;

newOps = true;
op.numOpsInQueue=numOpsInQueue++; 

queue.emplace(op);
```

</td>
</tr>
</table>

**End of while statement**:thought_balloon:
The end of the do loops over the number of iterations and the number of steps per iteration.
The necessary information gathered from all processes of the new communicator is communicated to the default communicator's master process.
 
<table>
<tr>
<th>MPI-parallelized</th>
<th>Serialized</th>
</tr>
<tr>
<td>
  
```cpp
while (cycle);
 cycle = true;
 operation_og.time = oSuccessor[operation.rank][operation.node];
 operation = copy(operation_og, operation);
 } while (iter_num <= iter);
 double arr[13];
 double brr[13];
 *serialize(operation, oSuccessor[rank][operation.node], arr);
 *finalize(oSuccessor[rank][operation.node], numRanks, numOpsInQueue, rank, operation.node, brr);
 MPI_Send(&brr, 13, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD);
 }
 else if (process_Rank == 0)
 {
 bool cycle = true;
 double brr[13];
 while (test.size() < numRanks)
 {
   MPI_Recv(&brr, 13, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
   if (status.MPI_TAG == 1)
   {
     test.emplace(status.MPI_SOURCE);
     oSuccessor[brr[3]][brr[4]] = brr[0];
     numOpsInQueue = brr[2];
   }
 }
}
ierr = MPI_Comm_rank(MPI_COMM_WORLD, &process_Rank);
MPI_Barrier(MPI_COMM_WORLD);
```
  
</td>
<td>

```cpp
while(!queue.empty() || newOps)
```

</td>
</tr>
</table>


<a name="library-limitations"></a>
## 🚀 Planned features for further development

* Threading model beyond message passing
* Networking-level contention model
* Energy consumption model

<a name="references"></a>
## 📚 References about the theory of potential application scenarios for DisCostiC

[1]: A. Afzal et al.: Propagation and Decay of Injected One-Off Delays on Clusters: A Case Study. [DOI:10.1109/CLUSTER.2019.8890995](https://doi.org/10.1109/CLUSTER.2019.8890995)

[2]: A. Afzal et al.: Desynchronization and Wave Pattern Formation in MPI-Parallel and Hybrid Memory-Bound Programs. [DOI:10.1007/978-3-030-50743-5_20](https://doi.org/10.1007/978-3-030-50743-5_20)

[3]: A. Afzal et al.: Analytic Modeling of Idle Waves in Parallel Programs: Communication, Cluster Topology, and Noise Impact. [DOI:10.1007/978-3-030-78713-4_19](https://doi.org/10.1007/978-3-030-78713-4_19)

[4]: A. Afzal et al.: The Role of Idle Waves, Desynchronization, and Bottleneck Evasion in the Performance of Parallel Programs. [DOI:10.1109/TPDS.2022.3221085](https://doi.org/10.1109/TPDS.2022.3221085)

[5]: A. Afzal et al.: Analytic performance model for parallel overlapping memory-bound kernels. [DOI:10.1002/cpe.6816](https://doi.org/10.1002/cpe.6816)

[6]: A. Afzal et al.: Exploring Techniques for the Analysis of Spontaneous Asynchronicity in MPI-Parallel Applications. [DOI:10.1007/978-3-031-30442-2_12](https://doi.org/10.1007/978-3-031-30442-2_12)

[7]: A. Afzal et al.: Making applications faster by asynchronous execution: Slowing down processes or relaxing MPI collectives. [DOI:10.1016/j.future.2023.06.017](https://10.1016/j.future.2023.06.017)

<a name="license"></a>
## 🔒 License
 
[AGPL-3.0](LICENSE)
 
<a name="disclaimer"></a>
## :warning: Disclaimer

> [!NOTE]
> A note to the reader: Please report any bugs to the issue tracker or contact [ayesha.afzal@fau.de](ayesha.afzal@fau.de).

<a name="acknowledgement"></a>
## 🔗 Acknowledgement

This work is funded by the **[KONWHIR](https://www.konwihr.de)** project **OMI4PAPPS**.

<a name="contact"></a>
## 📱Contact

[<img src="https://github.com/AyeshaAfzal91.png?size=115" width=115><br><sub>@AyeshaAfzal91</sub>](https://github.com/AyeshaAfzal91) <br><br> 
Ayesha Afzal, Erlangen National High Performance Computing Center (NHR@FAU)

mailto: [ayesha.afzal@fau.de](mailto:ayesha.afzal@fau.de)


