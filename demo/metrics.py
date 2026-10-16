#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: Shared derived-metric formulas for DisCostiC-demo figure scripts.
#          Wall time remains the scoring quantity for the wall figure:
#          error = (sim − meas) / meas. A rate is that run's work divided by
#          the combined wall, so its rate error is −wall_err / (1 + wall_err).
#          Pi+STREAM GFlop/s adds both kernels' flops. RT+OF Mpix/s adds both
#          kernels' pixels. Neither figure splits the run by kernel.
#
# References:
#   HEAT: Kerncraft heat.c — 5 flops/update, 32 B/update (AI 0.15625 F/B).
#   STREAM / Pi: write-allocate traffic = 4 cache lines (2 load + store + WA).
#     t/iter = (T_ECM / freq) * (8 / CL_bytes) with CL = 64 B.
#   Collectives: score min/max collective duration vs IMB,
#     not application wall. Parsers in parse_sim_collective_sizes /
#     parse_imb_collective_sizes.

from __future__ import annotations

import csv
import re
from pathlib import Path
from typing import Any

# Ice Lake-SP demo clock (Fritz ICX jobs pin 2.4 GHz).
ICX_FREQ_GHZ = 2.4
CACHELINE_BYTES = 64
HEAT_FLOPS_PER_UPDATE = 5
HEAT_BYTES_PER_UPDATE = 32
STREAM_FLOPS_PER_ELEM = 2
STREAM_STREAMS_WA = 4  # 2 loads + 1 store + write-allocate
PI_FLOPS_PER_INDEX = 1  # DIVIDE: one FP op per piSize index


COLLECTIVE_SCORE_SIZES_KB = (1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024)
COLLECTIVE_ACCURACY_CLIFF_KB = 64


def parse_sim_collective_sizes(path: Path | None) -> dict[int, dict[str, float]]:
    """Parse DisCostiC ``Collective Performance Analysis (Multiple Sizes)`` table.

    Returns ``{size_kb: {min_us, avg_us, max_us}}``. Times are microseconds.
    """
    out: dict[int, dict[str, float]] = {}
    if path is None or not path.is_file():
        return out
    in_table = False
    row_re = re.compile(
        r"^\s*(\d+)\s+([0-9.+-eE]+)\s+([0-9.+-eE]+)\s+([0-9.+-eE]+)\s*$"
    )
    with path.open(errors="replace") as fh:
        for line in fh:
            if "Collective Performance Analysis (Multiple Sizes)" in line:
                in_table = True
                continue
            if not in_table:
                continue
            if "Size" in line and "Min" in line:
                continue
            if set(line.strip()) <= set("-"):
                if out:
                    break
                continue
            m = row_re.match(line)
            if not m:
                if line.strip() and out:
                    break
                continue
            size_kb = int(m.group(1))
            out[size_kb] = {
                "min_us": float(m.group(2)),
                "avg_us": float(m.group(3)),
                "max_us": float(m.group(4)),
            }
    return out


def parse_imb_collective_sizes(path: Path | None) -> dict[int, dict[str, float]]:
    """Parse IMB-MPI1 ``#bytes t_min t_max t_avg`` table into ``{size_kb: ...}``.

    IMB times are microseconds. Zero-byte rows are dropped.
    """
    out: dict[int, dict[str, float]] = {}
    if path is None or not path.is_file():
        return out
    in_table = False
    row_re = re.compile(
        r"^\s*(\d+)\s+(\d+)\s+([0-9.+-eE]+)\s+([0-9.+-eE]+)\s+([0-9.+-eE]+)"
    )
    with path.open(errors="replace") as fh:
        for line in fh:
            s = line.strip()
            if "#bytes" in s.replace(" ", "") or ("#bytes" in s and "t_min" in s):
                in_table = True
                continue
            if not in_table:
                continue
            m = row_re.match(s)
            if not m:
                if s.startswith("#") or not s:
                    if out:
                        break
                    continue
                continue
            nbytes = int(m.group(1))
            if nbytes <= 0 or nbytes % 1024 != 0:
                continue
            size_kb = nbytes // 1024
            out[size_kb] = {
                "min_us": float(m.group(3)),
                "max_us": float(m.group(4)),
                "avg_us": float(m.group(5)),
            }
    return out


