# DisCostiC Demo

Validation suite for **DisCostiC** (Distributed Communicating Stencil Toolkit).

Each folder is self-contained: benchmark, DisCostiC configs, SLURM jobs, results, and figures. Demos run on Intel Ice Lake-SP (ICX) and, where noted, Sapphire Rapids (SPR) at NHR@FAU (Fritz).

## Demos

| Demo | Kernel | Stages | README |
|------|--------|--------|--------|
| [`streamtriad/`](streamtriad/) | STREAM Triad (`STREAM_FILE`) | 1–5 · ICX + SPR · O3 + O1 (stages 2–3) | [`streamtriad/README.md`](streamtriad/README.md) |
| [`pisolver-stream/`](pisolver-stream/) | Pi + STREAM (`PITRIADLBL`) | 1–5 · ICX · O3 | [`pisolver-stream/README.md`](pisolver-stream/README.md) |
| [`raytracing-opticalflow/`](raytracing-opticalflow/) | Ray tracing + optical flow (`RTOF4`) | 1–5 · ICX · O3 | [`raytracing-opticalflow/README.md`](raytracing-opticalflow/README.md) |
| [`heat/`](heat/) | Jacobi 2D (`HEAT_FILE`) | 1–5 · ICX · O3 + penalty | [`heat/README.md`](heat/README.md) |
| [`collectives/`](collectives/) | MPI collectives vs IMB | 1–2 · ICX | [`collectives/README.md`](collectives/README.md) |

Place this tree as a direct subdirectory of a `discostic-extension` checkout. Scripts resolve paths from their own location. If the demo root is moved:

```bash
export DISCOSTIC_EXTENSION_ROOT=/absolute/path/to/discostic-extension
```

Submit `sbatch` jobs from inside the relevant `*/` directory so relative `--output` and `--error` paths resolve.

```
DisCostiC/
├── README.md
├── metrics.py                         # shared wall, rate, and collective formulas
├── streamtriad/
├── pisolver-stream/
├── raytracing-opticalflow/
├── heat/
└── collectives/
```

## Run

```bash
cd <kernel>/
bash setup_output_dirs.sh
module load intel/2023.2.1 intelmpi/2021.16.0 python git cmake likwid
# fallback: intel/2025.2.0 intelmpi/2021.16.0

python3 run_job_chain.py --dry-run --stages 1,2,3,4,5
python3 run_job_chain.py --stages 1,2,3,4,5
python3 generate_figures.py --stages 1,2,3,4,5   # STREAM: shared/scripts/generate_figures.py
```

The chain skips steps whose artifacts already verify. Use `--force` to re-run them. Collectives uses `--stages 1,2` and generates figures for stage 1 only.

Regenerate figures from existing artifacts:

```bash
python3 streamtriad/shared/scripts/generate_figures.py --stages 1,2,3,4,5
python3 pisolver-stream/generate_figures.py --stages 1,2,3,4,5
python3 raytracing-opticalflow/generate_figures.py --stages 1,2,3,4,5
python3 heat/generate_figures.py --stages 1,2,3,4,5
python3 collectives/generate_figures.py --stages 1
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

1. Create `DisCostiC/<kernel>/` next to the existing demos.
2. Keep scripts, configs, and benchmarks inside that demo.
3. Add `<kernel>/README.md` and link it from the table above.
