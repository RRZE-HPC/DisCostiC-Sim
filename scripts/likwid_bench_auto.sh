#!/bin/bash -l

#SBATCH --job-name=likwid_bench_auto
#SBATCH --partition=cpu
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --time=24:00:00
#SBATCH --output=outputs/logs/likwid_bench_auto.o%j
#SBATCH --error=outputs/errors/likwid_bench_auto.e%j

set -euo pipefail

# ---------------------------------------------------------------------------
# REQUIRED SLURM RESOURCES
# ---------------------------------------------------------------------------

CPUS_PER_TASK="${SLURM_CPUS_PER_TASK:?ERROR: submit with --cpus-per-task}"

CPU_FREQ="${SLURM_CPU_FREQ:-${SLURM_CPU_FREQ_REQ:-${LIKWID_BENCH_CPU_FREQ:-}}}"
if [[ -z "${CPU_FREQ}" ]]; then
  echo "ERROR: CPU frequency for srun is unknown (no SLURM_CPU_FREQ / SLURM_CPU_FREQ_REQ)." >&2
  echo "Pass --cpu-freq on sbatch, and/or export LIKWID_BENCH_CPU_FREQ to match it." >&2
  echo "Example:" >&2
  echo "  sbatch --cpus-per-task=192 --cpu-freq=2000000-2000000:performance script.sh" >&2
  echo "  export LIKWID_BENCH_CPU_FREQ=2000000-2000000:performance   # if Slurm omits the env vars" >&2
  exit 1
fi

TIMESTAMP=$(date +%Y%m%d_%H%M%S)
OUT=${1:-machine-files/${TIMESTAMP}-machine-file.yml}
EXTRA="${LIKWID_BENCH_AUTO_EXTRA:-}"

cd "${SLURM_SUBMIT_DIR}" || exit 1

echo "CPUS_PER_TASK: ${CPUS_PER_TASK}"
echo "CPU_FREQ:      ${CPU_FREQ}"
echo "Output:        ${OUT}"

# ---------------------------------------------------------------------------
# Modules
# ---------------------------------------------------------------------------
module purge 2>/dev/null || true
module load python
module --force load cpu-env/2026 gpu-env/2025
module load likwid/5.5.1

export PATH="$HOME/.local/bin:$PATH"

PYTHON="$(command -v python3)"

ensure_kerncraft() {
  "${PYTHON}" -c "import kerncraft" 2>/dev/null && return 0
  "${PYTHON}" -m pip install --user --upgrade kerncraft
  "${PYTHON}" -c "import kerncraft" 2>/dev/null || {
    echo "ERROR: kerncraft not available" >&2
    exit 1
  }
}

ensure_pyyaml() {
  "${PYTHON}" -c "import yaml" 2>/dev/null && return 0
  "${PYTHON}" -m pip install --user --upgrade pyyaml
  "${PYTHON}" -c "import yaml" 2>/dev/null || {
    echo "ERROR: PyYAML not available" >&2
    exit 1
  }
}

ensure_kerncraft
ensure_pyyaml

mkdir -p "$(dirname "${OUT}")"

echo "Running kerncraft machine model..."

# ---------------------------------------------------------------------------
# Execution (frequency enforced here)
# ---------------------------------------------------------------------------
srun --ntasks=1 \
     --cpus-per-task="${CPUS_PER_TASK}" \
     --cpu-freq="${CPU_FREQ}" \
     "${PYTHON}" -m kerncraft.machinemodel ${EXTRA} "${OUT}"

echo "Finished benchmark: $(date)"
echo "Raw output written to: ${OUT}"

# ---------------------------------------------------------------------------
# Reformat auto-bench output into kerncraft node-level machine-file layout.
# Result is written next to the source file with `_formatted` suffix; the
# benchmarks block (kernels + measurements) is preserved byte-for-byte.
# ---------------------------------------------------------------------------
OUT_DIR="$(dirname "${OUT}")"
OUT_BASE="$(basename "${OUT}")"
if [[ "${OUT_BASE}" == *.* ]]; then
  OUT_NAME="${OUT_BASE%.*}"
  OUT_EXT="${OUT_BASE##*.}"
  FORMATTED="${OUT_DIR}/${OUT_NAME}_formatted.${OUT_EXT}"
else
  FORMATTED="${OUT_DIR}/${OUT_BASE}_formatted"
fi

if [[ ! -s "${OUT}" ]]; then
  echo "WARNING: ${OUT} is missing or empty; skipping reformat" >&2
