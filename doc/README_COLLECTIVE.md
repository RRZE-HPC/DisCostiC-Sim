Part of the [main README](../README.md#further-reading).

<a name="modes"></a>
## 🧭 Modes

DisCostiC selects behavior with several independent modes. Collective mode is one of them. Kernel modes choose how a compute kernel is timed. Multi-kernel mode chains two compute phases. Heterogeneous mode places ranks on two system descriptions. `exchange_mode` (message size, step, direction, periodicity) stays in the [configuration tables](../README.md#configuration) of the main README.

<a name="kernel-modes"></a>
📝 **Kernel modes**💬

`kernel_mode` is `COMP`, `LBL`, `FILE`, or `SRC`. The summary table is in the [main README](../README.md#configuration). `FILE` and `SRC` call [Kerncraft](https://github.com/RRZE-HPC/kerncraft). `COMP` and `LBL` do not. The `Exec` string carries the mode:

```cpp
// LBL — pre-recorded ECM from nodelevel/configs
DisCosTiC->Exec("LBL:STREAM_TRIAD", deps);

// COMP — ECM layer times in nanoseconds
DisCosTiC->Exec("COMP:TOL=2.0||TnOL=1.0|TL1L2=3.0|TL2L3=6.0|TL3Mem=14.2", deps);

// FILE — kernel source from nodelevel/kernels
DisCosTiC->Exec("FILE:stream-triad.c//BREAK:STREAM//./nodelevel/machine-files/IcelakeSP_Platinum-8360Y.yml//18//-D N 1000000", deps);

// SRC — C source inline, then //BREAK:
DisCosTiC->Exec("SRC:...source...//BREAK:STREAM//./nodelevel/machine-files/IcelakeSP_Platinum-8360Y.yml//18//-D N 1000000", deps);
```

Headers follow `<BENCHMARK>_<MODE>.hpp` under [test](../test), for example `STREAM_FILE`, `STREAM_SRC`, `STREAM_LBL`, and `STREAM_COMP`. Set `benchmark_kernel` in [config.cfg](../config.cfg) to that header name without `.hpp`.

<a name="collective-mode"></a>
📡 **Collective mode**💬

Set `collective_mode = 1` in [config.cfg](../config.cfg). The simulator then runs **one timestep**. `heteregeneous` must be `0`. Combining `collective_mode = 1` with `heteregeneous = 1` is rejected at startup.

[test/STREAM_COLLECTIVE.hpp](../test/STREAM_COLLECTIVE.hpp) runs a STREAM `Exec` and then `DisCosTiC->Collective(...)`. The header reads three keys from `config.cfg`. The shipped [config.cfg](../config.cfg) does not define them, so a `STREAM_COLLECTIVE` run needs these lines added:

| Key | Role |
| --------------------- | ------------- |
| `collective_type` | `DisCosTiC::CollectiveType`: `COLL_BCAST` = 0, `COLL_REDUCE` = 1, `COLL_ALLREDUCE` = 2 |
| `collective_algorithm` | Algorithm id for that type |
| `collective_bytes` | Collective payload in bytes |

| Type | Algorithm enum | Value |
| --------------------- | ------------- | ------------- |
| `COLL_BCAST` | `BCAST_BINOMIAL` | 0 |
| `COLL_BCAST` | `BCAST_RING` | 1 |
| `COLL_REDUCE` | `REDUCE_BINOMIAL` | 0 |
| `COLL_ALLREDUCE` | `ALLREDUCE_RECURSIVE_DOUBLING` | 0 |

[include/CollectiveModel.hpp](../include/CollectiveModel.hpp) builds the broadcast, reduce, and allreduce schedules from LogGP or Hockney parameters for intra-chip, inter-chip, and inter-node scopes. The call shape used by `STREAM_COLLECTIVE` is:

```cpp
collective = DisCosTiC->Collective(
    coll_type,
    root, coll_bytes,
    rank, systemsize, comp,
    static_cast<uint8_t>(coll_algo));
```

<a name="multi-kernel-mode"></a>
🧩 **Multi-kernel mode**💬

Two compute phases in one timestep use `part1` and `part2` in [config.cfg](../config.cfg). Each phase has its own `Exec` mode (`FILE`, `LBL`, `SRC`, or `COMP`). The shipped config sets `part1 = STREAM` and `part2 = TRIAD`.

| Header | What it chains |
| --------------------- | ------------- |
| `TWOKERNEL` | Two compute kernels selected by `part1` / `part2` |
| `PITRIADFILE` | Pi and STREAM/TRIAD in `FILE` mode |
| `PITRIADLBL` | Pi and STREAM/TRIAD in `LBL` mode |
| `RTOF4` | Ray tracing and optical flow in `LBL` mode |

Kerncraft and the per-rank ECM config copy run once per kernel, not once per modeled rank. See [include/AST.hpp](../include/AST.hpp).

<a name="heterogeneous-mode"></a>
🖥️ **Heterogeneous mode**💬

`heteregeneous` is spelled that way in [config.cfg](../config.cfg) and in [src/DisCosTiC.cpp](../src/DisCosTiC.cpp).

| `heteregeneous` | System |
| --------------------- | ------------- |
| `0` | One system: `number_of_processes` ranks on `micro_architecture` |
| `1` | Two systems: primary ranks plus `secondary_number_of_processes` ranks on `secondary_micro_architecture` |

Secondary keys in `config.cfg` include `secondary_task_per_node`, `secondary_compiler_flags`, `secondary_pmodel`, `secondary_N`, `secondary_dim_x`, `secondary_dim_y`, and `secondary_penalty`. This mode is independent of collective mode and cannot be combined with `collective_mode = 1`.
