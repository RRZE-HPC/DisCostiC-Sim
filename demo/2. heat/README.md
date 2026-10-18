# HEAT Jacobi 2D demo

Validates DisCostiC against a measured MPI Jacobi 2D stencil (vertical 1D decomposition, top and bottom halo exchange) on Fritz Ice Lake-SP, `-O3` / AVX-512, MPI only.

Suite overview: [`../README.md`](../README.md).

**Kernel:** `test/HEAT_FILE.hpp` (Kerncraft kernel `nodelevel/kernels/heat.c`).
**Measured bench:** `shared/benchmarks/heat.c` — `heat <global_dim_y> <global_dim_x> <iterations>`.

## Stages

| Stage | What it checks | Platforms |
|------:|----------------|-----------|
| 1 | ECM model and measured wall | ICX socket · ICX tpn=1 |
| 2 | One timestep (100 iterations) | ICX socket · ICX tpn=1 |
| 3 | Cost versus iteration count | 100 / 1 000 / 10 000 iterations, socket and tpn=1 |
| 4 | Idle-wave delay | ICX, 1 node × 72 ranks (trace in Intel Trace Analyzer) |
| 5 | Strong and weak scaling | 1 / 2 / 4 / 8 / 16 / 32 nodes × 72 ranks |

An optional stage-2 full-node test (1 node × 72 ranks) is kept under `jobs/measured/icx-onenode.sh` and `jobs/simulated/icx-onenode-100.sh`. It is not in `run_job_chain.py` and is not plotted.

### Grid

| Quantity | Value | Role |
|----------|-------|------|
| `dim_x` | 16384 | One halo row is 128 KiB |
| `dim_y` | 92160 | Divisible by 36 and 72 |
| Two grids | about 24.2 GB | `2 * dim_x * dim_y * 8` bytes |
| Stage 5 strong | same `(dim_x, dim_y)` | More ranks, fewer local rows |
| Stage 5 weak | `dim_y` grows with nodes | Local rows stay constant; `dim_x` stays fixed |

DisCostiC sets `number_of_processes = ntasks - 1` (socket 35, full node 71, and 575 / 1151 / 2303 at 8 / 16 / 32 nodes).

## Layout

```
2. heat/
├── shared/benchmarks/heat.c
├── shared/scripts/                  # build, measured helpers, DisCostiC, traces
├── stageN-*/configs/                # O3, penalty = 0
├── stageN-*/configs-penalty/        # cases with |wall error| > 5%; simulation only
├── stageN-*/jobs/measured/
├── stageN-*/jobs/measured-trace/    # stage 4 trace only; does not write the TSV
├── stageN-*/jobs/simulated/
├── stageN-*/jobs/simulated-penalty/
├── stageN-*/results/measured/       # shared by base and penalty figures
├── stageN-*/results/measured-trace/
├── stageN-*/results/simulated/
├── stageN-*/results/simulated-penalty/
├── stageN-*/figures/                # measured vs base simulation
├── stageN-*/figures-penalty/        # measured vs base vs penalty, where calibrated
├── calibrate_penalties.py
├── generate_figures.py
├── setup_output_dirs.sh
└── run_job_chain.py
```

Penalty is simulator-only: `HEAT_FILE.hpp` adds `penalty` nanoseconds to each timestep. Measured jobs are not duplicated. The stage-4 trace job writes `.stf` only and is not the source of `wall_seconds`.

## Run

```bash
cd "demo/2. heat/"
bash setup_output_dirs.sh
module load intel/2023.2.1 intelmpi/2021.16.0 python git cmake likwid
# fallback: intel/2025.2.0 intelmpi/2021.16.0

python3 run_job_chain.py --dry-run --stages 1,2,3,4,5
python3 run_job_chain.py --stages 1,2,3,4,5
python3 generate_figures.py --stages 1,2,3,4,5
```

Submit SLURM jobs from inside `demo/2. heat/`. The chain runs the base campaign (`penalty = 0`), then `calibrate_penalties.py`, then penalty simulations for pairs listed in `penalty_active.tsv`. Use `--force` to re-run a step that already verifies.

Job time limits use a 10-minute floor. When a previous run of the same job took more than 5 minutes, `--time` is twice that elapsed time.

## How results are read

Scoring uses wall time. Simulated wall is the max-rank `runtime [s]` under `FULL APPLICATION PERFORMANCE (for all ranks):` in `slurm-discostic-pass2.out`. Measured wall is `wall_seconds` in the stage TSV. If a config appears more than once, the last row wins.

```
wall_err = (sim − meas) / meas
```

Rates in [`../metrics.py`](../metrics.py) use that same wall (not LIKWID counters):