def collective_score_rows(
    sim: dict[int, dict[str, float]],
    meas: dict[int, dict[str, float]],
    sizes_kb: tuple[int, ...] = COLLECTIVE_SCORE_SIZES_KB,
) -> list[dict[str, float]]:
    """Join sim and IMB tables; error = (sim − meas) / meas on min and max."""
    rows: list[dict[str, float]] = []
    for size_kb in sizes_kb:
        s = sim.get(size_kb)
        m = meas.get(size_kb)
        if not s or not m:
            continue
        rows.append(
            {
                "size_kb": float(size_kb),
                "sim_min_us": s["min_us"],
                "sim_avg_us": s["avg_us"],
                "sim_max_us": s["max_us"],
                "meas_min_us": m["min_us"],
                "meas_avg_us": m["avg_us"],
                "meas_max_us": m["max_us"],
                "min_err_pct": signed_error_pct(s["min_us"], m["min_us"]),
                "max_err_pct": signed_error_pct(s["max_us"], m["max_us"]),
                "avg_err_pct": signed_error_pct(s["avg_us"], m["avg_us"]),
            }
        )
    return rows


def signed_error_pct(sim: float, meas: float) -> float:
    """Scoring formula used by all demos: (sim − meas) / meas × 100."""
    if meas == 0:
        return float("nan")
    return (sim - meas) / meas * 100.0


def rate(count: float, wall_s: float, scale: float) -> float:
    """Generic count / wall / scale (returns 0 if wall ≤ 0)."""
    if wall_s <= 0:
        return 0.0
    return count / wall_s / scale


def heat_mlup_s(dim_x: int, dim_y: int, ntimes: int, wall_s: float) -> float:
    """Lattice updates per second (millions): dim_x × dim_y × ntimes / wall / 1e6."""
    return rate(dim_x * dim_y * ntimes, wall_s, 1e6)


def heat_gf_s(dim_x: int, dim_y: int, ntimes: int, wall_s: float) -> float:
    """HEAT GFlop/s from 5 flops/point."""
    return rate(HEAT_FLOPS_PER_UPDATE * dim_x * dim_y * ntimes, wall_s, 1e9)


def heat_gb_s(dim_x: int, dim_y: int, ntimes: int, wall_s: float) -> float:
    """HEAT GB/s from 32 B/update (Kerncraft bytes transfer)."""
    return rate(HEAT_BYTES_PER_UPDATE * dim_x * dim_y * ntimes, wall_s, 1e9)


def heat_it_s(ntimes: int, wall_s: float) -> float:
    """Iterations per second."""
    return rate(ntimes, wall_s, 1.0)


def stream_gflops(n_total: int, ntimes: int, wall_s: float) -> float:
    """STREAM Triad GFlop/s: 2 × N × ntimes / wall / 1e9."""
    return rate(STREAM_FLOPS_PER_ELEM * n_total * ntimes, wall_s, 1e9)


def stream_gb_s(n_total: int, ntimes: int, wall_s: float) -> float:
    """STREAM write-allocate bandwidth: 4 × 8 × N × ntimes / wall / 1e9."""
    return rate(STREAM_STREAMS_WA * 8 * n_total * ntimes, wall_s, 1e9)


def pi_gflops(nsteps: int, ntimes: int, wall_s: float) -> float:
    """Pi/DIVIDE GFlop/s: one FP op per index × Nsteps × ntimes / wall / 1e9."""
    return rate(PI_FLOPS_PER_INDEX * nsteps * ntimes, wall_s, 1e9)


def app_gflops(n_total: int, nsteps: int, ntimes: int, wall_s: float) -> float:
    """Pi+STREAM GFlop/s: (2×N + Nsteps) × ntimes / combined wall / 1e9."""
    return stream_gflops(n_total, ntimes, wall_s) + pi_gflops(nsteps, ntimes, wall_s)


def ecm_eq22_s_per_iter(
    t_ecm_cy: float,
    freq_ghz: float = ICX_FREQ_GHZ,
    cl_bytes: int = CACHELINE_BYTES,
) -> float:
    """Time per iteration [s] from ECM cycles per cache line.

    t/iter = (T_ECM / freq_Hz) * (8 / CL_bytes).  8 doubles per 64 B line.
    """
    if freq_ghz <= 0 or cl_bytes <= 0:
        return 0.0
    return (t_ecm_cy / (freq_ghz * 1e9)) * (8.0 / cl_bytes)


def rtof_loop_sizes(mg_levels: int, supersampling: int) -> tuple[int, int]:
    """Return (N_OF, N_RT) = (nxOF², nxRT²).

    nxOF = 2^(MG−1) + 2; nxRT = nxOF × SS.
    MG=9, SS=2 → 258² = 66564 and 516² = 266256.
    """
    nx_of = (2 ** (mg_levels - 1)) + 2
    nx_rt = nx_of * supersampling
    return nx_of * nx_of, nx_rt * nx_rt


