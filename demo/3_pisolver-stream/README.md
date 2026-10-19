# Pi solver + STREAM demo

Validates DisCostiC against a measured two-kernel MPI run on Fritz Ice Lake-SP, `-O3` / AVX-512: a Pi solver (`DIVIDE`), then STREAM Triad, then a one-way ring (tag 1, 200000 doubles). The DisCostiC kernel is `test/PITRIADLBL.hpp`.

Suite overview: [`../README.md`](../README.md).

**Measured bench:** `shared/pisolver-triad-bench/` (`bin/plain-mpi`). Job configs for the benchmark are under `shared/pisolver-triad-bench/configs/`.
**Simulation:** hand ECM in `nodelevel/configs/DIVIDE.cfg` and `STREAM.cfg`. Do not edit those files. Demo DisCostiC configs are under each stage’s `configs/`.

Defaults: `Nsteps = 3600000000`, `N = 1360000000`, scalar `0.41`, `penalty = 0`, `part1 = DIVIDE`, `part2 = STREAM`. Weak stage 5 scales `Nsteps` and `N` with the node count.

## Stages

| Stage | What it checks | Platforms |
|------:|----------------|-----------|
| 1 | ECM model and measured wall | ICX socket · ICX tpn=1 |
| 2 | One timestep (100 iterations) | ICX socket · ICX tpn=1 |
| 3 | Cost versus iteration count | 100 / 1 000 / 10 000 iterations, socket and tpn=1 |
| 4 | Idle-wave delay | ICX, 1 node × 72 ranks (trace in Intel Trace Analyzer) |
| 5 | Strong and weak scaling | 1 / 2 / 4 / 8 / 16 / 32 nodes |

An optional stage-2 full-node test (1 node × 72 ranks) is kept under `jobs/measured/icx-onenode.sh` and `jobs/simulated/icx-onenode-100.sh`. It is not in `run_job_chain.py`. `generate_figures.py` adds `icx-onenode-100` to the stage-2 plot only when that TSV row and pass-2 file already exist.

## Layout

```
3_pisolver-stream/
├── shared/
│   ├── pisolver-triad-bench/    # measured MPI binary and configs
│   └── scripts/                 # build, measured helpers, DisCostiC, traces
├── stage1-ecm-modeling/
├── stage2-single-timestep/
├── stage3-multi-timestep/
├── stage4-idlewave/
├── stage5-scaling/
├── setup_output_dirs.sh
├── run_job_chain.py
└── generate_figures.py
```

## Simulator requirements

Two behaviors in the DisCostiC tree are required for this kernel to finish:

1. **10 000 timesteps.** `PITRIADLBL` issues two compute operations per timestep. On `SEND`, the simulator must set `computingProcessesCount` back to the remaining compute-rank count, and the per-rank buffer must be at least `max(2000, COMPCount + 1)`. Otherwise the 10 000-step jobs stop after about 2000 steps (`src/DisCosTiC.cpp`).
2. **tpn=1.** The ring sends in one direction. A receive with `task_per_node = 1` needs the reverse internode edge (`include/AST.hpp`, `src/DisCosTiC.cpp`). Rebuild with `make -C build discostic` after either change.

## Run

```bash
cd demo/3_pisolver-stream/
bash setup_output_dirs.sh
module load intel/2023.2.1 intelmpi/2021.16.0 python git cmake likwid
bash shared/scripts/build_benchmark.sh

python3 run_job_chain.py --dry-run --stages 1,2,3,4,5
python3 run_job_chain.py --stages 1,2,3,4,5
python3 generate_figures.py --stages 1,2,3,4,5
```

Submit SLURM jobs from inside `demo/3_pisolver-stream/`. The chain skips steps whose artifacts already verify. Use `--force` to re-run them.

Optional full-node test, not part of the chain:

```bash
# sbatch stage2-single-timestep/jobs/measured/icx-onenode.sh
# sbatch stage2-single-timestep/jobs/simulated/icx-onenode-100.sh
```

Stage 4 injects a one-rank delay. The scoring measurement is not traced, so Intel Trace Collector does not change `wall_seconds`. Simulations in stages 2–5 convert `{chromevizfilename}.single.stf` inside the job and fail the job if that conversion fails. The measured trace is a separate stage-4 job.

