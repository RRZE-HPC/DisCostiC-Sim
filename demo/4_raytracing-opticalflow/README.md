# Ray tracing + optical flow demo

Validates DisCostiC against a measured MPI ray-tracing and optical-flow (multigrid) solver on Fritz Ice Lake-SP, `-O3` / AVX-512. Simulations use `RTOF4` with two ECM parts, `part1 = RT` and `part2 = OF`.

Suite overview: [`../README.md`](../README.md).

**Measured bench:** `shared/rt-of-bench/` (`bin/ray-tracing-optical-flow-base`).
**Simulation ECM:** `nodelevel/configs/RT.cfg` and `nodelevel/configs/OF.cfg`.

| Item | Value |
|------|--------|
| Application | Ray tracing (compute-bound) and optical flow / multigrid (memory-bound) |
| DisCostiC kernel | `RTOF4` |
| Default workload | MG 9, supersampling 2, `sinPrefactor = 0`, `dt = 1e-5`, `mgIterations = 10`, inner `numTimeSteps = 4` |

## Stages

| Stage | What it checks | Platforms |
|------:|----------------|-----------|
| 1 | ECM model and measured wall | ICX socket · ICX tpn=1 |
| 2 | One outer timestep (100 repetitions) | ICX socket · ICX tpn=1 |
| 3 | Cost versus repetition count | 100 / 1 000 / 10 000, socket and tpn=1 |
| 4 | Idle-wave delay | ICX, 1 node × 72 ranks (trace in Intel Trace Analyzer) |
| 5 | Strong and weak scaling | 1 / 2 / 4 / 8 nodes |

## Layout

```
4_raytracing-opticalflow/
├── shared/
│   ├── rt-of-bench/             # measured solver and Makefile
│   └── scripts/                 # build, measured helpers, DisCostiC, traces
├── stage1-ecm-modeling/
├── stage2-single-timestep/
├── stage3-multi-timestep/
├── stage4-idlewave/
├── stage5-scaling/
├── generate_figures.py
├── setup_output_dirs.sh
└── run_job_chain.py
```

## Run

```bash
cd demo/4_raytracing-opticalflow/
bash setup_output_dirs.sh
module load intel/2023.2.1 intelmpi/2021.16.0 python git cmake likwid
bash shared/scripts/build_benchmark.sh

python3 run_job_chain.py --dry-run --stages 1,2,3,4,5
python3 run_job_chain.py --stages 1,2,3,4,5
python3 generate_figures.py --stages 1,2,3,4,5
```

Submit SLURM jobs from inside `demo/4_raytracing-opticalflow/`. The chain skips steps whose artifacts already verify. Use `--force` to re-run them.

### Measured command line

```text
ray-tracing-optical-flow-base \
  {NUM_MG_LEVELS} {SUPERSAMPLING} {NUM_MG} {TIMESTEP} \
  {NUM_TIMESTEP} {REPETITIONS} {PRINT_IMG} {OFFSET} {SIN_PREFACTOR}
```

Demo jobs use `9 2 10 0.00001 4 <REPS> 0 0.625 0.0`. Stage 5 uses 100 outer repetitions. Strong scaling keeps MG = 9. Weak scaling raises MG with node count (9, then 10, 11, 12) and stops at 8 nodes.

`shared/scripts/measured_common.sh` sets `I_MPI_ASYNC_PROGRESS=1` only for single-node jobs with at most 36 ranks per node. One-rank-per-node jobs and 72-rank jobs leave it unset.

### Simulation

- RT is compute-bound (`T_ECM` about 161.21 cycles per cache line). OF is memory-bound (`T_ECM = 6942`, `T_36ECM = 773.13`). `Scaling_cores = inf` turns off the multi-core bandwidth shrink for this full-grid-per-rank pattern.
- `N` is the optical-flow loop (`nxOF × nyOF`). `Nsteps` is the ray-tracing loop (`nxRT × nyRT`). Weak simulations use the same MG grids as the measurement; they do not scale the grid linearly with nodes.
- `RTOF4` scales only the ray-tracing factor with socket occupancy. Isend and Irecv both use `Nsteps × 8` bytes.
- Simulations in stages 2–5 require `{chromevizfilename}.single.stf` (`otf2-to-stf` inside the job; the job fails if conversion fails). The measured stage-4 trace is a separate job (`jobs/measured-trace/icx.sh`) and does not write the scoring TSV.