def of_mpix_s(n_of: int, nreps: int, wall_s: float) -> float:
    """Optical-flow megapixels/s: N × nreps / wall / 1e6."""
    return rate(n_of * nreps, wall_s, 1e6)


def rt_mpix_s(n_rt: int, nreps: int, wall_s: float) -> float:
    """Ray-tracing megapixels/s: Nsteps × nreps / wall / 1e6."""
    return rate(n_rt * nreps, wall_s, 1e6)


def app_mpix_s(n_of: int, n_rt: int, nreps: int, wall_s: float) -> float:
    """RT+OF Mpix/s: (N + Nsteps) × nreps / combined wall / 1e6."""
    return of_mpix_s(n_of, nreps, wall_s) + rt_mpix_s(n_rt, nreps, wall_s)


def strong_efficiency(t1: float, tn: float, nodes: int) -> float:
    """E_strong = T(1) / (nodes × T(n))."""
    if tn <= 0 or nodes <= 0:
        return 0.0
    return t1 / (nodes * tn)


def weak_efficiency(t1: float, tn: float) -> float:
    """E_weak = T(1) / T(n)."""
    if tn <= 0:
        return 0.0
    return t1 / tn


def last_row_tsv(path: Path) -> dict[str, dict[str, Any]]:
    """Parse a tab TSV; last row for a given ``config`` wins (duplicate configs)."""
    out: dict[str, dict[str, Any]] = {}
    if not path.is_file():
        return out
    with path.open(newline="") as fh:
        reader = csv.DictReader(fh, delimiter="\t")
        for row in reader:
            cfg = (row.get("config") or "").strip()
            if not cfg:
                continue
            out[cfg] = dict(row)
    return out


def _as_float(row: dict, *keys: str, default: float = 0.0) -> float:
    for key in keys:
        raw = row.get(key)
        if raw is None or raw == "":
            continue
        try:
            return float(raw)
        except (TypeError, ValueError):
            continue
    return default


def _as_int(row: dict, *keys: str, default: int = 0) -> int:
    return int(_as_float(row, *keys, default=float(default)))


def heat_from_tsv_row(row: dict, wall_s: float) -> dict[str, float]:
    dim_x = _as_int(row, "dim_x")
    dim_y = _as_int(row, "dim_y")
    ntimes = _as_int(row, "ntimes")
    return {
        "mlup_s": heat_mlup_s(dim_x, dim_y, ntimes, wall_s),
        "gf_s": heat_gf_s(dim_x, dim_y, ntimes, wall_s),
        "gb_s": heat_gb_s(dim_x, dim_y, ntimes, wall_s),
        "it_s": heat_it_s(ntimes, wall_s),
        "dim_x": dim_x,
        "dim_y": dim_y,
        "ntimes": ntimes,
    }


def stream_from_tsv_row(row: dict, wall_s: float) -> dict[str, float]:
    n_total = _as_int(row, "N_total", "arraySize", "N")
    ntimes = _as_int(row, "ntimes")
    return {
        "gflops": stream_gflops(n_total, ntimes, wall_s),
        "gb_s": stream_gb_s(n_total, ntimes, wall_s),
        "N": n_total,
        "ntimes": ntimes,
    }


def pi_from_tsv_row(row: dict, wall_s: float) -> dict[str, float]:
    nsteps = _as_int(row, "piSize", "Nsteps")
    n_total = _as_int(row, "arraySize", "N")
    ntimes = _as_int(row, "ntimes")
    return {
        "gflops": app_gflops(n_total, nsteps, ntimes, wall_s),
        "pi_gflops": pi_gflops(nsteps, ntimes, wall_s),
        "stream_gflops": stream_gflops(n_total, ntimes, wall_s),
        "stream_gb_s": stream_gb_s(n_total, ntimes, wall_s),
        "Nsteps": nsteps,
        "N": n_total,
        "ntimes": ntimes,
    }


def rtof_from_tsv_row(row: dict, wall_s: float) -> dict[str, float]:
    levels = _as_int(row, "levels")
    ss = _as_int(row, "ss")
    nreps = _as_int(row, "nreps", "ntimes")
    n_of, n_rt = rtof_loop_sizes(levels, ss) if levels > 0 else (0, 0)
    return {
        "mpix_s": app_mpix_s(n_of, n_rt, nreps, wall_s),
        "of_mpix_s": of_mpix_s(n_of, nreps, wall_s),
        "rt_mpix_s": rt_mpix_s(n_rt, nreps, wall_s),
        "N": n_of,
        "Nsteps": n_rt,
        "nreps": nreps,
        "levels": levels,
        "ss": ss,
        "inner_steps": _as_int(row, "inner_steps", default=1),
    }


