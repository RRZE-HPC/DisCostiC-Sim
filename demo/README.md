# DisCostiC Demo

Validation suite for **DisCostiC** (Distributed Cost in Cluster).

Each folder is self-contained: benchmark, DisCostiC configs, SLURM jobs, results, and figures. Demos run on Intel Ice Lake-SP (ICX) and, where noted, Sapphire Rapids (SPR) at NHR@FAU (Fritz).

## Demos

| Demo | Kernel | Stages | README |
|------|--------|--------|--------|
| [`1_triad/`](1_triad/) | STREAM Triad (`STREAM_FILE`) | 1–5 · ICX + SPR · O3 + O1 (stages 2–3) | [`1_triad/README.md`](1_triad/README.md) |
| [`2_heat/`](2_heat/) | Jacobi 2D (`HEAT_FILE`) | 1–5 · ICX · O3 + penalty | [`2_heat/README.md`](2_heat/README.md) |
| [`3_pisolver-stream/`](3_pisolver-stream/) | Pi + STREAM (`PITRIADLBL`) | 1–5 · ICX · O3 | [`3_pisolver-stream/README.md`](3_pisolver-stream/README.md) |
| [`4_raytracing-opticalflow/`](4_raytracing-opticalflow/) | Ray tracing + optical flow (`RTOF4`) | 1–5 · ICX · O3 | [`4_raytracing-opticalflow/README.md`](4_raytracing-opticalflow/README.md) |
| [`5_collectives/`](5_collectives/) | MPI collectives vs IMB | 1–2 · ICX | [`5_collectives/README.md`](5_collectives/README.md) |

This `demo/` folder lives inside the [DisCostiC-Sim](https://github.com/RRZE-HPC/DisCostiC-Sim) repository. Build DisCostiC from the repository root first (see the [main README](../README.md#compilation-and-build)). Scripts resolve paths from their own location and treat two levels above each demo folder (the DisCostiC-Sim root) as the repository root, so no extra setup is needed in a normal checkout. If the demo tree is moved elsewhere, point the scripts at the DisCostiC-Sim checkout:

```bash
export DISCOSTIC_ROOT=/absolute/path/to/DisCostiC-Sim   # the old name DISCOSTIC_EXTENSION_ROOT still works
```

Submit `sbatch` jobs from inside the relevant demo directory so relative `--output` and `--error` paths resolve.

```
DisCostiC-Sim/
└── demo/
    ├── README.md
    ├── metrics.py                         # shared wall, rate, and collective formulas
    ├── 1_triad/
    ├── 2_heat/
    ├── 3_pisolver-stream/
    ├── 4_raytracing-opticalflow/
    └── 5_collectives/
```

## Run

```bash
cd demo/<N_kernel>/                     # e.g. cd demo/2_heat/
bash setup_output_dirs.sh
module load intel/2023.2.1 intelmpi/2021.16.0 python git cmake likwid
# fallback: intel/2025.2.0 intelmpi/2021.16.0

python3 run_job_chain.py --dry-run --stages 1,2,3,4,5
python3 run_job_chain.py --stages 1,2,3,4,5
python3 generate_figures.py --stages 1,2,3,4,5   # 1_triad: shared/scripts/generate_figures.py
```

The chain skips steps whose artifacts already verify. Use `--force` to re-run them. `5_collectives` uses `--stages 1,2` and generates figures for stage 1 only.

Regenerate figures from existing artifacts (run from `demo/`):

```bash
python3 1_triad/shared/scripts/generate_figures.py --stages 1,2,3,4,5
python3 2_heat/generate_figures.py --stages 1,2,3,4,5
python3 3_pisolver-stream/generate_figures.py --stages 1,2,3,4,5
python3 4_raytracing-opticalflow/generate_figures.py --stages 1,2,3,4,5
python3 5_collectives/generate_figures.py --stages 1
```

## How results are scored

HEAT, STREAM, Pi, and ray tracing + optical flow score **wall time**. The simulated value is the max-rank `runtime [s]` in the pass-2 block `FULL APPLICATION PERFORMANCE (for all ranks):`. The measured value is `wall_seconds` in the stage TSV. Duplicate `config` rows: the last row wins.

```
wall_err = (sim − meas) / meas
```

The collectives demo scores **min and max collective duration** against IMB, not application wall.

Derived rates (GFlop/s, GB/s, Mpix/s, efficiency) are computed in [`metrics.py`](metrics.py) from that same wall. They are not hardware-counter measurements. Because a rate is proportional to `1/wall`:

```
rate_err = −wall_err / (1 + wall_err)
```

Kernel-split timers (Pi `TriadTime` / `PiTime`, ray tracing `Timer ray-tracing` / `Timer optical-flow`) are extra measured-only CSV columns and are not scored.

| Kind | Path |
|------|------|
| Measured TSV | `stageN-*/results/measured/stageN-measured*.tsv` |
| Simulated wall | `stageN-*/results/simulated/<config>/slurm-discostic-pass2.out` |
| Figures and CSV | `stageN-*/figures/` (HEAT also `figures-penalty/`; STREAM stages 2–3 also `figures-o1/`) |
| Stage 4 trace | sim `stage4-idlewave/results/simulated/…/*.single.stf`; measured `…/measured-trace/…` |

## Adding a kernel demo

1. Create `demo/<N_kernel>/` next to the existing demos, using the next free number.
2. Keep scripts, configs, and benchmarks inside that demo.
3. Add `<N_kernel>/README.md` and link it from the table above.
