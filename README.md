# DisCostiC: Distributed Cost in Cluster
======================================================================================

  </a><a href="https://DisCostiC-Sim.readthedocs.io/en/latest/?badge=latest">
  <img alt="Documentation Status" src="https://readthedocs.org/projects/DisCostiC-Sim/badge/?version=latest" />
  </a><a href="https://DisCostiC-Sim.readthedocs.io/">
  <img alt="Docs" src="https://img.shields.io/badge/read-the_docs-blue" />
  </a><a href="https://github.com/RRZE-HPC/DisCostiC-Sim/src/Dev/discussions">
  <img alt="Join the discussion on Github" src="https://img.shields.io/badge/Github%20Discussions%20%26%20Support-Chat%20now!-blue" />
  </a><a href="https://github.com/RRZE-HPC/DisCostiC-Sim/src/Dev/actions">
  <img alt="Build Status" src="https://github.com/RRZE-HPC/DisCostiC-Sim/src/Dev/workflows/test-n-publish/badge.svg?branch=master&event=push" />
  </a><a href="https://github.com/RRZE-HPC/DisCostiC-Sim/src/Dev/" target="\_parent">
  <img alt="" src="https://img.shields.io/github/stars/tanstack/react-table.svg?style=social&label=Star" />
  </a><a href="https://twitter.com/AyeshaHamad4" target="\_parent">
  <img alt="" src="https://img.shields.io/twitter/follow/AyeshaHamad4.svg?style=social&label=Follow" />
  </a>