```bash
module load intel-itac/2022.4.0
traceanalyzer stage4-idlewave/results/simulated/icx/rtof-idlewave.single.stf
traceanalyzer stage4-idlewave/results/measured-trace/icx/rtof-idlewave-measured.single.stf
```

## How results are read

Scoring uses wall time. Simulated wall is the max-rank `runtime [s]` in pass-2. Measured wall is `wall_seconds` in the stage TSV. If a config appears more than once, the last row wins.

```
wall_err = (sim − meas) / meas
```

Rates in [`../metrics.py`](../metrics.py) use that same wall. Mpix/s adds both kernels' pixels.

| Metric | Formula |
|--------|---------|
| Mpix/s | `(N + Nsteps) × nreps / wall / 1e6`, with `nxOF = 2^(MG−1) + 2` and `nxRT = nxOF × supersampling` |
| Strong efficiency | `T(1) / (n · T(n))` from the combined wall |
| Weak efficiency | `T(1) / T(n)` from the combined wall |
| Rate error | `(sim − meas) / meas` on the combined wall |

Recorded wall error on ICX, `-O3`:

| Config | Wall error |
|--------|------------|
| Stage 2 tpn=1 / socket | about +3.8% / −10.1% |
| Stage 3 tpn=1, 100–10 000 | about +3.9% to −0.9% |
| Stage 3 socket, 100–10 000 | about −5.6% to −13.1% |
| Stage 4 | about −11.4% |
| Stage 5 strong, 1–8 nodes | about −9.5% to +1.0% |
| Stage 5 weak, 2–8 nodes | about −25% to −37% |

## Figures

`generate_figures.py` reads the files below. Each metric is its own PNG. Stage 2 draws one panel per platform. Stage 3 draws wall or rate lines with `|% error|` on a second axis. Stage 5 annotates that error on the lines; the y-axis starts at 0 except for efficiency, and node ticks are powers of two. Stage 4 has no Gantt chart.

Stage 1 ECM terms are read from `nodelevel/configs/RT.cfg` and `nodelevel/configs/OF.cfg`. If either file is missing, the script uses `stage2-single-timestep/results/simulated/icx-socket-100/RT_ecm.cfg` or `OF_ecm.cfg`. With the current tree those copies match the `nodelevel` files. `T_ECM = max(T_comp, data path)`.

| Output | Measured | Simulated |
|--------|----------|-----------|
| `stage1-ecm-modeling/figures/stage1-ecm-breakdown.png`, `stage1-measured-*.png`, `stage1_ecm_data.csv` | `results/measured/stage1-measured.tsv` | `nodelevel/configs/RT.cfg` and `OF.cfg` |
| `stage2-single-timestep/figures/stage2-{wall,mpix}.png`, `stage2_data.csv` | `results/measured/stage2-measured-100.tsv` | pass-2 `runtime [s]` |
| `stage3-multi-timestep/figures/stage3-{wall,mpix}.png`, `stage3_data.csv` | `results/measured/stage3-measured.tsv` | pass-2 `runtime [s]` |
| `stage4-idlewave/figures/stage4-{wall,mpix}.png`, `stage4_metrics.csv` | last `icx` row of `results/measured/stage4-measured.tsv` | pass-2 `runtime [s]` |
| Stage 4 traces | `results/measured-trace/icx/rtof-idlewave-measured.single.stf` | `results/simulated/icx/rtof-idlewave.single.stf` |
| `stage5-scaling/figures/stage5-{wall,mpix,efficiency}.png`, `stage5-{strong,weak}-{wall,mpix,efficiency}.png`, `stage5_scaling_wall.csv` | `results/measured/stage5-measured.tsv` | pass-2 `runtime [s]` |

Stage 5 also writes one PNG per series (`stage5-{strong,weak}-<metric>.png`). CSV columns include `wall_err_pct` and `rate_err_pct`. Paths above are relative to the stage directory unless they start with `nodelevel/`.
