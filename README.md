<p align="center">
  <img src="doc/Logo/DisCostiC-symbol.png" alt="DisCostiC logo" width="140">
</p>

<h1 align="center">DisCostiC: Distributed Cost in Cluster</h1>

<a href="https://rrze-hpc.github.io/DisCostiC-Sim/doc/html/index.html">
  <img alt="HTML Doxygen doc" src="https://img.shields.io/badge/HTML%20Doxygen%20Doc%20-Read%20now!-red" />
</a>
<a href="https://github.com/RRZE-HPC/DisCostiC-Sim/wiki/DisCostiC">
  <img alt="Wiki Page" src="https://img.shields.io/badge/Wiki%20Page%20-Read%20now!-pink" />
</a>
<a href="https://matrix.to/#/!OeNPngEkrqRRkBJjwz:gwaaf.rrze.fau.de?via=gwaaf.rrze.fau.de">
  <img alt="Join the discussion on Matrix" src="https://img.shields.io/badge/Discussions%20%26%20Support-Chat%20now!-yellow" />
</a>
<a href="https://www.linkedin.com/in/ayeshaafzal-/" target="_parent">
  <img alt="linkedin" src="https://i.stack.imgur.com/gVE0j.png" />
</a>
<a href="https://github.com/AyeshaAfzal91" target="_parent">
  <img alt="" src="https://img.shields.io/github/stars/tanstack/react-table.svg?style=social&label=Star" />
</a>
<a href="https://twitter.com/AyeshaHamad4" target="_parent">
  <img alt="" src="https://img.shields.io/twitter/follow/AyeshaHamad4.svg?style=social&label=Follow" />
</a>
<a href="https://doi.org/10.5281/zenodo.10606290">
  <img alt="DisCostiC Zenodo release" src="https://zenodo.org/badge/DOI/10.5281/zenodo.10606290.svg" />
</a>
<a href="https://github.com/RRZE-HPC/DisCostiC-Sim/releases">
  <img alt="GitHub Release" src="https://img.shields.io/badge/GitHub%20Release%20-See%20now!-purple" />
</a>