Table of Contents
=================

  * [Description](#description)
    * [Framework workflow](#workflow)
    * [Advantages over existing tools](#advantages)
  * [Compilation and build](#compilation-and-build)
  * [DisCostiC output](#visualisation)
  * [Code documentation](#documentation)
    * [Suite of C++ data structures and enumerated types](#suite-of-c---data-structures-and-enumerated-types)
  * [Cluster configuration: Hypothetical or actual](#cluster-configuration)
    * [Cluster level](#cluster)
    * [Node level](#node)
    * [Network level](#network)
  * [DSEL code example: Hypothetical or actual](#example)
    * [Seven essential DisCostiC routines](#seven-essential-routines)
    * [Code blueprint as DisCostiC input](#DSEL)
  * [Automating DSEL generation through static analysis](#automating-dsel-generation-through-static-analysis)
  * [MPI-parallelized implementation](#mpi-parallelized-implementation)
  * [Planned features for further development](#library-limitations)
  * [License](#license)
  * [Disclaimer](#disclaimer)
  * [Acknowledgement](#acknowledgement)
  * [Contact](#contact)

<a name="description"></a>
## Description:speech_balloon:
-------------------------
A cross-architecture resource-based parallel simulation framework that can efficiently predict the performance of real or hypothetical massively parallel MPI programs on current and future heterogeneous systems.

This simulator aims to predict the runtime for massively parallel applications in a _`straightforward, portable, scalable, and efficient`_ manner.:star:

The simulation framework is _`built on first-principles analytical performance models`_.:star2:

It is specifically tuned and designed to simulate MPI parallel micro-benchmarks or proxy application test cases that use highly parallel systems and are _`compute- and memory-bound`_ (such as stencil codes).:sparkles:

The toolkit is intended to serve as a vehicle for testing the performance of any _`actual or hypothetical parallel distributed memory applications and systems`_ under controlled experimental conditions.:fire:

DisCostiC facilitates _`key concepts and elements embedded in the modern C++ language`_ to cater for higher expressivity and readability for domain experts.:boom:


<a name="workflow"></a>
**Framework workflow**:speech_balloon:

```diff
- Dependency graph generation

	ASTs for managing the semantics of MPI message matching and
    the topology of the system (i.e., multiple chips, nodes and network interfaces).

- Performance models
    
    * Computation models: Fundamental analytic Roofline model and refined ECM model with chip-level contention concept included

    * Communication model: Straightforward latency-bandwidth and refined LogGP analytic models

- System model 
 
+ DisCostiC combines the above four, domain knowledge of the applications (written in DisCostiC language), and target platforms (config.cfg file) to simulate the runtime cost of distributed proxy applications
```

<a name="advantages"></a>
**Advantages over existing tools**:speech_balloon:

```diff
- Zero dependency 

	Except default compiler for C/C++ languages and Google Chrome browser for visualization

- A full parallel application toolkit based on analytical first-principle performance models

	The application is modeled inside DisCostiC tool that emulates target architectures

- Efficient speed

	* No intermediate tracing files requirement like any offline, trace-driven tools 
	* No high memory requirement like any online tools that use the host architectures to execute code
	
+ And last but not least, it's an open-source low entry cost lightweight toolkit 
    for leveraging the 3Ps: performance, portability, and productivity
```

<a name="compilation-and-build"></a>
## Compilation and build:speech_balloon:
---------------------------------------

First, clone the git repository using the command:
 
`$ git clone git@github.com:RRZE-HPC/DisCostiC-Sim.git && cd DisCostiC-Sim`
 
Before continuing, make sure the environment is prepared for the compilation.
The installation steps are listed below:
```
    $ module load python git intel intelmpi itac cmake
    $ conda create --name XYZ
    $ conda activate XYZ
    $ conda install pip 
    $ cmake -DCMAKE_INSTALL_PREFIX=~/.local . && make all install
```

**Configuration settings**:speech_balloon:
The test folder in DisCostiC offers four <testcase>s 

    1. HEAT
    2. STREAM
    3. HPCG
    4. SOR

of following four distinct functionalities:

	1. COMP <without Kerncraft>
	2. LBL <without Kerncraft>
	3. FILE <with Kerncraft>
	4. SRC <with Kerncraft>

Use one of these functionalities and test case of your choice:

`cp <testcase>.hpp P2P.hpp`

In the `config.cfg` file: 
```
1. The name of the kernel needs to be updated for the `benchmark_kernel` value
2. The number of the simulated processes needs to be updated as the `number of simulator processes - 1`
```
 
**Compilation without ITAC profiling**:speech_balloon:

```
$ make
```

**Compilation with ITAC profiling**:speech_balloon:

During compilation, the ITAC tracing can be enabled in two ways.

Command                  | Description
--------------------- | -------------
`$ make trace_MPI`          | This will enable standard tracing mode without the information on user-defined functions (enabled -trace flag).
`$ make trace_all`               | This will enable verbose tracing mode with information on user-defined functions (enabled -trace -tcollect flag).

**Run without ITAC profiling**:speech_balloon:

Run the batch script on Fritz using this command:

```
$ sbatch Run-Fritz.sh
```

**Run with ITAC profiling**:speech_balloon:

To generate only a single stf file (Exec.single.stf) in the DisCostiC directory, export the following variables in your bash or add them to the Run-Fritz.sh bash script beforehand when running the batch script on a system:

```
$ export VT_FLUSH_PREFIX=/tmp
$ export VT_LOGFILE_FORMAT=SINGLESTF
$ export VT_LOGFILE_NAME=Exec
$ sbatch Run-Fritz.sh
```

**Clean**:speech_balloon:

To clean up your working directory and get rid of all the unwanted DisCostiC files, including *.dms, *.otf, *.csv files, use this command:
```
$ make clear
```

**Uninstall**:speech_balloon:

To uninstall the whole DisCostiC framework, including installed files and CMAKE specific files, use this command:
```
$ make uninstall
```


<a name="visualization"></a>
## DisCostiC output:speech_balloon:
---------------------------------------

**1. In-browser output visualization with Google Chrome (DMS file format)**:speech_balloon:

Upon completion of the run, DisCostiC generates a report referred to as `DisCostiC.dms`.
`DisCostiC.dms` is a (straightforward, but non-optimized) [JSON object data format file](https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU/edit) that can be viewed using the [Google Chrome browser](chrome://tracing).
The generated JSON file can then be loaded after entering [chrome://tracing](chrome://tracing) in the URL bar of the Google Chrome browser.


**2. Output visualization with Vampir (OTF2 file format)**:speech_balloon:

The `DisCostiC.otf2` file is an [OTF2 object data format](https://perftools.pages.jsc.fz-juelich.de/cicd/otf2/tags/otf2-3.0.2/html/) file and can be viewed using third-party tools like [ITAC](https://www.intel.com/content/www/us/en/developer/tools/oneapi/trace-analyzer.html), [Vampir](https://vampir.eu), [Score-P](https://www.vi-hps.org/projects/score-p/), and others.
This format is produced by the [ChromeTrace2Otf2](https://profilerpedia.markhansen.co.nz/converters/otf2-cli-chrome-trace-converter/) converter, which converts the JSON object data format file `(DisCostiC.dms)` to the OTF2 object data format file `(DisCostiC.otf2)`.

To convert a DMS file to the OTF2 file format and to open an OTF2 small trace file (DisCosTiC.otf2) in [Vampir](https://vampir.eu) follow the below steps:
```
    $ make otf2
    $ module use ~unrz139/.modules/modulefiles
    $ module load vampir
    $ vampir DisCostiC/traces.otf2
```


**3. Output visualization with ITAC (STF file format)**:speech_balloon:

To convert a DMS file to a single.stf file format and to open an STF trace file (DisCosTiC.single.stf) in [ITAC](https://www.intel.com/content/www/us/en/developer/tools/oneapi/trace-analyzer.html) follow the below steps:

```
    (1) make otf2
    (2) module load itac
    (3) traceanalyzer & (invoke the Intel Trace Analyzer GUI)
    (4) Go to File > Open
    (5) From the Files of a type field, select Open Trace Format, navigate to the `DisCosTiC/traces.otf2` file, and double-click to open it
    (6) The OTF2 to STF conversion dialog appears. Review the available fields and checkboxes, and click Start to start the conversion. As a result, the OTF2 file will be converted to STF (DisCosTiC/traces.otf2.single.stf), and you will be able to view it in the Intel Trace Analyzer.
```

**4. Statistical or log data**:speech_balloon:

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


<a name="documentation"></a>    
## Code documentation:speech_balloon:
-------------------------

The HTML documentation can be generated by [doxygen](http://www.doxygen.nl). To build the documentation, navigate to the `Doxyfile` file available in the current directory and run the command:

`$ doxygen Doxyfile`

To read the documentation, point to a web browser at `html/index.html`.

<a name="suite-of-c---data-structures-and-enumerated-types"></a>
** Suite of C++ data structures and enumerated types:speech_balloon:**

* **Single Operation**: accessors for local operations and their individual information at a certain grid point

Metadata information                  | Description
--------------------- | -------------
`bufSize`						| number of bytes (data size) transmitted in the communication operation (no real buffer size for comp, just added for completeness)
`DepOperations`				| dependencies for blocking routines, i.e., other operations that depend on this current operation
`IdepOperations`				| dependencies for non-blocking routines, i.e., other operations that depend on current operation
`depCount`					| number of dependencies for this current operation
`label`						| index/identifier of this current operation for each rack
`target` 						| rank of target/partner (source for recv / dest for send / no real target for comp, just added for completeness)
`rank`      					| owning rank of this current operation
`tag`							| tag of this current operation	(no real tag for comp, just added for completeness)
`node`						| node or proceessing element for this current operation						
`network`						| type of network for this current operation
`time`						|	ending time of this current operation
`starttime`					|	starting time of this current operation
`type`								| type of this current operation

```
    				enum Operation_t{		| the Operation_t enum defines different opertation types of entities
        				SEND = 1,         	| send operation type
        				RECV = 2,        	| recv operation type
        				COMP = 3,        	| compuation operation type
        				MSG = 4,         	| message operation type
    				};
 
```

  				
Metadata information | Description 
--------------------- | -------------   
`mode`	|  mode of this current calling operation  

```
			    	enum Mode_t {		| the Mode_t enum defines operation type of SEND and RECV entities (send/isend and recv/irecv)
        				NONBLOCKING, 	| routines that return with start of operation (next operation not be executed before starting of previous operation)
        				BLOCKING,     	| routines that return only on completion of operation (next operation not be executed before finishing of previous operation)
    				};
```

* **Total Operations**

Metadata information     | Description
--------------------- | -------------
`sendCount`				| number of times send routines called
`recvCount`				| number of times recv routines called
`compCount`				| number of times comp routines called
`depCount`				| number of dependencies
 

* **Custom data types, keywords and high-level classes functionality**

    * **accessors** for local vectors and matrices and their individual components without the need for index accesses
    * **abstract base classes** for the AST generated grid to access the operations and their associated features
    * **solver-specific data types**, e.g., time steps, operations, identifiers etc., are declared globally (or part of a special global declaration block)
```cpp
	* DisCostiC::Timetype
	* DisCostiC::Datatype
	* DisCostiC::Indextype
	* DisCostiC::Networktype
```	

<a name="cluster-configuration"></a>
## Cluster configuration: Hypothetical or actual:speech_balloon:
-----------------------------------

All parameters (hypothetical or actual) can be tuned by editing the `config.cfg` file.
For detailed documentation, please take a look at the `config.cfg` file available in the current directory.

<a name="cluster"></a>    
**Cluster level**:speech_balloon:

|Metadata information                  | Description |
--------------------- | -------------
|`name`										| the name of the cluster, processor, network, or running kernel |
|`number of homogeneous nodes`				| the number of nodes in the cluster |
|`number of sockets per node`				| the number of sockets in one node |
|`number of cores per socket`				| the number of cores in one socket |
|`number of pinned node`					| the number of utilizing nodes of the cluster |
|`numbering of pinned chips cores`			| the numbering of utilizing cores of the chip in the cluster |

<a name="node"></a>    
**Node level**:speech_balloon:

Metadata information                  | Description
--------------------- | -------------
`FP instructions per cycle`					| node level floating point instructions per cycle (ADD, MUL)
`FP operations per instruction (SP/DP)`		| node level single precision/ double precision floating point operations per instruction
`clock frequency`							| node level clock frequency [unit: GHz]
`memory bandwidth`							| node level memory bandwidth [unit: GB/s]

<a name="network"></a>    
**Network level**:speech_balloon:

Metadata information                  | Description
--------------------- | -------------
`intra chip`							    | communication inside chip
`inter chip`							    | communication across chips
`inter node`							    | communication across nodes
`latency`							        | networking level latency parameter for various kind of network [unit: s]
`bandwidth`							        | networking level bandwidth parameter for various kind of network [unit: GB/s]
`eager limit`							    | data size at which communication mode change from eager to rendezvous protocol for various kind of network [unit: bytes]

<a name="example"></a>
## DSEL code example: Hypothetical or actual:speech_balloon:
-------------------------

<a name="seven-essential-routines"></a>
**Seven basic DisCostiC routines:speech_balloon:**

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
**Code blueprint as DisCostiC input**:speech_balloon:

The simplest illustration of how the domain knowledge of the applications is expressed in the DisCostiC language is provided below.

```cpp
DisCostiC::Event recv, send, comp;  
for (auto rank : DisCostiC::getRange(NP))
{
   DisCostiC->Rank_Init(rank);
   for (auto timestep : DisCostiC::getRange(5))
   {
      comp = DisCostiC->Exec("STREAM_TRIAD", recv); 
      send = DisCostiC->Send(8, ((rank + 1) % NP), comp); 
      if(rank != 0)
      {
         recv = DisCostiC->Recv(8, rank - 1, comp);
      } 
      else 
      {
   	     recv = DisCostiC->Recv(8, NP - 1, comp);
      }	
   } 
   DisCostiC->Rank_Finalize(); 
} 
```

<a name="automating-dsel-generation-through-static-analysis"></a>
## Automating DSEL generation through static analysis:speech_balloon:
---------------------------------------
  
To perform this static analysis, the following procedures must be followed:
```
$ cd staticanalysis
$ pip install -r requirements.txt
$ python Convert-<testcase>.py
```

Current <testcase> options:
```
1. HEAT
2. HPCG
3. STREAM
```

For the specified <testcase>, this will produce files with the names `converted.cfg` (which contains computational loop kernels) and `converted.c` (which contains the DSEL code) in the folders `../staticanalysis/nodelevel/configs` and `../staticanalysis/,` respectively.
Now, after completing the copying step listed below, run discostic as previously described.
```
$ cp ../test/P2P-<testcase>.hpp ../test/P2P.hpp
$ cd .. && ./DisCostiC.sh 
```

Files                  | Description
--------------------- | -------------
`Convert-<testcase>.py`          | A helper script that takes the original code and, through annotation, locates code loops and communication and identifies user-defined variables, such as dimx and dimy in the Cartesian stencil heat.c code, and ultimately generates DSEL code.
`requirements.txt`               | Each (sub)dependency is listed and pinned using "==" to specify a particular package version. This project makes use of the lightweight `Python tree data structure` [anytree==2.8.0](https://pypi.org/project/anytree) and `type hints for Python 3.7+` [typing_extensions==4.4.0](https://pypi.org/project/typing-extensions). These dependencies are later installed (normally in a virtual environment) through pip using the `pip install -r requirements.txt` command. The generated tree's syntax is the same as the original C/C++ code.


<a name="mpi-parallelized-implementation"></a>
## MPI-parallelized implementation:speech_balloon:
---------------------------------------

The underlying principle of parallel simulation is that each operation's entire data is transmitted via blocking or non-blocking MPI routines to the processes it communicates with.

Terms                  | Description
--------------------- | -------------
`simulator processes Q_i`    | processes from the parallel simulation framework point of view
`simulated processes P_i`    | processes from the application point of view
`master simulator process Q_0`  | master process from the parallel simulation framework point of view


```cpp
#include <mpi.h>

/**
 *\brief A function that converts an operation object into a double array.
 * The array, called "arr," needs to be communicated via pairwise MPI routines.
 */
double *serialize(DisCostiC::DisCostiC_OP op, double c, double arr[13])
{
    arr[0] = op.time;
    arr[1] = op.starttime;
    arr[2] = op.syncstart;
    arr[3] = op.numOpsInQueue;
    arr[4] = op.bufSize;
    arr[5] = op.target;
    arr[6] = op.rank;
    arr[7] = op.label;
    arr[8] = op.tag;
    arr[9] = op.node;
    arr[10] = op.network;
    arr[11] = op.type;
    arr[12] = c;
    return arr;
}

/**
 *\brief A function that limits operations to the necessary data only.
 * At the end of the simulation, all necessary data about operations from every process must be sent to the master process. 
 */
double *finalize(double a, double b, double c, double d, double e, double arr[13])
{
    arr[0] = a;
    arr[1] = b;
    arr[2] = c;
    arr[3] = d;
    arr[4] = e;
    return arr;
}
```

**Initialization**
/**
 *\brief In the MPI implementation, only the master process of the default communicator makes any print `std::cout` calls,
 * and all other processes of the new communicator initialize root operations only once in each run. 
 */

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

**Start of simulation**
/**
 *\brief Do loop over the number of iterations and the number of steps per iteration.
 * Processes connected to the new communicator, or "newcomm," are only used in the main simulation.
 */

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

**SEND operation**
/**
 *\brief The currently active process sends the operation object or array to the process listed as "operation.target,"
 * which is specific to the communication pattern simulation program.
 */
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

**RECV operation**
/**
 *\brief The currently running process receives the operation object or array from the process listed as "operation.target".
 * Debug why the discostic Irecv and Recv have a BLOCKING mode_t value.
 */
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

**MSG operation**
/**
 *\brief If MSG is not found, the cycle will continue (cycle = true) until it is found.
 */
 
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

**Adding new sorted operations in queue in order**
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

**End of while statement**
/**
 *\brief End of the do loops over the number of iterations and the number of steps per iteration.
 * The necessary information gathered from all processes of the new communicator is communicated to the default communicator's master process.
 */
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
**Planned features for further development**:thought_balloon:

	* Threading model beyond message passing
	* Networking-level contention model
    * Energy consumption model

<a name="license"></a>
## License:speech_balloon:
-------------------------
 
 `AGPL-3.0 </LICENSE>`
 
<a name="disclaimer"></a>
## Disclaimer:speech_balloon:
-------------------------

> A note to the reader: Please report any bugs to the issue tracker or contact [ayesha.afzal@fau.de](ayesha.afzal@fau.de"Ayesha Afzal").


<a name="acknowledgement"></a>
## Acknowledgement:speech_balloon:
-------------------------

This work is funded by the **[KONWHIR](https://www.konwihr.de"KONWHIR")** project **OMI4PAPPS**.

<a name="contact"></a>
## Contact:speech_balloon:
-------------------------

Ayesha Afzal, Erlangen National High Performance Computing Center (NHR@FAU)
Email: [ayesha.afzal@fau.de](ayesha.afzal@fau.de"Ayesha Afzal")
