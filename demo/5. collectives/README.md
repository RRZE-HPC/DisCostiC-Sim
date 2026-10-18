# Collectives demo

Validates DisCostiC MPI collectives against Intel MPI Benchmarks (IMB) on Fritz Ice Lake-SP, `-O3` / AVX-512, MPI only. The kernel is `test/STREAM_COLLECTIVE.hpp` (STREAM plus one collective). Measured runs use `IMB-MPI1` with `I_MPI_ADJUST_*` set by the job.

Suite overview: [`../README.md`](../README.md).

This demo does not cover Allgather, Alltoall, Gather, Scatter, Barrier, non-blocking collectives, or neighborhood collectives. `collective_mode = 1` is one timestep and homogeneous ranks only.

## Stages

| Stage | What it checks | Placement |
|------:|----------------|-----------|
| 1 | Collective duration versus IMB, four algorithms | IMB: 1 node × 72 ranks, shared memory. Simulation: 2 nodes, internode LogGP |
| 2 | Trace of binomial broadcast | Same placement. Open the STF files in Intel Trace Analyzer. No plot. |

## Algorithms

Confirm the Intel MPI algorithm IDs on the loaded module before treating the measurement as the reference:

```bash
impi_info -v I_MPI_ADJUST_BCAST
impi_info -v I_MPI_ADJUST_REDUCE
impi_info -v I_MPI_ADJUST_ALLREDUCE
```

The jobs set these IDs:

| Case | DisCostiC | Job setting |
|------|-----------|-------------|
| Broadcast, binomial | `collective_type=0`, `collective_algorithm=0` | `I_MPI_ADJUST_BCAST=1` |
| Broadcast, ring | `collective_type=0`, `collective_algorithm=1` | `I_MPI_ADJUST_BCAST=3` |
| Reduce, binomial | `collective_type=1`, `collective_algorithm=0` | `I_MPI_ADJUST_REDUCE=2` |
| Allreduce, recursive doubling | `collective_type=2`, `collective_algorithm=0` | `I_MPI_ADJUST_ALLREDUCE=1` |

Message sizes are 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, and 1024 KB (`shared/sizes.txt`). Through about 64 KB the model stays near IMB. Above that it underestimates IMB, and the figures mark that change.

Simulation configs use `task_per_node = 36` and `number_of_processes = 72`, so the network model is internode LogGP. Simulation jobs run on 2 nodes (`ntasks = 73`: 72 workers plus the coordinator). Binomial trees have `ceil(log2(72)) = 7` stages. IMB runs on 1 node × 72 ranks (`singlenode`, `I_MPI_FABRICS=shm`, pin 0–71).

Ring broadcast returns the same duration for every rank, so the simulated minimum equals the simulated maximum. Reduce adds `gamma = messageSize * 0.001` per stage. Allreduce uses `2 × ceil(log2 N)` stages and a per-rank delay. Binomial broadcast uses eager `L + o + m · max(g, G)` below the eager limit and `m / bandwidth` above it (`eagerLimit = 65535`). Collectives always use the LogGP parameters; `comm_model` in the config applies to point-to-point only. Idle wave is off.

IMB jobs use `-iter 10000 -iter_policy dynamic`, `-npmin 72`, and `-msglen shared/sizes.txt`. Rooted operations use `-root_shift on`. Allreduce uses `-root_shift off`. Pinning is `I_MPI_PIN=1` and `I_MPI_PIN_DOMAIN=core`. `-warm_up` is not set.

## Layout

```
5. collectives/
├── shared/
│   ├── sizes.txt                # IMB -msglen list, in bytes
│   └── scripts/                 # DisCostiC and IMB helpers
├── stage1-collective-cost/      # four algorithms, simulation and IMB
├── stage2-traces/               # binomial broadcast traces
├── setup_output_dirs.sh
├── run_job_chain.py
└── generate_figures.py
```

## Run

```bash
cd "demo/5. collectives/"
bash setup_output_dirs.sh
module load intel/2023.2.1 intelmpi/2021.16.0 python git cmake likwid

python3 run_job_chain.py --dry-run --stages 1,2
python3 run_job_chain.py --stages 1,2
python3 generate_figures.py --stages 1
```

Submit SLURM jobs from inside `demo/5. collectives/`. The chain skips steps whose artifacts already verify. Use `--force` to re-run them.

Stage 2 traces:

```bash
module load intel-itac/2022.4.0
traceanalyzer stage2-traces/results/simulated/icx/bcast-binomial.single.stf
traceanalyzer stage2-traces/results/measured-trace/icx/bcast-binomial-measured.single.stf
```

## How results are read

Scoring uses collective duration, not application wall. Error is computed separately for the minimum and the maximum:

```
err = (sim − meas) / meas
```

| Side | File | Fields |
|------|------|--------|
| Simulation | `stage1-collective-cost/results/simulated/<algo>/slurm-discostic-pass2.out` | `Collective Performance Analysis (Multiple Sizes)` |
| IMB | `stage1-collective-cost/results/measured/<algo>/imb.out` | `t_min`, `t_max`, `t_avg` |

Parsers are in [`../metrics.py`](../metrics.py): `parse_sim_collective_sizes`, `parse_imb_collective_sizes`, and `collective_score_rows`. Scored sizes are the 1–1024 KB list above. IMB times are microseconds; a zero-byte row is dropped.

### Config keys

| Key | Meaning |
|-----|---------|
| `collective_mode` | `1` = one timestep, homogeneous ranks |
| `collective_type` | `0` broadcast, `1` reduce, `2` allreduce |
| `collective_algorithm` | `0` binomial or recursive doubling, `1` broadcast ring |
| `collective_bytes` | Payload on the collective node. The printed size table is separate. |

## Figures

`generate_figures.py --stages 1` reads the pass-2 file and `imb.out` for each algorithm and writes:

| Output | Contents |
|--------|----------|
| `stage1-collective-cost/figures/stage1-<algo>.png` | Minimum and maximum duration versus message size, simulation and IMB |
| `stage1-collective-cost/figures/stage1-fig6.1.png` | The four algorithms on one page |
| `stage1-collective-cost/figures/stage1_collective_cost.csv` | Size, sim and IMB min/avg/max, signed error, absolute difference |

`<algo>` is `bcast-binomial`, `bcast-ring`, `reduce-binomial`, or `allreduce-recdbl`. Stage 2 is not plotted.