def rate_err_pct(sim_rate: float, meas_rate: float) -> float:
    """Signed rate error ``(sim − meas) / meas``.

    When both rates divide the same work count by the same wall, this equals
    ``−wall_err / (1 + wall_err)``. Kernel rates use each kernel's own time,
    so the two kernels no longer share one percentage.
    """
    return signed_error_pct(sim_rate, meas_rate)


def _quoted_field(line: str, key: str) -> str | None:
    token = f'"{key}":'
    i = line.find(token)
    if i < 0:
        return None
    rest = line[i + len(token) :].strip().rstrip(",")
    if rest.startswith('"'):
        return rest.strip('"')
    return rest


def parse_sim_kernel_seconds(sim_dir: Path, rank: int) -> tuple[dict[str, float], dict[str, int]]:
    """Sum Chrome-trace durations for one simulated rank.

    DisCosTiC writes ``dur`` in microseconds (nanoseconds / 1000). Kernel
    events are named ``part1`` and ``part2`` (DIVIDE/STREAM or RT/OF), often
    prefixed with the job id. Returns ``(seconds_by_name, count_by_name)``.
    A trace may contain fewer timesteps than the run; callers scale by
    ``ntimes / count`` after checking that the scaled sum matches the pass-2 wall.
    """
    traces = sorted(p for p in sim_dir.glob("*.dms") if p.is_file())
    if not traces:
        return {}, {}
    pid_line = f'"pid": {rank},'
    sums: dict[str, float] = {}
    counts: dict[str, int] = {}
    name: str | None = None
    dur_s: float | None = None
    with traces[0].open(errors="replace") as fh:
        for line in fh:
            if '"dur"' in line:
                raw = _quoted_field(line, "dur")
                dur_s = float(raw) / 1e6 if raw else None
            elif '"name"' in line:
                raw = _quoted_field(line, "name")
                name = None if raw in (None, "dummy") else raw.rsplit("/", 1)[-1]
            elif pid_line in line:
                if name and dur_s:
                    sums[name] = sums.get(name, 0.0) + dur_s
                    counts[name] = counts.get(name, 0) + 1
                name = None
                dur_s = None
            elif '"pid"' in line:
                name = None
                dur_s = None
    return sums, counts


def trace_scale_to_wall(
    raw_sum: float,
    n_seen: int,
    n_outer: int,
    inner_steps: int,
    sim_wall_s: float,
) -> float | None:
    """Pick a scale that brings the traced sum up to the pass-2 wall.

    A complete trace already matches (scale 1). Pi writes one kernel event per
    timestep, so a short trace scales by ``ntimes / events``. RT+OF writes
    ``inner_steps`` events per outer rep. The chosen scale must land within
    1% under the wall (unnamed idle) and not above it.
    """
    if sim_wall_s <= 0 or n_seen <= 0 or raw_sum <= 0:
        return None
    candidates = [1.0]
    if n_outer > 0:
        candidates.append(n_outer / n_seen)
        candidates.append(n_outer * max(inner_steps, 1) / n_seen)
    best: float | None = None
    best_abs = None
    for scale in dict.fromkeys(candidates):
        gap = (sim_wall_s - raw_sum * scale) / sim_wall_s
        if gap < -1e-3 or gap > 0.01:
            continue
        if best_abs is None or abs(gap) < best_abs:
            best = scale
            best_abs = abs(gap)
    if best is not None:
        return best
    # A complete trace can still sit under the wall: idle-wave delay is not a
    # kernel event. Keep scale 1 and leave that idle out of both kernel times.
    expected = n_outer * max(inner_steps, 1) if n_outer > 0 else 0
    if n_seen in (n_outer, expected):
        gap = (sim_wall_s - raw_sum) / sim_wall_s
        if gap >= -1e-3:
            return 1.0
    return None


