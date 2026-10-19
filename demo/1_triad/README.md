# STREAM Triad demo

Validates DisCostiC against a measured MPI STREAM Triad with a closed-ring exchange on Fritz Ice Lake-SP and Sapphire Rapids, `-O3` / AVX-512, MPI only. Stages 2 and 3 also compare `-O1`.

Suite overview: [`../README.md`](../README.md).

**Kernel:** `test/STREAM_FILE.hpp`.
**Measured bench:** `shared/benchmarks/mpi_stream_bench_comm.c` — `mpi_stream_bench_comm <N_total> <ntimes>`.

## Stages

| Stage | What it checks | Platforms |
|------:|----------------|-----------|
| 1 | ECM model and measured wall | ICX socket · ICX tpn=1 · SPR socket |
| 2 | One timestep (100 iterations) | ICX socket · ICX tpn=1 · SPR socket |
| 3 | Cost versus iteration count | 100 / 1 000 / 10 000 iterations, same platforms |
| 4 | Idle-wave delay | ICX 1 node × 72 ranks · SPR 1 node × 104 ranks |
| 5 | Strong and weak scaling | ICX 1–32 nodes · SPR 1–8 nodes (`spr1tb` allows at most 8) |

An optional stage-2 full-node test (ICX, 1 node × 72 ranks) is kept under `jobs/measured/icx-onenode.sh` and `jobs/simulated/icx-onenode-100.sh`. It is not in `run_job_chain.py` and is not plotted.

Build flags for the measured bench:

```
-O3 -fno-alias -xCORE-AVX512 -qopt-zmm-usage=high
```

The `-O1` binaries use the same flags with `-O1`.

## Layout

```
1_triad/
├── shared/benchmarks/mpi_stream_bench_comm.c
├── shared/scripts/                  # build, measured helpers, DisCostiC, traces, figures
├── stageN-*/configs/                # -O3
├── stageN-*/configs-o1/             # stages 2–3
├── stageN-*/jobs/measured/
├── stageN-*/jobs/measured-o1/       # stages 2–3
├── stageN-*/jobs/measured-trace/    # stage 4 trace only; does not write the scoring TSV
├── stageN-*/jobs/simulated/
├── stageN-*/jobs/simulated-o1/
├── stageN-*/results/measured/
├── stageN-*/results/measured-o1/
├── stageN-*/results/measured-trace/
├── stageN-*/results/simulated/
├── stageN-*/results/simulated-o1/
├── stageN-*/figures/                # -O3 measurement vs -O3 simulation
├── stageN-*/figures-o1/             # stages 2–3, including -O1
├── setup_output_dirs.sh
└── run_job_chain.py
```

## Run

```bash
cd demo/1_triad/
bash setup_output_dirs.sh
module load intel/2023.2.1 intelmpi/2021.16.0 python git cmake likwid
# fallback: intel/2025.2.0 intelmpi/2021.16.0

python3 run_job_chain.py --dry-run --stages 1,2,3,4,5
python3 run_job_chain.py --stages 1,2,3,4,5
python3 shared/scripts/generate_figures.py --stages 1,2,3,4,5
```

Submit SLURM jobs from inside `demo/1_triad/`. The chain runs `-O3`, then `-O1` for stages 2–3, then stages 4 and 5. Use `--force` to re-run a step that already verifies.

Job time limits use a 10-minute floor. When a previous run of the same job took more than 5 minutes, `--time` is twice that elapsed time. The stage-4 measured trace job is 20 minutes. Stage 5 trace conversion is capped at 180 seconds so it does not exhaust the scoring job.

## How results are read

Scoring uses wall time. Simulated wall is the max-rank `runtime [s]` under `FULL APPLICATION PERFORMANCE (for all ranks):` in `slurm-discostic-pass2.out`. The figure script uses pass-2 when that file exists. Measured wall is `wall_seconds` in the stage TSV. If a config appears more than once, the last row wins.

```
wall_err = (sim − meas) / meas
```

Rates in [`../metrics.py`](../metrics.py) use that same wall (not LIKWID counters):

| Metric | Formula |
|--------|---------|
| GFlop/s | `2 × N × ntimes / wall / 1e9` |
| GB/s | `4 × 8 × N × ntimes / wall / 1e9` (two loads, one store, one write-allocate) |
| Strong efficiency | `T(1) / (n · T(n))` |
| Weak efficiency | `T(1) / T(n)` |
| Rate error | `−wall_err / (1 + wall_err)` |

`-O1` has its own measured and simulated jobs. `figures-o1/` scores those walls the same way and records `wall_err_o1_pct` and `rate_err_o1_pct` beside the `-O3` columns.

