# DisCostiC: Distributed Cost in Cluster

  </a><a href="https://DisCostiC-Sim.readthedocs.io/">
  <img alt="Docs" src="https://img.shields.io/badge/read-the_docs-blue" />
  </a><a href="https://github.com/RRZE-HPC/DisCostiC-Sim/src/Dev/discussions">
  <img alt="Join the discussion on Github" src="https://img.shields.io/badge/Github%20Discussions%20%26%20Support-Chat%20now!-blue" />
  </a><a href="https://www.linkedin.com/in/ayeshaafzal-/">
    <img alt="linkedin" src="https://i.stack.imgur.com/gVE0j.png" />
  <a href="https://github.com/AyeshaAfzal91" rel="nofollow noreferrer">
    <img alt="github" src="https://i.stack.imgur.com/tskMh.png">
  </a>
  </a><a href="https://github.com/RRZE-HPC/DisCostiC-Sim/src/Dev/" target="\_parent">
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
  * [:signal_strength: DisCostiC output](#visualisation)
    * [🌐 1. In-browser output visualization with Google Chrome (DMS file format)](#chrome)
    * [📊 2. Output visualization with Vampir (OTF2 file format)](#vampir)
    * [📊 3. Output visualization with ITAC (STF file format)](#itac)
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

A `cross-architecture resource-based parallel simulation framework` that can efficiently predict the performance of `real or hypothetical massively parallel MPI programs on current and future highly parallel heterogeneous systems.✨

The runtime predictions are carried out in a controlled environment through _`straightforward, portable, scalable, and efficient simulations`_.⭐

The simulation framework is an automated version of `analytical first-principle performance models at a full-scale scope`, including cores, chips, nodes, networks, clusters, and their mutual interactions plus inherent bottlenecks such as memory bandwidth.🌟

The application blueprint created using `Domain Specific Embedded Language (DSEL)` enables key concepts and elements embedded in the modern C++ language, enhancing `readability for domain experts and accurately encoding inter-process dependencies without introducing unknown consequences from the actual systems`. 💥


<a name="workflow"></a>
⚙️ **Framework workflow**💬

```diff
- Application model

    * Domain knowledge of the application written in DisCostiC DSEL language

- Analytic first-principle performance models
    
    * Computation models: Fundamental analytic Roofline and refined ECM models with including memory bandwidth bottleneck concept at system level
    * Communication model: Fundamental latency-bandwidth and refined LogGP models

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
	
+ Last but not least, an open-source low entry cost lightweight simulator enabling model-based design-space exploration 
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
One way to check the installation is to print the version of the DisCostiC using `./discostic --version`.

<a name="configuration"></a>
⏱️ **Configuration settings**:thought_balloon:

The [test](test) folder in DisCostiC offers multiple MPI-parallelized programs (`benchmark_kernel`) in distinct functionality for computation (`kernel_mode`) and communication (`exchange_mode`). For illustration, a few examples are given below:

`benchmark_kernel`                 | Description
--------------------- | -------------
`HEAT`          |       Two-dimensional five-point Jacobi
`STREAM`               | STREAM Triad
`SOR`          |    Gauss-Seidel Successive Over-Relaxation solver
`HPCG`               | High Performance Conjugate Gradients

`kernel_mode`                 | Description
--------------------- | -------------
`COMP`          |       no external tool 
`LBL`               |   no external tool
`FILE`          |       [Kerncraft](https://github.com/RRZE-HPC/kerncraft) integrated
`SRC`               | [Kerncraft](https://github.com/RRZE-HPC/kerncraft) integrated

`exchange_mode`                  | Description
--------------------- | -------------
`direction`          |       It specifies the direction of message exchange as an int (1: uni-directional upwards shift in only positive direction, 2: bi-directional upwards and downwards shift in both positive and negative directions) 
`stepsize`          |       It describes the step size of message exchange as an int (1: distance one communication, 2: distance two communication, ...)
`periodic`          |       It enables or disables the periodicity as a bool (1: true, 0: false)


<a name="compilation"></a>
🥅 **Compilation**:thought_balloon:

The compilation offers following choices to enable the output report's data formats generation and to enable the tracing of the simulator's own implementation. An executable will be generated after compilation. 

Command                  | Description
--------------------- | -------------
`make`          | This enables JSON data format without ITAC profiling of simulator implementation.
`make otf2`          | This enables both JSON and OTF2 data format without ITAC profiling of simulator implementation.
`make trace_MPI`          | This enables JSON data format and the standard ITAC tracing mode with the information about MPI call functions (enabled flag: `-trace`).
`make trace_all`               | This enables JSON data format and the verbose ITAC tracing mode with the information on both user-defined and MPI call functions (enabled flag: `-trace -tcollect flag`).


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
`make uninstall`        |   This uninstalls the DisCostiC framework, including installed files and CMAKE specific files.


<a name="visualization"></a>
## :signal_strength: DisCostiC output visualization

<a name="chrome"></a>
🌐 **1. In-browser output visualization with Google Chrome (DMS file format)**:speech_balloon:

Upon completion of the run, DisCostiC generates a report referred to as `DisCostiC.dms`.
`DisCostiC.dms` is a straightforward [JSON object data format file](https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU/edit) that can be viewed using the [Google Chrome browser](chrome://tracing).
Use the following Google Chrome web browser to load the generated JSON file:

```
    chrome://tracing
```

<a name="vampir"></a>
📊 **2. Output visualization with Vampir (OTF2 file format)**:speech_balloon:

The `DisCosTiC/traces.otf2` file is an [OTF2 object data format](https://perftools.pages.jsc.fz-juelich.de/cicd/otf2/tags/otf2-3.0.2/html/) file and can be viewed using third-party tools like [ITAC](https://www.intel.com/content/www/us/en/developer/tools/oneapi/trace-analyzer.html) and [Vampir](https://vampir.eu).
This format is produced by the [ChromeTrace2Otf2](https://profilerpedia.markhansen.co.nz/converters/otf2-cli-chrome-trace-converter/) converter, which converts the JSON object data format file `(DisCostiC.dms)` to the OTF2 object data format file `(DisCosTiC/traces.otf2)`.

Use the following commond to open an OTF2 small trace file (`DisCosTiC/traces.ot2`) in [Vampir](https://vampir.eu):

```
    vampir DisCostiC/traces.otf2
```

<a name="itac"></a>
📊 **3. Output visualization with ITAC (STF file format)**:speech_balloon:

To convert a DMS file to a single.stf file format and to open an STF trace file (DisCosTiC.single.stf) in [ITAC](https://www.intel.com/content/www/us/en/developer/tools/oneapi/trace-analyzer.html), invoke the Intel Trace Analyzer GUI and follow the below steps:

```
    (1) traceanalyzer &
    (2) Go to File > Open; from the Files of a type field, select Open Trace Format, navigate to the `DisCosTiC/traces.otf2` file, and double-click to open it
    (3) The OTF2 to STF conversion dialog appears. Review the available fields and checkboxes, and click Start to start the conversion. As a result, the OTF2 file will be converted to STF (DisCosTiC/traces.otf2.single.stf), and you will be able to view it in the Intel Trace Analyzer.
```

<a name="stat"></a>
🗄️ **4. Statistical or log data**:thought_balloon:

		----------------------------------------------------------------
		DisCostiC:
		----------------------------------------------------------------
		full form: Distributed CosT in Cluster
		version: v0.0.0 (initial release)
		timestamp: Sun Oct 27 14:24:48 2019
		author: Ayesha Afzal <ayesha.afzal@fau.de>
        	copyright: © 2023 HPC, FAU Erlangen-Nuremberg. All rights reserved
        
		verbose							| more verbose output (0=disable, 1=enable)
		progress						| progress printing information (0=disable, 1=enable)
		batchmode						| batchmode (0=disable i.e., print runtime for each rank, 1=enable i.e., only print runtime for slowest rank)
		operationCount					| total number of operations for DisCostiC test cases
		time							| DisCostiC speed / performance [unit: s]
		speed							| DisCostiC speed / performance [unit: operations/s]

		DisCostiC PERFORMANCE:
		----------------------------------------------------------------
		Number of operations    	Runtime [ns]    	 Speed [operations/ns]
		----------------------------------------------------------------
		800     			7.79465e+09   		1.02634e-07

		FULL APPLICATION PERFORMANCE (for all ranks):
		----------------------------------------------------------------
		rank            runtime [ns]     runtime [s]
		----------------------------------------------------------------
		0            	27710          	2.771e-05
		1            	28210          	2.821e-05
		.				.....			.........
		.				.....			.........
		.				.....			.........    	     
		N-1            	30710          	3.071e-05


<a name="cluster-configuration"></a>
## 💻 System model

All system parameters (hypothetical or actual) can be tuned by editing the [config.cfg](config.cfg) file available in the current directory.
For the detailed documentation of the system model, please take a look at the [config.cfg](config.cfg) file and the `YAML files` available in the [machine files](nodelevel/machine-files) and [network files](nodelevel/network-files) directories.

<a name="cluster"></a>    
🔌 **Cluster model**:thought_balloon:

|Metadata information                  | Description |
--------------------- | -------------
|`name`										| Name of the cluster, processor, network, or running kernel |
|`number of homogeneous nodes`				| Number of nodes in the cluster |
|`number of sockets per node`				| Number of sockets in one node |
|`number of cores per socket`				| Number of cores in one socket |
|`number of pinned node`					| Number of utilizing nodes of the cluster |
|`numbering of pinned chips cores`			| Number of utilizing cores of the chip in the cluster |
|`inter cluster`							| Communication across clusters


<a name="network"></a>    
🔌 **Network model**:thought_balloon:

Metadata information                  | Description
--------------------- | -------------
`intra chip`							    | Communication inside chip
`inter chip`							    | Communication across chips
`inter node`							    | Communication across nodes
`latency`							        | Networking latency parameter for various kind of network [unit: s]
`bandwidth`							        | Networking bandwidth parameter for various kind of network [unit: GB/s]
`eager limit`							    | Data size at which communication mode change from eager to rendezvous protocol for various kind of network [unit: bytes]

For illustration, a few examples of available [network files](nodelevel/network-files) are given below:

Network files (YAML format)                 | Description
--------------------- | -------------
`InfiniBand_WaitIO_intercluster`          |       Communication across clusters for WaitIO library and InfiniBand interconnect
`InfiniBand_WaitIO_internode`          |       Communication across nodes for WaitIO library and InfiniBand interconnect
`InfiniBand_WaitIO_interchip`          |       Communication across chips for WaitIO library and InfiniBand interconnect
`InfiniBand_WaitIO_intrachip`          |       Communication inside chip for WaitIO library and InfiniBand interconnect
`Tofu-D_WaitIO_internode`          |       Communication across nodes for WaitIO MPI library and Tofu-D interconnect
`Tofu-D_WaitIO_interchip`          |       Communication across chips for WaitIO MPI library and Tofu-D interconnect
`Tofu-D_WaitIO_intrachip`          |       Communication inside chip for WaitIO MPI library and Tofu-D interconnect
`InfiniBand_IntelMPI_internode`          |       Communication across nodes for Intel library and InfiniBand interconnect
`InfiniBand_IntelMPI_interchip`          |       Communication across chips for Intel library and InfiniBand interconnect
`InfiniBand_IntelMPI_intrachip`          |       Communication inside chip for Intel library and InfiniBand interconnect
`OmniPath_IntelMPI_internode`          |       Communication across nodes for Intel MPI library and Omni-Path interconnect
`OmniPath_IntelMPI_interchip`          |       Communication across chips for Intel MPI library and Omni-Path interconnect
`OmniPath_IntelMPI_intrachip`          |       Communication inside chip for Intel MPI library and Omni-Path interconnect


<a name="node"></a>    
🔌 **Node model**:thought_balloon:

Metadata information                  | Description
--------------------- | -------------
`FP instructions per cycle`					| Floating point instructions per cycle (ADD, MUL)
`FP operations per instruction (SP/DP)`		| Single precision/ double precision floating point operations per instruction
`clock frequency`							| Clock frequency [unit: GHz]
`memory bandwidth`							| Memory bandwidth [unit: GB/s]


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

<a name="automating-dsel-generation-through-static-analysis"></a>
## 👩‍💻 Automating DSEL generation through static analysis

To perform this static analysis, the following procedure must be followed before runing the discostic batch script:
```
pip install -r staticanalysis/requirements.txt
python staticanalysis/Convert-<program>.py
```

Files                  | Description
--------------------- | -------------
`Convert-<program>.py`          | A helper script that takes the original code and, through annotation, locates code loops and communication and identifies user-defined variables, such as dimx and dimy in the Cartesian stencil heat.c code, and ultimately generates DSEL code.
`requirements.txt`               | Each (sub)dependency is listed and pinned using "==" to specify a particular package version. This project makes use of the lightweight `Python tree data structure` [anytree==2.8.0](https://pypi.org/project/anytree) and `type hints for Python 3.7+` [typing_extensions==4.4.0](https://pypi.org/project/typing-extensions). These dependencies are later installed (normally in a virtual environment) through pip using the `pip install -r requirements.txt` command. The generated tree's syntax is the same as the original C/C++ code.

For the specified program, this will produce following files:

Files                  | Folder location       |Description
--------------------- | ------------- | -------------
`<program>.cfg`          |  nodelevel/configs  | It contains the generated computational loop kernel.
`<program>_<mode>.hpp`   |  test               | It contains the generated DSEL code.

<a name="essential-routines"></a>
📝 **Essential DisCostiC routines**:thought_balloon:

The goal is to offer convenient, compact and practically usable application programming interfaces (APIs) with appropriate abstractions.

```cpp
1. DisCostiC->Rank_Init(DisCostiC::Indextype rank);

2. DisCostiC->Exec("LBL:STREAM_TRIAD", 
					DisCostiC::Event depending_operations);
   DisCostiC->Exec("COMP:TOL=2.0||TnOL=1.0|TL1L2=3.0|TL2L3=6.0|TL3Mem=14.2", 
   					DisCostiC::Event depending_operations);
   DisCostiC->Exec("FILE:copy.c//BREAK:COPY//./BroadwellEP_E5-2697v4_CoD.yml//18//-D N 100000", 
   					DisCostiC::Event depending_operations);
   DisCostiC->Exec("SRC:double s, a[N],b[N],c[N];\n\nfor(int i=0; i<N; i++)\n\ta[i]=b[i]+s*c[i];\n//BREAK:DAXPY//./BroadwellEP_E5-2697v4_CoD.yml//18//-D N 100000",
   					DisCostiC::Event depending_operations);

3. DisCostiC->Send(DisCostiC::Datatype sending_message_size_in_bytes, 
					DisCostiC::Indextype destination_rank, 
					DisCostiC::Event depending_operations);

4. DisCostiC->Isend(DisCostiC::Datatype sending_message_size_in_bytes, 
					DisCostiC::Indextype destination_rank, 
					DisCostiC::Event depending_operations);

5. DisCostiC->Recv(DisCostiC::Datatype receiving_message_size_in_bytes, 
					DisCostiC::Indextype source_rank, 
					DisCostiC::Event depending_operations);

6. DisCostiC->Irecv(DisCostiC::Datatype receiving_message_size_in_bytes,
					DisCostiC::Indextype source_rank,
					DisCostiC::Event depending_operations);
					
7. DisCostiC->Rank_Finalize(); 

```

<a name="DSEL"></a>
📝 **Code blueprint as DisCostiC input**:thought_balloon:

This DSEL code example shows a simplest illustration of how the domain knowledge of the applications is expressed in the DisCostiC language:

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
--------------------- | -------------
`bufSize`						| Number of bytes (data size) transmitted in the communication operation (no real buffer size for comp, just added for completeness)
`DepOperations`				| Dependencies for blocking routines, i.e., other operations that depend on this current operation
`IdepOperations`				| Dependencies for non-blocking routines, i.e., other operations that depend on current operation
`depCount`					| Number of dependencies for this current operation
`label`						| Index/identifier of this current operation for each rack
`target` 						| Rank of target/partner (source for recv / dest for send / no real target for comp, just added for completeness)
`rank`      					| Owning rank of this current operation
`tag`							| Tag of this current operation	(no real tag for comp, just added for completeness)
`node`						| Node or proceessing element for this current operation						
`network`						| Type of network for this current operation
`time`						|	Ending time of this current operation
`starttime`					|	Starting time of this current operation
`type`								| Type of this current operation

```
enum Operation_t {		| Operation_t enum defines different opertation types of entities
	SEND = 1,         	| Send operation type
        RECV = 2,        	| Recv operation type
        COMP = 3,        	| Compuation operation type
        MSG = 4         	| Message operation type
}; 
```

  				
Metadata information | Description 
--------------------- | -------------   
`mode`	|  mode of this current calling operation  

```
enum Mode_t {		| Mode_t enum defines operation type of SEND and RECV entities (send/isend and recv/irecv)
        NONBLOCKING, 	| Routines that return with start of operation (next operation not be executed before starting of previous operation)
        BLOCKING     	| Routines that return only on completion of operation (next operation not be executed before finishing of previous operation)
};
```

* **Performance model**

Metadata information     | Description
--------------------- | -------------
`Model_t`				| analytic first-principle performance model for computation and communication

```
enum Model_t {		| Mode_t enum defines the used performance model 
        Roofline,		| Simple computation model type
        ECM,		| Advanced computation model type
        LOGGP,		| Simple communication model type
        HOCKNEY		| Advanced communication model type
};
```

* **Custom data types, keywords and high-level classes functionality**:thought_balloon:

    * **accessors** for local vectors and matrices and their individual components without the need for index accesses
    * **abstract base classes** for the AST generated grid to access the operations and their associated features
    * **solver-specific data types**, e.g., time steps, operations, identifiers etc., are declared globally (or part of a special global declaration block)
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
`simulator processes Q_i`    | processes from the parallel simulation framework point of view
`simulated processes P_i`    | processes from the application point of view
`master simulator process Q_0`  | master process from the parallel simulation framework point of view


**Initialization**:thought_balloon:
In the MPI implementation, only the master process of the default communicator makes any print calls, and all other processes of the new communicator initialize root operations only once in each run. 

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
 Debug why the discostic Irecv and Recv have a BLOCKING mode_t value.
 
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
End of the do loops over the number of iterations and the number of steps per iteration.
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

