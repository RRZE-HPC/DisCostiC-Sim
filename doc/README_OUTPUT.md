Part of the [main README](../README.md#further-reading).

<a name="output"></a>
## :signal_strength: DisCostiC output

<a name="chrome"></a>
🌐 **1. Web interface with Google Chrome browser (DMS file format)**:speech_balloon:

Upon completion of the run, the controller rank merges the per-rank Chrome traces into `<chromevizfilename>.dms`. The prefix comes from `chromevizfilename` in [config.cfg](../config.cfg) (the shipped file uses `DisCosTiC`).
The `.dms` file is a [Chrome trace JSON document](https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU/edit).
Load it in Google Chrome with:

```
chrome://tracing
```

<a name="vampir"></a>
📊 **2. Graphical user interface with Vampir (OTF2 file format)**:speech_balloon:

`make otf2` runs `python visualization/chrome2otf2.py -i DisCosTiC.dms -o DisCosTiC -c`. That writes an [OTF2](https://perftools.pages.jsc.fz-juelich.de/cicd/otf2/tags/otf2-3.0.2/html/) archive in the `DisCosTiC` directory (`DisCosTiC/traces.otf2`). Open a small trace in [Vampir](https://vampir.eu) with:

```
vampir DisCosTiC/traces.otf2
```

<a name="itac"></a>
📊 **3. Graphical user interface with ITAC (STF file format)**:speech_balloon:

Two different ITAC products exist.

The modeled application is traced inside the simulator when `USE_ITACVIZ` is enabled. Rank files are merged to `DisCosTiC.avt`. `make stf` runs [visualization/convert_itac_stf.sh](../visualization/convert_itac_stf.sh) on that file and writes `DisCosTiC.single.stf`.

Separately, `make trace_MPI` or `make trace_all` rebuilds the simulator with Intel `-trace` so ITAC can record the simulator process itself. There is no separate launch script for that trace. Open the resulting STF file in [Intel Trace Analyzer](https://www.intel.com/content/www/us/en/developer/tools/oneapi/trace-analyzer.html).

<a name="stat"></a>
🗄️ **4. Statistical or log data**:thought_balloon:

With `Verbose = 1` in [config.cfg](../config.cfg), DisCostiC prints per-rank runtimes and the computation and communication model parameters. The listing below is an example of that log. The current binary prints `Version : v2.0.0 (Second release)`.

```text
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
```