def find_measured_out(measured_dir: Path, config: str) -> Path | None:
    """Resolve ``.out`` next to a TSV config (flat file or config subdirectory)."""
    if not measured_dir.is_dir() or not config:
        return None
    names = [
        measured_dir / f"{config}.out",
        measured_dir / config / f"{config}.out",
        measured_dir / config / "icx.out",
    ]
    plat = re.match(r"^(icx-socket|icx-tpn1)(?:-\d+)?$", config)
    if plat:
        names.append(measured_dir / plat.group(1) / f"{config}.out")
        names.append(measured_dir / plat.group(1) / f"{plat.group(1)}.out")
    if config == "icx":
        names.append(measured_dir / "icx" / "icx.out")
    seen: set[Path] = set()
    for path in names:
        if path in seen:
            continue
        seen.add(path)
        if path.is_file() and path.stat().st_size > 20:
            return path
    hits = sorted(p for p in measured_dir.rglob(f"{config}.out") if p.is_file())
    for path in hits:
        if "slurm" in path.name:
            continue
        if path.stat().st_size > 20:
            return path
    return None


def parse_pi_kernel_times(out: Path | None) -> dict[str, float]:
    """Slow rank = max sum of TimePerReps; accumulate matching TriadTime/PiTime.

    Expects 6-col CSV: Rank,NITER,TriadTime,PiTime,TimePerReps,Performance.
    """
    empty = {"triad_s": 0.0, "pi_s": 0.0, "wall_s": 0.0, "nsteps": 0, "rank": -1.0}
    if out is None or not out.is_file():
        return empty
    by_rank: dict[int, dict[str, float]] = {}
    nsteps_by_rank: dict[int, int] = {}
    with out.open(errors="replace") as fh:
        for line in fh:
            m = re.match(
                r"^(\d+),(\d+),([0-9.eE+-]+),([0-9.eE+-]+),([0-9.eE+-]+),",
                line.strip(),
            )
            if not m:
                continue
            rank = int(m.group(1))
            rec = by_rank.setdefault(rank, {"triad_s": 0.0, "pi_s": 0.0, "wall_s": 0.0})
            rec["triad_s"] += float(m.group(3))
            rec["pi_s"] += float(m.group(4))
            rec["wall_s"] += float(m.group(5))
            nsteps_by_rank[rank] = nsteps_by_rank.get(rank, 0) + 1
    if not by_rank:
        return empty
    slow = max(by_rank, key=lambda r: by_rank[r]["wall_s"])
    rec = by_rank[slow]
    return {
        "triad_s": rec["triad_s"],
        "pi_s": rec["pi_s"],
        "wall_s": rec["wall_s"],
        "nsteps": float(nsteps_by_rank.get(slow, 0)),
        "rank": float(slow),
    }


def parse_rtof_timers(out: Path | None) -> dict[str, float]:
    """Timer ray-tracing / Timer optical-flow totals (seconds) on the wall-slow rank."""
    empty = {"rt_s": 0.0, "of_s": 0.0, "wall_s": 0.0, "rank": -1.0}
    if out is None or not out.is_file():
        return empty
    wall_by_rank: dict[int, float] = {}
    rt_ms: dict[int, float] = {}
    of_ms: dict[int, float] = {}
    timer_re = re.compile(
        r"^Timer\s+(ray-tracing|optical-flow)\s+on rank\s+(\d+)\s*:\s+([\d.]+)\s*ms",
        re.I,
    )
    with out.open(errors="replace") as fh:
        for line in fh:
            s = line.strip()
            t = timer_re.match(s)
            if t:
                rank = int(t.group(2))
                ms = float(t.group(3))
                if t.group(1).lower() == "ray-tracing":
                    rt_ms[rank] = ms
                else:
                    of_ms[rank] = ms
                continue
            m6 = re.match(r"^(\d+),(\d+),([0-9.eE+-]+),([0-9.eE+-]+),([0-9.eE+-]+),", s)
            m4 = re.match(r"^(\d+),(\d+),([0-9.eE+-]+),", s)
            if m6:
                rank = int(m6.group(1))
                wall_by_rank[rank] = wall_by_rank.get(rank, 0.0) + float(m6.group(5))
            elif m4:
                rank = int(m4.group(1))
                wall_by_rank[rank] = wall_by_rank.get(rank, 0.0) + float(m4.group(3))
    if wall_by_rank:
        slow = max(wall_by_rank, key=wall_by_rank.get)
        wall_s = wall_by_rank[slow]
    elif rt_ms or of_ms:
        keys = set(rt_ms) | set(of_ms)
        slow = max(keys, key=lambda r: rt_ms.get(r, 0.0) + of_ms.get(r, 0.0))
        wall_s = 0.0
    else:
        return empty
    rt_s = rt_ms.get(slow, max(rt_ms.values()) if rt_ms else 0.0) / 1000.0
    of_s = of_ms.get(slow, max(of_ms.values()) if of_ms else 0.0) / 1000.0
    return {"rt_s": rt_s, "of_s": of_s, "wall_s": wall_s, "rank": float(slow)}