else
  echo "Reformatting ${OUT} -> ${FORMATTED} ..."
  INPUT_FILE="${OUT}" OUTPUT_FILE="${FORMATTED}" "${PYTHON}" - <<'PYEOF'
"""
Inline reformatter: convert a likwid auto-bench YAML output into the
kerncraft node-level machine-file format used in nodelevel/machine-files/
(reference: Zen2_EPYC-7452.yml).

Input/output paths come from the INPUT_FILE / OUTPUT_FILE env vars.
The `benchmarks:` block from the source file is preserved verbatim so
kernel + measurement arrays are not re-flowed by the YAML emitter.
"""
import os
import sys
from collections import OrderedDict

import yaml


# Parse !!omap as OrderedDict so source ordering is preserved.
def _construct_omap(loader, node):
    omap = OrderedDict()
    for subnode in node.value:
        for key_node, value_node in subnode.value:
            key = loader.construct_object(key_node)
            val = loader.construct_object(value_node)
            omap[key] = val
    return omap


yaml.add_constructor(
    "tag:yaml.org,2002:omap", _construct_omap, Loader=yaml.SafeLoader
)


def _represent_ordereddict(dumper, data):
    return dumper.represent_mapping("tag:yaml.org,2002:map", data.items())


yaml.add_representer(OrderedDict, _represent_ordereddict, Dumper=yaml.SafeDumper)


_BLANK = object()

TOP_LEVEL_ORDER = [
    "kerncraft version", "model name", "model type", "clock", _BLANK,
    "sockets", "cores per socket", "threads per core",
    "NUMA domains per socket", "cores per NUMA domain",
    "transparent hugepage", _BLANK,
    "in-core model", "isa", _BLANK, _BLANK,
    "compiler", _BLANK,
    "FLOPs per cycle", _BLANK,
    "non-overlapping model", "overlapping model", _BLANK,
    "cacheline size", "memory hierarchy", _BLANK,
    "benchmarks", "machine state",
]

LEVEL_KEY_ORDER = [
    "level", "cache per group", "cores per group", "groups",
    "upstream throughput", "transfers overlap",
    "penalty cycles per cacheline load", "penalty cycles per cacheline store",
    "performance counter metrics", "size per group", "threads per group",
]

CACHE_PER_GROUP_ORDER = [
    "sets", "ways", "cl_size",
    "replacement_policy", "write_allocate", "write_back",
    "load_from", "victims_to", "store_to",
]

PERF_COUNTER_ORDER = ["loads", "loads+stores", "stores", "evicts", "misses"]
COMPILER_PREFERRED = ["clang", "gcc", "icc"]
INCORE_PREFERRED = ["OSACA", "LLVM-MCA", "IACA"]


def _dump_scalar(value):
    if value is None:
        return "~"
    if isinstance(value, bool):
        return "true" if value else "false"
    text = yaml.safe_dump(
        value, default_flow_style=True, allow_unicode=True, width=10**9
    ).rstrip("\n")
    if text.endswith("\n..."):
        text = text[:-4]
    return text


def _render_inline(value):
    if value is None:
        return "~"
    text = yaml.safe_dump(
        value, default_flow_style=True, allow_unicode=True, width=10**9
    ).rstrip("\n")
    if text.endswith("\n..."):
        text = text[:-4]
    return text


def _ordered_items(d, order):
    seen = set()
    for k in order:
        if k in d:
            seen.add(k)
            yield k, d[k]
    for k, v in d.items():
        if k not in seen:
            yield k, v


def _reorder(d, preferred):
    if not isinstance(d, dict):
        return d
    out = OrderedDict()
    for k, v in _ordered_items(d, preferred):
        out[k] = v
    return out


def _emit_omap(key, omap, out, preferred):
    out.append(f"{key}: !!omap")
    for k, v in _ordered_items(omap, preferred):
        out.append(f"  - {k}: {_dump_scalar(v)}")


def _emit_flops(value, out):
    out.append("FLOPs per cycle:")
    inner_order = ["ADD", "FMA", "MUL", "total"]
    for band in ("DP", "SP"):
        if band not in value:
            continue
        inner = value[band]
        if isinstance(inner, dict):
            inner = _reorder(inner, inner_order)
            out.append(f"  {band}: {_render_inline(inner)}")
        else:
            out.append(f"  {band}: {_dump_scalar(inner)}")
    for k, v in value.items():
        if k in ("DP", "SP"):
            continue
        if isinstance(v, dict):
            out.append(f"  {k}: {_render_inline(v)}")
        else:
            out.append(f"  {k}: {_dump_scalar(v)}")


