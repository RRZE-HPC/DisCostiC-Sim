Part of the [main README](../README.md#further-reading).

<a name="description"></a>
## 💡 Description

DisCostiC is a `cross-architecture, resource-based parallel simulation framework` for predicting the performance of MPI programs on current and future HPC systems.✨

It schedules computation and communication from a C++ domain-specific embedded language (DSEL) and evaluates them with analytical models: Roofline and ECM for computation, and LogGP and Hockney for communication. Machine descriptions are YAML files. Network descriptions are numeric `.cfg` tables (latency, inverse bandwidth, eager limit, and LogGP parameters). The simulation does **not** run the full application on the target hardware.⭐

The same DSEL covers MPI collective modeling, multi-kernel workflows, and optional heterogeneous clusters. Those placements are documented in [Modes](README_COLLECTIVE.md).🌟

The application blueprint uses concepts from modern C++ so domain experts can encode inter-process dependencies without taking on unknown effects from a particular machine.💥


<a name="workflow"></a>
⚙️ **Framework workflow**💬

```diff
- Application model

    * Domain knowledge expressed in the DisCostiC DSEL: call dependencies, computation (data access, flop count), and communication (volume, mode, and protocol), including point-to-point, collectives, multi-kernel phases, and heterogeneous placements

- Analytic first-principle performance models

    * Computation models: Roofline and refined ECM models, including the memory-bandwidth bottleneck at chip level
    * Communication models: LogGP and Hockney (latency-bandwidth) for intra-chip, inter-chip, and inter-node scopes

- System models

    * YAML machine files and numeric network `.cfg` tables for the full topology: cores, chips, nodes, network interfaces, and clusters

+ DisCostiC combines all of them to simulate the runtime cost of distributed applications without executing the full application on the target hardware.
```

<a name="advantages"></a>
🖍 **Advantages over existing tools**💬

```diff
- DSEL approach for creating an application blueprint

    * Inter-process dependencies, collectives, multi-kernel phases, and heterogeneous clusters, without introducing unknown consequences from the actual systems

- A full-scale first-principle-model-based simulator

    * Roofline/ECM and LogGP/Hockney across cores, chips, nodes, and networks

- Efficient speed

    * No intermediate application traces, unlike offline trace-driven tools
    * No requirement to execute the application on the host architecture

+ An open-source, low-entry simulator for model-based design-space exploration
```
