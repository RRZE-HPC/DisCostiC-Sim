# Running `likwid_bench_auto` via SLURM on Helma

## Purpose

`likwid_bench_auto.sh` submits a SLURM job that runs kerncraft's
`likwid_bench_auto` benchmark suite on a compute node, then automatically
reformats the raw output into the kerncraft node-level machine-file layout.
Two YAML files are produced per run: the raw benchmark output and a formatted
version ready for use with kerncraft performance models.

The job must run on a real compute node (not a login node) so that:
- all physical CPU cores and cache topology are accessible,
- the CPU frequency can be pinned to a stable value for reproducible measurements.

> **Wall-time note:** Full benchmarking across all cache levels and core counts
> can exceed 24 hours.  The default `--time` in the script is a short placeholder
> (`00:05:00`) — always override it with `--time` on the `sbatch` command.
> For runs expected to exceed the standard wall-time limit, request a dedicated
> or reserved node through **NHR FAU support** before submitting.

## Environment note

The module names, versions, and load order in this script are specific to the
**NHR Helma** cluster.  Each Helma node contains two AMD EPYC 9965 processors,
each with 192 cores.  The benchmark targets a **single processor (192 cores)**,
not the full node (384 cores).  On any other HPC system the module stack will
differ — software names, version numbers, and dependency constraints vary between
clusters and change over time as the system software is updated.  Before running
on a different system:

1. Check available modules: `module avail python`, `module avail likwid`
2. Confirm the loaded environment: `module list`
3. Verify LIKWID and kerncraft versions are compatible.

## Before running — required directory setup

The script writes SLURM logs to `outputs/logs/` and `outputs/errors/`.  Create
these directories once in your project root before the first submission:

```bash
mkdir -p outputs/{logs,errors}
```

The `outputs/` directory is listed in `.gitignore` and will not appear in
`git status`.

## Submitting the job

```bash
sbatch --cpus-per-task=<N> --cpu-freq=<FREQ> --time=<HH:MM:SS> [sbatch-flags] \
  scripts/likwid_bench_auto.sh [OUT]
```

### Required sbatch flags

Both flags must be provided on the `sbatch` command line.  The job aborts
immediately if either is missing.

| Flag | Description |
|------|-------------|
| `--cpus-per-task=N` | Number of CPUs to allocate. SLURM sets `$SLURM_CPUS_PER_TASK`; `srun` reads it automatically. |
| `--cpu-freq=FREQ` | CPU frequency spec. SLURM sets `$SLURM_CPU_FREQ`; `srun` enforces it at the task-step level. |

`FREQ` follows the format `<min_kHz>-<max_kHz>:<governor>`.  Setting min and max
to the same value pins the frequency; `performance` disables turbo scaling.
For a single Helma processor (AMD EPYC 9965, 192 cores) the nominal clock is
**2.25 GHz**: `2250000-2250000:performance`.

### Positional script argument

| Position | Variable | Default | Description |
|----------|----------|---------|-------------|
| `$1` | `OUT` | `machine-files/<timestamp>-machine-file.yml` | Output path for the raw machine-model YAML |

The timestamp in the default filename is generated at job start in the format
`YYYYMMDD_HHMMSS`, e.g. `machine-files/20250427_143012-machine-file.yml`.

### `#SBATCH` defaults

These apply when the corresponding flag is not passed on the `sbatch` command
line.  `--cpus-per-task` and `--cpu-freq` are intentionally absent — both are
required and must always be provided explicitly.

| Directive | Default | Note |
|-----------|---------|------|
| `--partition` | `cpu` | |
| `--time` | `00:05:00` | Placeholder — always override; full runs take up to 24 h |

### Environment variable overrides

| Variable | Default | Description |
|----------|---------|-------------|
| `LIKWID_BENCH_AUTO_EXTRA` | _(empty)_ | Extra flags forwarded to `kerncraft.machinemodel` |

### Examples

```bash
# Helma: benchmark one full processor (192 cores, 2.25 GHz pinned), 24 h wall time
sbatch --cpus-per-task=192 --cpu-freq=2250000-2250000:performance --time=24:00:00 \
  scripts/likwid_bench_auto.sh

# Same, with a named output file
sbatch --cpus-per-task=192 --cpu-freq=2250000-2250000:performance --time=24:00:00 \
  scripts/likwid_bench_auto.sh machine-files/helma_epyc9965.yml

# Half the cores of one processor (96 cores), 48 h wall time
sbatch --cpus-per-task=96 --cpu-freq=2250000-2250000:performance --time=48:00:00 \
  scripts/likwid_bench_auto.sh machine-files/helma_96core.yml
```

## Post-processing — automatic reformatting

After the benchmark finishes, the script runs an inline Python reformatter that
converts the raw `likwid_bench_auto` YAML output into the kerncraft node-level
machine-file layout (field ordering and structure compatible with existing
machine files such as `Zen2_EPYC-7452.yml`).

The formatted file is written next to the raw file with a `_formatted` suffix:

| Raw output | Formatted output |
|------------|-----------------|
| `machine-files/20250427_143012-machine-file.yml` | `machine-files/20250427_143012-machine-file_formatted.yml` |
| `machine-files/helma_epyc9965.yml` | `machine-files/helma_epyc9965_formatted.yml` |

The `benchmarks:` block (kernels and measurement arrays) is preserved verbatim
from the raw file — the reformatter only reorders and restructures the
surrounding metadata.

**Dependencies:** the reformatter requires PyYAML (`import yaml`).  The script
installs it automatically in user space (`pip install --user pyyaml`) if it is
not already available, in the same way kerncraft is handled.

## Output locations

| File | Path |
|------|------|
| Raw machine-model YAML | `machine-files/<timestamp>-machine-file.yml` |
| Formatted machine-model YAML | `machine-files/<timestamp>-machine-file_formatted.yml` |
| SLURM stdout log | `outputs/logs/likwid_bench_auto.o<jobid>` |
| SLURM stderr log | `outputs/errors/likwid_bench_auto.e<jobid>` |