def _emit_model(key, value, out):
    out.append(f"{key}:")
    pcm_key = "performance counter metric"
    if pcm_key in value:
        out.append(f"  {pcm_key}: {_dump_scalar(value[pcm_key])}")
    if "ports" in value:
        ports = value["ports"]
        if isinstance(ports, dict):
            out.append("  ports:")
            for k, v in ports.items():
                out.append(f"    {k}: {_render_inline(v)}")
        elif isinstance(ports, list):
            out.append(f"  ports: {_render_inline(ports)}")
        else:
            out.append(f"  ports: {_dump_scalar(ports)}")
    for k, v in value.items():
        if k in (pcm_key, "ports"):
            continue
        out.append(f"  {k}: {_dump_scalar(v)}")


def _emit_memory_hierarchy(levels, out):
    out.append("memory hierarchy:")
    for level in levels:
        if not isinstance(level, dict):
            out.append(f"- {_dump_scalar(level)}")
            continue
        first = True
        for k, v in _ordered_items(level, LEVEL_KEY_ORDER):
            prefix = "- " if first else "  "
            first = False
            if k == "cache per group" and isinstance(v, dict):
                out.append(f"{prefix}{k}:")
                for ck, cv in _ordered_items(v, CACHE_PER_GROUP_ORDER):
                    out.append(f"    {ck}: {_dump_scalar(cv)}")
            elif k == "performance counter metrics" and isinstance(v, dict):
                out.append(f"{prefix}{k}:")
                for pk, pv in _ordered_items(v, PERF_COUNTER_ORDER):
                    out.append(f"    {pk}: {_dump_scalar(pv)}")
            elif k == "upstream throughput" and isinstance(v, list):
                out.append(f"{prefix}{k}: {_render_inline(v)}")
            else:
                out.append(f"{prefix}{k}: {_dump_scalar(v)}")


def _emit_machine_state(value, out):
    dumped = yaml.safe_dump(
        {"machine state": value},
        default_flow_style=False,
        allow_unicode=True,
    )
    out.append(dumped.rstrip("\n"))


def _extract_benchmarks_block(input_text):
    lines = input_text.split("\n")
    start = None
    for i, line in enumerate(lines):
        if line.startswith("benchmarks:"):
            start = i
            break
    if start is None:
        return None
    end = len(lines)
    for i in range(start + 1, len(lines)):
        line = lines[i]
        if line and not line[0].isspace() and not line.startswith("#"):
            end = i
            break
    return "\n".join(lines[start:end]).rstrip("\n")


def _build(data, benchmarks_text):
    out = []
    for key in TOP_LEVEL_ORDER:
        if key is _BLANK:
            out.append("")
            continue
        if key not in data:
            continue
        value = data[key]
        if key == "in-core model":
            _emit_omap(key, value, out, INCORE_PREFERRED)
        elif key == "compiler":
            _emit_omap(key, value, out, COMPILER_PREFERRED)
        elif key == "FLOPs per cycle":
            _emit_flops(value, out)
        elif key in ("non-overlapping model", "overlapping model"):
            _emit_model(key, value, out)
        elif key == "memory hierarchy":
            _emit_memory_hierarchy(value, out)
        elif key == "benchmarks":
            if benchmarks_text:
                out.append(benchmarks_text)
        elif key == "machine state":
            _emit_machine_state(value, out)
        else:
            out.append(f"{key}: {_dump_scalar(value)}")
    while out and out[-1] == "":
        out.pop()
    return "\n".join(out) + "\n"


def main():
    input_path = os.environ.get("INPUT_FILE")
    output_path = os.environ.get("OUTPUT_FILE")
    if not input_path or not output_path:
        print("ERROR: INPUT_FILE / OUTPUT_FILE env vars required", file=sys.stderr)
        return 1
    with open(input_path, "r", encoding="utf-8") as fh:
        src = fh.read()
    data = yaml.safe_load(src)
    if not isinstance(data, dict):
        print(f"ERROR: {input_path} root is not a YAML mapping", file=sys.stderr)
        return 1
    benchmarks_text = _extract_benchmarks_block(src)
    output = _build(data, benchmarks_text)
    with open(output_path, "w", encoding="utf-8") as fh:
        fh.write(output)
    print(f"reformatted -> {output_path}")
    return 0


sys.exit(main())
PYEOF
  echo "Reformatted: ${FORMATTED}"
fi

echo "Finished: $(date)"
echo "Raw output:       ${OUT}"
echo "Formatted output: ${FORMATTED}"