```bash
module load intel-itac/2022.4.0
traceanalyzer stage4-idlewave/results/simulated/icx/pitriadlbl-idlewave.single.stf
traceanalyzer stage4-idlewave/results/measured-trace/icx/pitriadlbl-idlewave-measured.single.stf
```

## How results are read

Scoring uses wall time. Simulated wall is the max-rank `runtime [s]` under `FULL APPLICATION PERFORMANCE (for all ranks):` in `slurm-discostic-pass2.out`. Measured wall is `wall_seconds` in the stage TSV. If a config appears more than once, the last row wins.

```
wall_err = (sim − meas) / meas
```

Rates in [`../metrics.py`](../metrics.py) use that same wall. GFlop/s adds both kernels' flop counts. GB/s is the STREAM traffic of the whole run.

| Metric | Formula |
|--------|---------|
| GFlop/s | `(2 × N + Nsteps) × ntimes / wall / 1e9` |
| GB/s | `4 × 8 × N × ntimes / wall / 1e9` (two loads, one store, one write-allocate) |
| Strong efficiency | `T(1) / (n · T(n))` from the combined wall |
| Weak efficiency | `T(1) / T(n)` from the combined wall |
| Rate error | `(sim − meas) / meas`. GFlop/s and GB/s share it, because both use the combined wall |

Recorded wall error on ICX, `-O3` (optional onenode included):

| Stage | Wall error |
|------:|------------|
| 2 | tpn=1 about +2.8%; socket about +1.9%; optional onenode about −0.6% |
| 3 | tpn=1 about +2.7% to +3.2%; socket about +1.7% to +1.9% |
| 4 | about −5.5% |
| 5 | 1–8 nodes about ±4%; strong 16/32 about −29% / −76%; weak 16/32 about −1.2% |

## Figures

`generate_figures.py` reads the files below. Each metric is its own PNG. Stage 2 draws one panel per platform. Stage 3 draws wall or rate lines with `|% error|` on a second axis. Stage 5 annotates that error on the lines; the y-axis starts at 0 except for efficiency, and node ticks are powers of two. Stage 4 has no Gantt chart.

Stage 1 ECM terms are read from `nodelevel/configs/DIVIDE.cfg` and `nodelevel/configs/STREAM.cfg`. If either file is missing, the script uses `stage2-single-timestep/results/simulated/icx-socket-100/DIVIDE_ecm.cfg` or `STREAM_ecm.cfg`. With the current tree those copies match the `nodelevel` files. `T_ECM = max(T_comp, data path)`.

| Output | Measured | Simulated |
|--------|----------|-----------|
| `stage1-ecm-modeling/figures/stage1-ecm-breakdown.png`, `stage1-measured-*.png`, `stage1_ecm_data.csv` | `results/measured/stage1-measured.tsv` | `nodelevel/configs/DIVIDE.cfg` and `STREAM.cfg` |
| `stage2-single-timestep/figures/stage2-{wall,gflops,gb}.png`, `stage2_data.csv` | `results/measured/stage2-measured-100.tsv` | pass-2 `runtime [s]` |
| `stage3-multi-timestep/figures/stage3-{wall,gflops,gb}.png`, `stage3_data.csv` | `results/measured/stage3-measured.tsv` | pass-2 `runtime [s]` |
| `stage4-idlewave/figures/stage4-{wall,gflops,gb}.png`, `stage4_metrics.csv` | last `icx` row of `results/measured/stage4-measured.tsv` | pass-2 `runtime [s]` |
| Stage 4 traces | `results/measured-trace/icx/pitriadlbl-idlewave-measured.single.stf` | `results/simulated/icx/pitriadlbl-idlewave.single.stf` |
| `stage5-scaling/figures/stage5-{wall,gflops,gb,efficiency}.png`, `stage5-{strong,weak}-{wall,gflops,gb,efficiency}.png`, `stage5_scaling_wall.csv` | `results/measured/stage5-measured.tsv` | pass-2 `runtime [s]` |

Stage 5 also writes one PNG per series (`stage5-{strong,weak}-<metric>.png`). CSV columns include `wall_err_pct` and `rate_err_pct`. GFlop/s and GB/s share `rate_err_pct`. Paths above are relative to the stage directory unless they start with `nodelevel/`.