## Table of Contents

  * [:octocat: Compilation and build](#compilation-and-build)
  * [📖 Further reading](#further-reading)
    * [💡Description](doc/README_DESCRIPTION.md#description)
    * [:signal_strength: DisCostiC output](doc/README_OUTPUT.md#output)
    * [💻 System model](doc/README_MODEL.md#cluster-configuration)
    * [👩‍💻 Automating DSEL generation through static analysis](doc/README_MODEL.md#automating-dsel-generation-through-static-analysis)
    * [🧭 Modes](doc/README_COLLECTIVE.md#modes)
    * [🧐 MPI-parallelized DisCostiC implementation](doc/README_MPI.md#mpi-parallelized-implementation)
    * [🚀 Planned features for further development](doc/README_TODO.md#library-limitations)
    * [📚 References about the theory of potential application scenarios for DisCostiC](doc/README_REF.md#references)
  * [🔒 License](#license)
  * [:warning: Disclaimer](#disclaimer)
  * [🔗 Acknowledgement](#acknowledgement)
  * [📱Contact](#contact)


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

The [test](test) folder holds one header per benchmark and kernel mode. [Run_Simulation.sh](Run_Simulation.sh) copies `test/<benchmark_kernel>.hpp` to `test/P2P.hpp`, so `benchmark_kernel` is the header name without `.hpp`. The shipped [config.cfg](config.cfg) sets `benchmark_kernel = STREAM_COLLECTIVE`. Examples of headers:

| Header name (`benchmark_kernel`) | Description |
| --------------------- | ------------- |
| `STREAM_COLLECTIVE` | STREAM compute plus an MPI collective |
| `STREAM_FILE`, `STREAM_LBL`, `STREAM_SRC`, `STREAM_COMP` | STREAM triad in one kernel mode |
| `HEAT_FILE` | Two-dimensional five-point Jacobi kernel |
| `SOR_LBL` | Gauss-Seidel successive over-relaxation |
| `HEATHEAT_FILE`, `HEATSOR_FILE`, `HEATDIVIDE_FILE` | Back-to-back kernels |
| `TWOKERNEL`, `PITRIADFILE`, `PITRIADLBL`, `RTOF4` | Multi-kernel workflows |
| `HPCG` | High Performance Conjugate Gradient |


`kernel_mode` changes how the kernel time is obtained. `FILE` and `SRC` run [Kerncraft](https://github.com/RRZE-HPC/kerncraft). `LBL` and `COMP` use recorded ECM numbers and do not. Further `Exec` strings are in [Essential DisCostiC routines](doc/README_MODEL.md#essential-routines). Modes other than these four are in [Modes](doc/README_COLLECTIVE.md).

| `kernel_mode`                 | Integrated tool | Description |
| --------------------- | ------------- | ------------- |
| `COMP`          |   no external tool integrated |  This directly embeds the single-core pre-recorded ECM performance model data of the computational kernel into the simulator. |
| `LBL`           |   no external tool integrated |  This reads the pre-recorded ECM performance model data for the computational kernel from an external file located at [nodelevel/configs](nodelevel/configs) folder. |
| `SRC`           |   [Kerncraft](https://github.com/RRZE-HPC/kerncraft) integrated      |  This directly embeds the source code of the computational kernel into the simulator. |
| `FILE`          |   [Kerncraft](https://github.com/RRZE-HPC/kerncraft) integrated      |  This reads the source code for the computational kernel from an external file located at [nodelevel/kernels](nodelevel/kernels) folder. |

Some `*_LBL.hpp` headers read `message_size` from [config.cfg](config.cfg) and use it as the byte count of the point-to-point exchange. `step_size`, `direction`, and `periodic` are not read by the headers on this tree.

The shipped [config.cfg](config.cfg) selects the header with `benchmark_kernel`. `FILE` and `SRC` headers run Kerncraft. `LBL` and `COMP` headers use recorded ECM numbers. The keys `./discostic --help` actually prints are listed in [DisCostiC help](doc/README_MODEL.md#help).

<a name="compilation"></a>
🥅 **Compilation**:thought_balloon:

The compilation offers the following choices to enable the output report's data format generation and to enable the tracing of the simulator's own implementation. An executable will be generated after compilation. 

| Command                  | Description |
| --------------------- | ------------- |
| `make`          | This enables JSON data format without ITAC profiling of simulator implementation. |
| `make otf2`          | This enables both JSON and OTF2 data format without ITAC profiling of simulator implementation. |
| `make trace_MPI`          | This enables JSON data format and the standard ITAC tracing mode of simulator implementation with the information about MPI call functions (enabled flag: `-trace`). |
| `make trace_all`               | This enables JSON data format and the verbose ITAC tracing mode of simulator implementation with the information on both user-defined and MPI call functions (enabled flag: `-trace -tcollect`). |

`make stf` does not rebuild the executable. It converts the modeled-application ITAC file `DisCosTiC.avt` to `DisCosTiC.single.stf`. See [ITAC output](doc/README_OUTPUT.md#itac).


<a name="run"></a>
:green_circle: **Run**:thought_balloon:

[Run_Simulation.sh](Run_Simulation.sh) reads [config.cfg](config.cfg) in place and launches `number_of_processes + 1` ranks on Fritz (`singlenode`). Submit it from the repository root. The allocation must be at least that many tasks.

| Command                  | Description |
| --------------------- | ------------- |
| `sbatch Run_Simulation.sh`          | This runs the simulation described by the current `config.cfg`. |

<a name="clean"></a>
🔁 **Clean and uninstall**:thought_balloon:

| Command                  | Description |
| --------------------- | ------------- |
| `make clear`            |   This cleans up the working directory by removing all unnecessary DisCostiC files, such as *.dms, *.otf, *.csv files. |
| `make uninstall`        |   This uninstalls the DisCostiC framework, including installed files and CMake-specific files. |


<a name="further-reading"></a>
## 📖 Further reading

  * [💡 Description, workflow, and advantages](doc/README_DESCRIPTION.md)
  * [:signal_strength: DisCostiC output](doc/README_OUTPUT.md)
  * [💻 System model, DSEL, and data structures](doc/README_MODEL.md)
  * [🧭 Modes: kernel, collective, multi-kernel, and heterogeneous](doc/README_COLLECTIVE.md)
  * [🧐 MPI-parallelized DisCostiC implementation](doc/README_MPI.md)
  * [🚀 Planned features for further development](doc/README_TODO.md)
  * [📚 References about the theory of potential application scenarios for DisCostiC](doc/README_REF.md)

<a name="license"></a>
## 🔒 License
 
[AGPL-3.0](LICENSE)
 
<a name="disclaimer"></a>
## :warning: Disclaimer

> [!NOTE]
> A note to the reader: Please report any bugs to the issue tracker or contact [ayesha.afzal@fau.de](mailto:ayesha.afzal@fau.de).

<a name="acknowledgement"></a>
## 🔗 Acknowledgement

This work is funded by the **[DFG MOD4COMP RESEARCH UNIT](https://www.mod4comp.de)** "Holistic Energy and Performance Modeling for Sustainable Computing".

<a name="contact"></a>
## 📱Contact

[<img src="https://github.com/AyeshaAfzal91.png?size=115" width=115><br><sub>@AyeshaAfzal91</sub>](https://github.com/AyeshaAfzal91) <br><br> 
[Ayesha Afzal](https://hpc.fau.de/faudir/ayesha-afzal), Erlangen National High Performance Computing Center (NHR@FAU)

mailto: [ayesha.afzal@fau.de](mailto:ayesha.afzal@fau.de)