| Metric | Formula |
|--------|---------|
| MLUP/s | `dim_x × dim_y × ntimes / wall / 1e6` |
| GF/s | `5 × updates / wall / 1e9` |
| GB/s | `32 × updates / wall / 1e9` |
| Strong efficiency | `T(1) / (n · T(n))` |
| Weak efficiency | `T(1) / T(n)` |
| Rate error | `−wall_err / (1 + wall_err)` |

### Penalty

[`calibrate_penalties.py`](calibrate_penalties.py) writes [`penalty_active.tsv`](penalty_active.tsv) and the matching `configs-penalty/*.cfg` files:

```
if |(sim − meas) / meas| > 0.05:
    penalty_ns = (meas − sim) / ntimes × 1e9
```

A negative penalty means the base simulation was slower than the measurement. Chain steps whose `(stage, config)` pair is absent from `penalty_active.tsv` are skipped. Socket stages 2 and 3 stay inside 5% and have no penalty jobs. The active pairs, base error, and penalty are listed in that TSV. The penalty re-simulation is what `figures-penalty/` plots. A uniform per-timestep add matches wall time; it does not by itself explain a change in parallel efficiency.

## Figures

`generate_figures.py` reads the files below. Each metric is its own PNG.

Stage 1 ECM terms come from `stage2-single-timestep/results/simulated/icx-socket-100/HEAT_FILE_ecm.cfg`, then the stage-3 copy of that file, then `nodelevel/configs/HEAT_FILE.cfg`. A built-in fallback is used only if none of those files exist. `T_ECM = max(T_comp, data path)`.

`figures/` is measured versus the base simulation. `figures-penalty/` adds the penalty simulation on calibrated cases and omits it otherwise. Stage 2 draws one panel per platform (simulation, measurement, and an error box). Stage 3 draws wall or rate lines with `|% error|` on a second axis. Stage 5 annotates that error on the lines. Y-axes start at 0 except efficiency. Stage 4 has no Gantt chart; open the STF files in Intel Trace Analyzer.

| Output | Measured | Simulated |
|--------|----------|-----------|
| `stage1-ecm-modeling/figures/stage1-ecm-breakdown.png`, `stage1-measured-wall.png`, `stage1_ecm_data.csv` | `results/measured/stage1-measured.tsv` | ECM file order above |
| `stage2-single-timestep/figures/stage2-{wall,mlup,gf,gb}.png`, `stage2_data.csv` | `results/measured/stage2-measured-100.tsv` | `results/simulated/<cfg>/slurm-discostic-pass2.out` |
| `stage2-single-timestep/figures-penalty/stage2-{wall,mlup,gf,gb}.png`, `stage2_data.csv` | same TSV | base pass-2, plus `results/simulated-penalty/<cfg>/` when present |
| `stage3-multi-timestep/figures/stage3-{wall,mlup,gf,gb}.png`, `stage3_data.csv` | `results/measured/stage3-measured.tsv` | `results/simulated/<cfg>/` |
| `stage3-multi-timestep/figures-penalty/stage3-{wall,mlup,gf,gb}.png`, `stage3_data.csv` | same TSV | base pass-2 plus `results/simulated-penalty/` |
| `stage4-idlewave/figures/stage4-{wall,mlup,gf,gb}.png`, `stage4_metrics.csv` | `results/measured/stage4-measured.tsv` (last `icx` row) | `results/simulated/icx/` |
| `stage4-idlewave/figures-penalty/stage4-{wall,mlup,gf,gb}.png`, `stage4_metrics.csv` | same TSV | base plus `results/simulated-penalty/icx/` |
| Stage 4 traces | `results/measured-trace/icx/heat-idlewave-measured.single.stf` | `results/simulated/icx/heat-idlewave.single.stf` and `results/simulated-penalty/icx/heat-idlewave-penalty.single.stf` |
| `stage5-scaling/figures/stage5-{wall,mlup,gf,gb,efficiency}.png`, `stage5-{strong,weak}-{wall,mlup,gf,gb,efficiency}.png`, `stage5_scaling_wall.csv` | `results/measured/stage5-measured.tsv` | `results/simulated/<name>/` |
| `stage5-scaling/figures-penalty/stage5-{wall,mlup,gf,gb,efficiency}.png`, `stage5-{strong,weak}-{wall,mlup,gf,gb,efficiency}.png`, `stage5_scaling_wall.csv` | same TSV | base plus `results/simulated-penalty/` |

Stage 5 also writes one PNG per series (`stage5-{strong,weak}-<metric>.png`), in `figures/` and in `figures-penalty/`. CSV columns include `wall_err_pct` and `rate_err_pct`. Paths above are relative to the stage directory.
