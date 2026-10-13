### TEST FILES/DSEL FILES:

- STREAM_FILE.hpp: To run single kenel test files use N parameter from congig file to determine the size of problem.
- TWOKERNEL.hpp: Stream-triad and triad in FILE mode
- PITRIADFILE.hpp: Stream-triad and triad in FILE mode. Because divide can't get ECM values with kerncraft. Nsteps in config file for part1 and N is for part2.
- PITRIADLBL.hpp: First run with PISOLVERFILE then make changes to DIVIDE.cfg. N in config file is for STREAM and NSTEPS is for DIVIDE. (First it does DIVIDE and then STREAM) 
- UNI.hpp: To check unidirectional communication works or not after changes.
- RTOF4.hpp: Replication of RT-OF application. It is LBL mode. First run PITRIADFILE.hpp. In config file: Nsteps RT problem size and N OF problem size.

### nodelevel/configs:

Contains config files of DIVIDE.cfg to use when given PITRIADLBL.hpp similarly RT.cfg and OF.cfg.

### Important:

For handling memory bandwidth contention model for two kernels. Must always have first kernel COMPUTE and second kernel MEMORY or both MEMORY

- Currently, it is set to ``` if ((bound == MEMORY) && scaling_cores != -1) ``` compatable when two kernels are memory bound and dealing with single kernel simulations.

- To handle this for two kernels first kernel should be compute and second should be memory
    ``` if ((bound == MEMORY) && scaling_cores != -1 && operation.type == DisCosTiC::Operation_t::COMPADD) ```
- When both kernels are memory bound
    ``` if ((bound == MEMORY) && scaling_cores != -1) ```

- When using one kernel in the config.cfg don't forget to make ``` part2=0 ```

### Config file options to recreate the results of thesis:

- When starting to run, first run PITRIADFILE.hpp test case.

```
Nsteps: problem size of stream

N: problem size of triad

part1: STREAM

part2: TRIAD
```

- Now, go to src/Discostic.cpp file and change the memory bandwidth contention model if condition to:
    ``` if ((bound == MEMORY) && scaling_cores != -1 && operation.type == DisCosTiC::Operation_t::COMPADD) ```

To achieve results similar to 55:45

```
Nsteps = 3600000000 (problem size of DIVIDE) 

N = 1360000000 (problem size of stream-triad)

part1 = DIVIDE

part2 = TRIAD
```

- To recreate Raytracing Optical Flow solver 55:45 (best case) scenario which is when multigrid level: 9 and Supersampling factor: 2, use RTOF4.hpp test. 

```
Nsteps = 266256 (problem size of RT)

N = 66564 (problem size of OF)

part1 = RT

part2= OF
```