The measured STREAM benchmark does not inject an idle-wave delay. The stage-4 simulation sets `idlewave=1` on rank 4 at timestep 5. The measured stage-4 trace is the undelayed ring. Chain checks for stages 2–4 require the simulation `{stem}.single.stf`. Stage 5 attempts the conversion and may continue if `otf2-to-stf` warns on a large trace.

Stage 5 measured SPR jobs that hold the large triad (1-node strong, and every weak size) request 200 GB.

## Figures

`shared/scripts/generate_figures.py` reads the files below. Each metric is its own PNG. Stage 5 writes separate ICX and SPR figures so the 32-node ICX axis is not mixed with the 8-node SPR axis.

Stage 1 ECM terms come from the live `STREAM_FILE_ecm.cfg` next to the stage-2 simulation:

- `-O3` ICX: `stage2-single-timestep/results/simulated/icx-socket-100/STREAM_FILE_ecm.cfg`
- `-O3` SPR: `stage2-single-timestep/results/simulated/spr-socket-100/STREAM_FILE_ecm.cfg`
- `-O1` ICX and SPR: the same names under `results/simulated-o1/`

Built-in ECM tables are used only if those files are missing. `T_ECM = max(T_comp, data path)`.

| Stage | Layout | Measured TSV | Simulated wall |
|------:|--------|--------------|----------------|
| 1 | ICX tpn=1 is 36 nodes; ICX and SPR socket are 1 node | `stage1-ecm-modeling/results/measured/stage1-measured.tsv` | ECM files above |
| 2 | `icx-tpn1-100`, `icx-socket-100`, `spr-socket-100` | `stage2-single-timestep/results/measured/stage2-measured-100.tsv` and `measured-o1/` | `results/simulated{,-o1}/<cfg>/slurm-discostic-pass2.out` |
| 3 | same layouts × 100 / 1 000 / 10 000 iterations | `stage3-multi-timestep/results/measured/stage3-measured.tsv` and `measured-o1/` | `results/simulated{,-o1}/<platform>-<ntimes>/slurm-discostic-pass2.out` |
| 4 | ICX 1×72 · SPR 1×104 | `stage4-idlewave/results/measured/stage4-measured.tsv` | `results/simulated/{icx,spr}/slurm-discostic-pass2.out` |
| 5 ICX | 1, 2, 4, 8, 16, 32 nodes | `stage5-scaling/results/measured/stage5-measured.tsv` | `results/simulated/{strong,weak}-icx-<N>node/slurm-discostic-pass2.out` |
| 5 SPR | 1, 2, 4, 8 nodes | same TSV (`strong-spr-*`, `weak-spr-*`) | `results/simulated/{strong,weak}-spr-<N>node/slurm-discostic-pass2.out` |

| Output | Measured | Simulated |
|--------|----------|-----------|
| `stage1-ecm-modeling/figures/stage1-ecm-*.png`, `stage1-measured-wall.png`, `stage1_ecm_data.csv` | `stage1-measured.tsv` | the four `STREAM_FILE_ecm.cfg` files above |
| `stage2-single-timestep/figures/stage2-{wall,gflops,gb}.png`, `stage2_data.csv` | `stage2-measured-100.tsv` | `simulated/<cfg>/slurm-discostic-pass2.out` |
| `stage2-single-timestep/figures-o1/stage2-{wall,gflops,gb}.png`, `stage2_data.csv` | `-O3` TSV and `measured-o1/stage2-measured-100.tsv` | `simulated/` and `simulated-o1/` pass-2 |
| `stage3-multi-timestep/figures/stage3-{wall,gflops,gb}.png`, `stage3_data.csv` | `stage3-measured.tsv` | `simulated/<cfg>/slurm-discostic-pass2.out` |
| `stage3-multi-timestep/figures-o1/stage3-{wall,gflops,gb}.png`, `stage3_data.csv` | `-O3` TSV and `measured-o1/stage3-measured.tsv` | `simulated/` and `simulated-o1/` pass-2 |
| `stage4-idlewave/figures/stage4-{wall,gflops,gb}.png`, `stage4_metrics.csv` | `stage4-measured.tsv` (last `icx` and `spr` rows) | `simulated/{icx,spr}/slurm-discostic-pass2.out` |
| Stage 4 traces | `measured-trace/{icx,spr}/*-measured.single.stf` | `simulated/{icx,spr}/*.single.stf` |
| `stage5-scaling/figures/stage5-{icx,spr}-{wall,gflops,gb,efficiency}.png`, `stage5_scaling_wall.csv` | `stage5-measured.tsv` | `simulated/{strong,weak}-{icx,spr}-<N>node/slurm-discostic-pass2.out` |

Stage 5 also writes one PNG per series (`stage5-{strong,weak}-{icx,spr}-<metric>.png`). Stage 2 and 3 likewise write per-platform PNGs. CSV columns include `wall_err_pct` and `rate_err_pct`. Stage 4 has no Gantt chart; open the STF files in Intel Trace Analyzer.
