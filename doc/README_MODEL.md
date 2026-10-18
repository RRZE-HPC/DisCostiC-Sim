Part of the [main README](../README.md#further-reading).

<a name="cluster-configuration"></a>
## 💻 System model

The system parameters, either hypothetical or actual, can be tuned by editing the [config.cfg](../config.cfg) file available in the current directory.
Machine descriptions are YAML files in [machine files](../nodelevel/machine-files). Network descriptions are numeric `.cfg` tables in [network files](../nodelevel/network-files).

<a name="cluster"></a>
🔌 **Cluster model**:thought_balloon:

The resource allocation, intercluster characteristics, and runtime modalities are all included in the cluster model.

| Metadata information                  | Description |
| --------------------- | ------------- |
| `task_per_node` | Number of processes placed on one node |
| `number_of_processes` | Number of simulated processes. Launch the simulator with this count plus one. |
| `number_of_timesteps` | Number of iterations. `collective_mode = 1` forces this to one. |
| `heteregeneous` | Second system (0: off, 1: primary plus secondary processes). Documented in [Heterogeneous mode](README_COLLECTIVE.md#heterogeneous-mode). |
| `dim_x, dim_y, dim_z` | Problem size. High-dimensional parameters are ignored for low-dimensional problems. |

<a name="interconnect"></a>
🔌 **Interconnect model**:thought_balloon:

Each network file is one scope (intra-chip, inter-chip, inter-node, or inter-cluster). A row is `number_of_simultaneous_process, msg_size_in_bytes, latency_in_ns, inverse_bandwidth, comm_eagerlimit_in_bytes, G_in_ns, comm_o_in_ns, comm_g_in_ns, comm_O_in_ns`. [config.cfg](../config.cfg) selects the model with `comm_model` (0: LogGP, 1: Hockney) and the WaitIO mode with `waitio_mode` (`socket`, `file`, or `hybrid`).

| Column | Meaning |
| --------------------- | ------------- |
| `latency_in_ns` | Latency parameter L, in nanoseconds |
| `inverse_bandwidth` | Inverse bandwidth, GB/s interpreted as bytes per nanosecond |
| `comm_eagerlimit_in_bytes` | Message size at which the model switches from eager to rendezvous |
| `comm_o_in_ns`, `comm_g_in_ns`, `comm_O_in_ns` | LogGP overhead, gap, and per-byte overhead |

The [network files](../nodelevel/network-files) shipped in this tree are:

| Network file | Scope |
| --------------------- | ------------- |
| [A64FX_interchip.cfg](../nodelevel/network-files/A64FX_interchip.cfg) | A64FX inter-chip |
| [A64FX_internode.cfg](../nodelevel/network-files/A64FX_internode.cfg) | A64FX inter-node |
| [A64FX_intrachip.cfg](../nodelevel/network-files/A64FX_intrachip.cfg) | A64FX intra-chip |
| [A64FX_waitio_socket_interchip.cfg](../nodelevel/network-files/A64FX_waitio_socket_interchip.cfg) | A64FX WaitIO socket inter-chip |
| [A64FX_waitio_socket_internode.cfg](../nodelevel/network-files/A64FX_waitio_socket_internode.cfg) | A64FX WaitIO socket inter-node |
| [A64FX_waitio_socket_intrachip.cfg](../nodelevel/network-files/A64FX_waitio_socket_intrachip.cfg) | A64FX WaitIO socket intra-chip |
| [BroadwellEP_E5-2630v4_interchip.cfg](../nodelevel/network-files/BroadwellEP_E5-2630v4_interchip.cfg) | Broadwell EP E5-2630 v4 inter-chip |
| [BroadwellEP_E5-2630v4_internode.cfg](../nodelevel/network-files/BroadwellEP_E5-2630v4_internode.cfg) | Broadwell EP E5-2630 v4 inter-node |
| [BroadwellEP_E5-2630v4_intrachip.cfg](../nodelevel/network-files/BroadwellEP_E5-2630v4_intrachip.cfg) | Broadwell EP E5-2630 v4 intra-chip |
| [IcelakeSP_Platinum-8360Y_interchip.cfg](../nodelevel/network-files/IcelakeSP_Platinum-8360Y_interchip.cfg) | Ice Lake SP Platinum 8360Y inter-chip |
| [IcelakeSP_Platinum-8360Y_internode.cfg](../nodelevel/network-files/IcelakeSP_Platinum-8360Y_internode.cfg) | Ice Lake SP Platinum 8360Y inter-node |
| [IcelakeSP_Platinum-8360Y_intrachip.cfg](../nodelevel/network-files/IcelakeSP_Platinum-8360Y_intrachip.cfg) | Ice Lake SP Platinum 8360Y intra-chip |
| [IcelakeSP_Platinum-8360Y_waitio_socket_interchip.cfg](../nodelevel/network-files/IcelakeSP_Platinum-8360Y_waitio_socket_interchip.cfg) | Ice Lake SP Platinum 8360Y WaitIO socket inter-chip |
| [IcelakeSP_Platinum-8360Y_waitio_socket_internode.cfg](../nodelevel/network-files/IcelakeSP_Platinum-8360Y_waitio_socket_internode.cfg) | Ice Lake SP Platinum 8360Y WaitIO socket inter-node |
| [IcelakeSP_Platinum-8360Y_waitio_socket_intrachip.cfg](../nodelevel/network-files/IcelakeSP_Platinum-8360Y_waitio_socket_intrachip.cfg) | Ice Lake SP Platinum 8360Y WaitIO socket intra-chip |
| [SapphireRapids_Platinum-8470_interchip.cfg](../nodelevel/network-files/SapphireRapids_Platinum-8470_interchip.cfg) | Sapphire Rapids Platinum 8470 inter-chip |
| [SapphireRapids_Platinum-8470_internode.cfg](../nodelevel/network-files/SapphireRapids_Platinum-8470_internode.cfg) | Sapphire Rapids Platinum 8470 inter-node |
| [SapphireRapids_Platinum-8470_intrachip.cfg](../nodelevel/network-files/SapphireRapids_Platinum-8470_intrachip.cfg) | Sapphire Rapids Platinum 8470 intra-chip |
| [waitio_socket_intercluster.cfg](../nodelevel/network-files/waitio_socket_intercluster.cfg) | WaitIO socket inter-cluster |


<a name="node"></a>
🔌 **Node model**:thought_balloon:

The machine files in YAML format, the selection of compiler settings and the performance model of computation are included in the node model.

| Metadata information                  | Description | 
| --------------------- | ------------- | 
| `name`                                      | Name of the YAML formatted processor file | 
| `sockets_per_node`                          | Number of sockets in one node | 
| `ccNUMA_domains_per_socket`                 | Number of ccNUMA domains in one socket | 
| `cores_per_ccNUMA_domain`                   | Number of cores in one ccNUMA domain | 
| `FP_instructions_per_cycle`					| Floating point instructions (ADD, MUL) per cycle | 
| `FP_operations_per_instruction_(SP/DP)`		| Single or double precision floating point operations per instruction | 
| `clock_frequency`							| Clock frequency in GHz | 
| `memory_bandwidth`							| Memory bandwidth in GB/s | 
| `compiler-flags`                            | STD and SIMD optimizations of the compiler | 
| `pmodel`                                | Performance model for computation (Roofline, ECM) | 

For illustration, a few examples of available [machine files](../nodelevel/machine-files) are given below:

| Machine files (YAML format)                 | Description | 
| --------------------- | ------------- | 
| [A64FX](../nodelevel/machine-files/A64FX.yml)           |       48 core Fujitsu A64FX FX1000 CPU @ 1.8 GHz   | 
| [IcelakeSP_Platinum-8360Y](../nodelevel/machine-files/IcelakeSP_Platinum-8360Y.yml)          |      36 core Intel(R) Xeon(R) Icelake SP Platinum 8360Y CPU @ 2.4 GHz         | 
| [CascadelakeSP_Gold-6248_SNC](../nodelevel/machine-files/CascadelakeSP_Gold-6248_SNC.yml)           |   20 core Intel(R) Xeon(R) Cascadelake SP Gold 6248 CPU @ 2.5 GHz      | 
| [SkylakeSP_Gold-5122](../nodelevel/machine-files/SkylakeSP_Gold-5122.yml)           |       4 core Intel(R) Xeon(R) Skylake SP Gold 5122 CPU @ 3.6 GHz  |  
| [SkylakeSP_Gold-6148](../nodelevel/machine-files/SkylakeSP_Gold-6148.yml)           |       20 core Intel(R) Xeon(R) Skylake SP Gold 6148 CPU @ 2.4 GHz  | 
| [SkylakeSP_Gold-6148_SNC](../nodelevel/machine-files/SkylakeSP_Gold-6148_SNC.yml)           |    20 core Intel(R) Xeon(R) Skylake SP Gold 6148 CPU with SNC enabled @ 2.4 GHz   |   
| [SkylakeSP_Platinum-8147](../nodelevel/machine-files/SkylakeSP_Platinum-8147_2.7GHz.yml)           |      24 core Intel(R) Xeon(R) Skylake SP Platinum 8174 CPU @ 3.1 GHz (2.7 GHz used)     | 
| [BroadwellEP_E5-2630v4](../nodelevel/machine-files/BroadwellEP_E5-2630v4.yml) | 10 cores/socket Intel Xeon E5-2630 v4 @ 2.20 GHz |
| [SapphireRapids_Platinum-8470](../nodelevel/machine-files/SapphireRapids_Platinum-8470.yml) | 52 cores/socket Intel Xeon Platinum 8470 @ 2.00 GHz |
| [ARMv8_ThunderX2-CN9980](../nodelevel/machine-files/ARMv8_ThunderX2-CN9980.yml) | 32 cores/socket ThunderX2 CN9980; clock in the file depends on SKU (2.0-2.5 GHz) |
| [Zen5_EPYC-9965](../nodelevel/machine-files/Zen5_EPYC-9965.yml) | 192 cores/socket AMD EPYC 9965 @ 2.03 GHz |
| [Zen2_EPYC-7452](../nodelevel/machine-files/Zen2_EPYC-7452.yml) | 32 cores/socket AMD EPYC 7452 @ 2.35 GHz |
| [Zen_EPYC-7451](../nodelevel/machine-files/Zen_EPYC-7451.yml) | 24 cores/socket AMD EPYC 7451 @ 2.3 GHz |
| [Zen_Ryzen7-1700X](../nodelevel/machine-files/Zen_Ryzen7-1700X.yml) | 8 cores/socket AMD Ryzen 7 1700X @ 2.90 GHz |
| [BroadwellEP_E5-2697v4_CoD](../nodelevel/machine-files/BroadwellEP_E5-2697v4_CoD.yml)           |       18 core Intel(R) Xeon(R) Broadwell EN/EP/EX E5-2697 v4 CPU with CoD enabled @ 2.3 GHz | 
| [HaswellEP_E5-2695v3](../nodelevel/machine-files/HaswellEP_E5-2695v3.yml)           |       14 core Intel(R) Xeon(R) Haswell EN/EP/EX E5-2695 v3 CPU @ 2.3 GHz | 
| [HaswellEP_E5-2695v3_CoD](../nodelevel/machine-files/HaswellEP_E5-2695v3_CoD.yml)           |       14 core Intel(R) Xeon(R) Haswell EN/EP/EX E5-2695 v3 CPU with CoD enabled @ 2.3 GHz  | 
| [IvyBridgeEP_E5-2660v2](../nodelevel/machine-files/IvyBridgeEP_E5-2660v2.yml)           |       10 core Intel(R) Xeon(R) IvyBridge EN/EP/EX E5-2660 v2 CPU @ 2.2 GHz | 
| [IvyBridgeEP_E5-2690v2](../nodelevel/machine-files/IvyBridgeEP_E5-2690v2.yml)           |       10 core Intel(R) Xeon(R) IvyBridge EN/EP/EX E5-2690 v2 CPU @ 3.0 GHz | 
| [SandyBridgeEP_E5-2680](../nodelevel/machine-files/SandyBridgeEP_E5-2680.yml)           |    8 core Intel(R) Xeon(R) SandyBridge EN/EP E5-2680 CPU @ 2.7 GHz   | 

<a name="help"></a>
**DisCostiC help**:thought_balloon:

`./discostic --help` and `./discostic --version` print the text from `help()` and `version()` in [include/macro.hpp](../include/macro.hpp). The version line is `v1.0 (initial release)`. The parameter list is:

```
task_per_node              number of runing processes on one node
number_of_processes        number of runing processes on all nodes
idlewave                   1 to insert delay and create idlewave.
which_rank                 which rank should introduce the disturbance
at_timestep                at which timestep the given rank should introduce the disturbance
desync                     0 to turn off the desynchronization feature and 1 to turn on.
comm_model                 0 for LogGP model and 1 for HOCKNEY model
filename                   [optional: for debug puspose] output file name that contain details of the generated graph
timeunit_conv              time multiplier for conversation from baseline unit of nanoseconds to some other unit
chromevizfilename          the name of the output file contains all time rank tracing data for visulization with Chrome tracing browser
Verbose                    enable/disable more verbose output (0=disable, 1=enable)
batchmode                  enable/disable batchmode (0=disable i.e., printing performance for each rank, 1=enable i.e., never print detailed all rank info)
micro_architecture
FP_instructions_per_cycle
benchmark_kernel
P2P_mode                   choose either of FILE, SRC, LBL, SRC modes. Concatenated with benchmark kernel during runtime to determine which P2P file to use.
FLOPs_per_iteration        number of flops in one iteration
R_streams                  number of read streams
W_streams                  number of write streams
datasize_in_GB             number of array elements * (R_streams + W_streams) * size of datatype
penalty                    penalty in nanoseconds. Used only in LBL and COMP mode.
compiler-flags
pmodel
vvv
cache-predictor
```

`P2P_mode` is what `--help` prints. The shipped [config.cfg](../config.cfg) leaves `P2P_mode` commented out and sets `benchmark_kernel` to the full header name. `heteregeneous`, `collective_mode`, and `number_of_timesteps` are read by the simulator and are not listed by `--help`. The spelling `compiler-flags` in the help text does not match `compiler_flags` in `config.cfg`.

<a name="automating-dsel-generation-through-static-analysis"></a>
## 👩‍💻 Automating DSEL generation through static analysis

Static analysis is the most reliable for regular communication and known problem sizes. Unstructured or rank-unique patterns need a hand-written DSEL, and some kernels (for example HPCG SpMV) need manual extensions.

To perform the static analysis, the following procedure must be followed before running the DisCostiC batch script:
```
pip install -r staticanalysis/requirements.txt
cd staticanalysis
python Convert-<benchmark_kernel>.py
```
Converters in this tree include `Convert-HEAT.py`, `Convert-HPCG.py`, `Convert-STREAM.py`, and `Convert-POISSONNS.py`.

| Files                  | Description | 
| --------------------- | ------------- | 
| `Convert-<benchmark_kernel>.py`          | A helper script that takes the original code and, through annotation, locates code loops and communication and identifies user-defined variables, such as dim_x and dim_y in the Cartesian stencil heat.c code, and ultimately generates DSEL code. | 
| `requirements.txt`               | Each (sub)dependency is listed and pinned using "==" to specify a particular package version; see [requirements.txt](../staticanalysis/requirements.txt). This project makes use of the lightweight `Python tree data structure` [anytree==2.8.0](https://pypi.org/project/anytree) and `type hints for Python 3.7+` [typing_extensions==4.4.0](https://pypi.org/project/typing-extensions). These dependencies are later installed (normally in a virtual environment) through pip using the `pip install -r staticanalysis/requirements.txt` command. The generated tree's syntax is the same as the original C/C++ code. | 

For the specified program, this will produce the following files:

| Files                  | Location       |Description | 
| --------------------- | ------------- | ------------- | 
| `<benchmark_kernel>.c`          |  [nodelevel/kernels](../nodelevel/kernels)  | It comprises only the generated computational loop kernel of the MPI parallel program. | 
| `<benchmark_kernel>_<kernel_mode>.hpp`   |  [test](../test)               | It includes the entire generated code expressed in DisCostiC DSEL language. | 

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
   DisCostiC->Exec("FILE:stream-triad.c//BREAK:STREAM//./nodelevel/machine-files/IcelakeSP_Platinum-8360Y.yml//18//-D N 1000000", 
   					DisCostiC::Event depending_operations);
   DisCostiC->Exec("SRC:double s, a[N],b[N],c[N];\n\nfor(int i=0; i<N; i++)\n\ta[i]=b[i]+s*c[i];\n//BREAK:STREAM//./nodelevel/machine-files/IcelakeSP_Platinum-8360Y.yml//18//-D N 1000000",
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
					
8. DisCostiC->Collective(collectiveType,
                    root,
                    size_bytes,
                    rank,
                    numRanks,
                    depending_operations,
                    algorithm);

9. DisCostiC->Rank_Finalize(); 

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
   comp = DisCostiC->Exec("LBL:STREAM_TRIAD", recv);
   send = DisCostiC->Send(res, 8, MPI_DOUBLE, (rank + 1) % systemsize, MPI_COMM_WORLD, comp);
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

Collective calls use the signature in routine 8 above. Mode flags, algorithms, and the multi-kernel and heterogeneous placements are in [README_COLLECTIVE.md](README_COLLECTIVE.md).

<a name="documentation"></a>
📝 **Code documentation: suite of C++ data structures and enumerated types**:thought_balloon:

The HTML and LaTeX documentation can be generated by [doxygen](http://www.doxygen.nl). To build the documentation, navigate to the [Doxyfile](../Doxyfile) file available in the current directory and run the command:

```
doxygen Doxyfile
```
After `doxygen Doxyfile`, the HTML tree is `doc/html/index.html` (it is not checked in). The documentation shipped with this tree is the Markdown guides in `doc/`.

* **Single Operation**: accessors for local operations and their individual information at a certain grid point

| Metadata information                  | Description   | 
| ------------------------------------- | ------------- | 
| `bufSize`						| Number of bytes (data size) transmitted in the communication operation (no real buffer size for comp, just added for completeness)        | 
| `DepOperations`				| Dependencies for blocking routines, i.e., other operations that depend on this current operation      | 
| `IdepOperations`				| Dependencies for non-blocking routines, i.e., other operations that depend on the current operation   | 
| `depCount`					| Number of dependencies for this current operation             | 
| `label`						| Index/identifier of this current operation for each rack      | 
| `target` 						| Rank of target/partner (source for recv or destination for send or no real target for comp but added for completeness)    | 
| `rank`      					| Owning rank of this current operation     | 
| `tag`							| Tag of this current operation	(no real tag for comp, just added for completeness) | 
| `node`						| Node or processing element for this current operation		    | 				
| `network`						| Type of network for this current operation    | 
| `time`						|	Ending time of this current operation   | 
| `starttime`					|	Starting time of this current operation | 
| `type`								| Type of this current operation    | 

```cpp
enum Operation_t { // operation types
    SEND = 1,      // send
    RECV = 2,      // receive
    COMP = 3,      // computation
    MSG = 4        // message
};
```

  				
| Metadata information | Description | 
| --------------------- | -------------  |  
| `mode`	|  mode of this current calling operation |  

```cpp
enum Mode_t {  // blocking versus non-blocking communication
    NONBLOCKING, // Isend, Irecv: return when the operation starts
    BLOCKING     // Send, Recv: return when the operation finishes
};
```

* **Performance model**

| Metadata information     | Description | 
| --------------------- | ------------- | 
| `model`				| Analytic first-principle performance model for computation and communication | 

```cpp
enum Model_t { // performance model in use
    Roofline, // computation
    ECM,      // computation, with cache and memory layers
    LOGGP,    // communication
    HOCKNEY   // communication, latency and bandwidth
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

