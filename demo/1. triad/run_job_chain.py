#!/usr/bin/env python3
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: Submit DisCostiC validation jobs sequentially; advance on COMPLETED and halt on failure.

from __future__ import annotations

import argparse
import os
import re
import subprocess
import sys
import time
from collections.abc import Callable
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path

TC = Path(__file__).resolve().parent                                          # 1-triad/
ROOT = Path(os.environ.get("DISCOSTIC_EXTENSION_ROOT", str(TC.parent.parent)))  # DisCostiC-Sim
BENCH_BIN = TC / "shared/benchmarks/mpi_stream_bench_comm"

# Still in queue or on nodes — keep polling.
ACTIVE_STATES = frozenset(
    {
        "PENDING",
        "RUNNING",
        "CONFIGURING",
        "COMPLETING",
        "SUSPENDED",
        "REQUEUED",
        "PREEMPTED",
        "SPECIAL_EXIT",
    }
)

SUCCESS_STATES = frozenset({"COMPLETED"})


@dataclass(frozen=True)
class Step:
    label: str
    script: Path | None = None  # None => local shell command in `command`
    command: str | None = None
    nodes: int = 0
    ntasks: int = 0
    mem_gb: int = 0
    time_h: float = 0.0


# Ordered lightest → heaviest: nodes, ntasks, mem, time limit; shorter iter counts first.
STEPS: list[Step] = [
    Step("Pre-build benchmark", command=str(TC / "shared/scripts/build_benchmark_all.sh")),
    # singlenode · 1 node · 36 tasks · 60G
    Step("S1 measured ICX socket", TC / "stage1-ecm-modeling/jobs/measured/icx.sh", nodes=1, ntasks=36, mem_gb=60, time_h=0.17),
    Step("S2 measured ICX socket", TC / "stage2-single-timestep/jobs/measured/icx-socket.sh", nodes=1, ntasks=36, mem_gb=60, time_h=0.17),
    Step("S2 sim ICX socket", TC / "stage2-single-timestep/jobs/simulated/icx-socket-100.sh", nodes=1, ntasks=36, mem_gb=60, time_h=0.17),
    Step("S3 sim ICX socket 100", TC / "stage3-multi-timestep/jobs/simulated/icx-socket-100.sh", nodes=1, ntasks=36, mem_gb=60, time_h=0.17),
    Step("S3 sim ICX socket 1000", TC / "stage3-multi-timestep/jobs/simulated/icx-socket-1000.sh", nodes=1, ntasks=36, mem_gb=60, time_h=0.17),
    Step("S3 sim ICX socket 10000", TC / "stage3-multi-timestep/jobs/simulated/icx-socket-10000.sh", nodes=1, ntasks=36, mem_gb=60, time_h=0.17),
    Step("S3 measured ICX socket", TC / "stage3-multi-timestep/jobs/measured/icx-socket.sh", nodes=1, ntasks=36, mem_gb=60, time_h=1.05),
    # spr1tb · 1 node · 52 tasks · 60G
    Step("S1 measured SPR socket", TC / "stage1-ecm-modeling/jobs/measured/spr.sh", nodes=1, ntasks=52, mem_gb=60, time_h=0.17),
    Step("S2 sim SPR socket", TC / "stage2-single-timestep/jobs/simulated/spr-socket-100.sh", nodes=1, ntasks=52, mem_gb=60, time_h=0.17),
    Step("S2 measured SPR", TC / "stage2-single-timestep/jobs/measured/spr.sh", nodes=1, ntasks=52, mem_gb=60, time_h=0.17),
    Step("S3 sim SPR socket 100", TC / "stage3-multi-timestep/jobs/simulated/spr-socket-100.sh", nodes=1, ntasks=52, mem_gb=60, time_h=0.17),
    Step("S3 sim SPR socket 1000", TC / "stage3-multi-timestep/jobs/simulated/spr-socket-1000.sh", nodes=1, ntasks=52, mem_gb=60, time_h=0.17),
    Step("S3 sim SPR socket 10000", TC / "stage3-multi-timestep/jobs/simulated/spr-socket-10000.sh", nodes=1, ntasks=52, mem_gb=60, time_h=0.33),
    Step("S3 measured SPR", TC / "stage3-multi-timestep/jobs/measured/spr.sh", nodes=1, ntasks=52, mem_gb=60, time_h=0.75),
    # multinode · 36 nodes · 36 tasks · 10G
    Step("S1 measured ICX tpn1", TC / "stage1-ecm-modeling/jobs/measured/icx-tpn1.sh", nodes=36, ntasks=36, mem_gb=10, time_h=0.17),
    Step("S2 measured ICX tpn1", TC / "stage2-single-timestep/jobs/measured/icx-tpn1.sh", nodes=36, ntasks=36, mem_gb=10, time_h=0.17),
    Step("S2 sim ICX tpn1", TC / "stage2-single-timestep/jobs/simulated/icx-tpn1-100.sh", nodes=36, ntasks=36, mem_gb=10, time_h=0.25),
    Step("S3 sim ICX tpn1 100", TC / "stage3-multi-timestep/jobs/simulated/icx-tpn1-100.sh", nodes=36, ntasks=36, mem_gb=10, time_h=0.25),
    Step("S3 sim ICX tpn1 1000", TC / "stage3-multi-timestep/jobs/simulated/icx-tpn1-1000.sh", nodes=36, ntasks=36, mem_gb=10, time_h=0.25),
    Step("S3 sim ICX tpn1 10000", TC / "stage3-multi-timestep/jobs/simulated/icx-tpn1-10000.sh", nodes=36, ntasks=36, mem_gb=10, time_h=0.25),
    Step("S3 measured ICX tpn1", TC / "stage3-multi-timestep/jobs/measured/icx-tpn1.sh", nodes=36, ntasks=36, mem_gb=10, time_h=0.3),

    # --- O1 case study (stages 2 and 3, compiled with -O1) ---
    Step("S2-O1 measured ICX socket", TC / "stage2-single-timestep/jobs/measured-o1/icx-socket.sh", nodes=1, ntasks=36, mem_gb=60, time_h=0.17),
    Step("S2-O1 sim ICX socket", TC / "stage2-single-timestep/jobs/simulated-o1/icx-socket-100.sh", nodes=1, ntasks=36, mem_gb=60, time_h=0.20),
    Step("S2-O1 sim SPR socket", TC / "stage2-single-timestep/jobs/simulated-o1/spr-socket-100.sh", nodes=1, ntasks=52, mem_gb=60, time_h=0.30),
    Step("S2-O1 measured SPR", TC / "stage2-single-timestep/jobs/measured-o1/spr.sh", nodes=1, ntasks=52, mem_gb=60, time_h=0.17),
    Step("S2-O1 measured ICX tpn1", TC / "stage2-single-timestep/jobs/measured-o1/icx-tpn1.sh", nodes=36, ntasks=36, mem_gb=10, time_h=0.17),
    Step("S2-O1 sim ICX tpn1", TC / "stage2-single-timestep/jobs/simulated-o1/icx-tpn1-100.sh", nodes=36, ntasks=36, mem_gb=10, time_h=0.25),
    Step("S3-O1 sim ICX socket 100", TC / "stage3-multi-timestep/jobs/simulated-o1/icx-socket-100.sh", nodes=1, ntasks=36, mem_gb=60, time_h=0.20),
    Step("S3-O1 sim ICX socket 1000", TC / "stage3-multi-timestep/jobs/simulated-o1/icx-socket-1000.sh", nodes=1, ntasks=36, mem_gb=60, time_h=0.20),
    Step("S3-O1 sim ICX socket 10000", TC / "stage3-multi-timestep/jobs/simulated-o1/icx-socket-10000.sh", nodes=1, ntasks=36, mem_gb=60, time_h=0.25),
    Step("S3-O1 measured ICX socket", TC / "stage3-multi-timestep/jobs/measured-o1/icx-socket.sh", nodes=1, ntasks=36, mem_gb=60, time_h=1.08),
    Step("S3-O1 sim SPR socket 100", TC / "stage3-multi-timestep/jobs/simulated-o1/spr-socket-100.sh", nodes=1, ntasks=52, mem_gb=60, time_h=0.30),
    Step("S3-O1 sim SPR socket 1000", TC / "stage3-multi-timestep/jobs/simulated-o1/spr-socket-1000.sh", nodes=1, ntasks=52, mem_gb=60, time_h=0.30),
    Step("S3-O1 sim SPR socket 10000", TC / "stage3-multi-timestep/jobs/simulated-o1/spr-socket-10000.sh", nodes=1, ntasks=52, mem_gb=60, time_h=0.33),
    Step("S3-O1 measured SPR", TC / "stage3-multi-timestep/jobs/measured-o1/spr.sh", nodes=1, ntasks=52, mem_gb=60, time_h=0.7),
    Step("S3-O1 sim ICX tpn1 100", TC / "stage3-multi-timestep/jobs/simulated-o1/icx-tpn1-100.sh", nodes=36, ntasks=36, mem_gb=10, time_h=0.25),
    Step("S3-O1 sim ICX tpn1 1000", TC / "stage3-multi-timestep/jobs/simulated-o1/icx-tpn1-1000.sh", nodes=36, ntasks=36, mem_gb=10, time_h=0.25),
    Step("S3-O1 sim ICX tpn1 10000", TC / "stage3-multi-timestep/jobs/simulated-o1/icx-tpn1-10000.sh", nodes=36, ntasks=36, mem_gb=10, time_h=0.25),
    Step("S3-O1 measured ICX tpn1", TC / "stage3-multi-timestep/jobs/measured-o1/icx-tpn1.sh", nodes=36, ntasks=36, mem_gb=10, time_h=0.28),

    # --- Stage 4 idlewave (measured + sim → ITAC STF) ---
    Step("S4 measured ICX", TC / "stage4-idlewave/jobs/measured/icx.sh", nodes=1, ntasks=72, mem_gb=130, time_h=0.17),
    Step("S4 sim ICX idlewave", TC / "stage4-idlewave/jobs/simulated/icx-idlewave.sh", nodes=1, ntasks=72, mem_gb=130, time_h=0.28),
    Step("S4 measured-trace ICX", TC / "stage4-idlewave/jobs/measured-trace/icx.sh", nodes=1, ntasks=72, mem_gb=130, time_h=0.33),
    Step("S4 measured SPR", TC / "stage4-idlewave/jobs/measured/spr.sh", nodes=1, ntasks=104, mem_gb=130, time_h=0.17),
    Step("S4 sim SPR idlewave", TC / "stage4-idlewave/jobs/simulated/spr-idlewave.sh", nodes=1, ntasks=104, mem_gb=130, time_h=0.45),
    Step("S4 measured-trace SPR", TC / "stage4-idlewave/jobs/measured-trace/spr.sh", nodes=1, ntasks=104, mem_gb=130, time_h=0.33),

    # --- Stage 5 scaling (ICX 1–32; SPR 1–8 only — spr1tb MaxNodes=8) ---
    Step("S5 measured strong-icx-1node", TC / "stage5-scaling/jobs/measured/strong-icx-1node.sh", nodes=1, ntasks=72, mem_gb=130, time_h=1.1),
    Step("S5 sim strong-icx-1node", TC / "stage5-scaling/jobs/simulated/strong-icx-1node.sh", nodes=1, ntasks=72, mem_gb=130, time_h=0.22),
    Step("S5 measured weak-icx-1node", TC / "stage5-scaling/jobs/measured/weak-icx-1node.sh", nodes=1, ntasks=72, mem_gb=130, time_h=1.1),
    Step("S5 sim weak-icx-1node", TC / "stage5-scaling/jobs/simulated/weak-icx-1node.sh", nodes=1, ntasks=72, mem_gb=130, time_h=0.22),
    Step("S5 measured strong-spr-1node", TC / "stage5-scaling/jobs/measured/strong-spr-1node.sh", nodes=1, ntasks=104, mem_gb=200, time_h=0.8),
    Step("S5 sim strong-spr-1node", TC / "stage5-scaling/jobs/simulated/strong-spr-1node.sh", nodes=1, ntasks=104, mem_gb=130, time_h=0.33),
    Step("S5 measured weak-spr-1node", TC / "stage5-scaling/jobs/measured/weak-spr-1node.sh", nodes=1, ntasks=104, mem_gb=200, time_h=0.8),
    Step("S5 sim weak-spr-1node", TC / "stage5-scaling/jobs/simulated/weak-spr-1node.sh", nodes=1, ntasks=104, mem_gb=130, time_h=0.33),
    Step("S5 measured strong-icx-2node", TC / "stage5-scaling/jobs/measured/strong-icx-2node.sh", nodes=2, ntasks=144, mem_gb=130, time_h=0.55),
    Step("S5 sim strong-icx-2node", TC / "stage5-scaling/jobs/simulated/strong-icx-2node.sh", nodes=2, ntasks=144, mem_gb=130, time_h=0.47),
    Step("S5 measured weak-icx-2node", TC / "stage5-scaling/jobs/measured/weak-icx-2node.sh", nodes=2, ntasks=144, mem_gb=130, time_h=1.1),
    Step("S5 sim weak-icx-2node", TC / "stage5-scaling/jobs/simulated/weak-icx-2node.sh", nodes=2, ntasks=144, mem_gb=130, time_h=0.47),
    Step("S5 measured strong-spr-2node", TC / "stage5-scaling/jobs/measured/strong-spr-2node.sh", nodes=2, ntasks=208, mem_gb=130, time_h=0.43),
    Step("S5 sim strong-spr-2node", TC / "stage5-scaling/jobs/simulated/strong-spr-2node.sh", nodes=2, ntasks=208, mem_gb=130, time_h=0.77),
    Step("S5 measured weak-spr-2node", TC / "stage5-scaling/jobs/measured/weak-spr-2node.sh", nodes=2, ntasks=208, mem_gb=200, time_h=0.8),
    Step("S5 sim weak-spr-2node", TC / "stage5-scaling/jobs/simulated/weak-spr-2node.sh", nodes=2, ntasks=208, mem_gb=130, time_h=0.75),
    Step("S5 measured strong-icx-4node", TC / "stage5-scaling/jobs/measured/strong-icx-4node.sh", nodes=4, ntasks=288, mem_gb=130, time_h=0.28),
    Step("S5 sim strong-icx-4node", TC / "stage5-scaling/jobs/simulated/strong-icx-4node.sh", nodes=4, ntasks=288, mem_gb=130, time_h=1.17),
    Step("S5 measured weak-icx-4node", TC / "stage5-scaling/jobs/measured/weak-icx-4node.sh", nodes=4, ntasks=288, mem_gb=130, time_h=1.1),
    Step("S5 sim weak-icx-4node", TC / "stage5-scaling/jobs/simulated/weak-icx-4node.sh", nodes=4, ntasks=288, mem_gb=130, time_h=1.17),
    Step("S5 measured strong-spr-4node", TC / "stage5-scaling/jobs/measured/strong-spr-4node.sh", nodes=4, ntasks=416, mem_gb=130, time_h=0.22),
    Step("S5 sim strong-spr-4node", TC / "stage5-scaling/jobs/simulated/strong-spr-4node.sh", nodes=4, ntasks=416, mem_gb=130, time_h=2.0),
    Step("S5 measured weak-spr-4node", TC / "stage5-scaling/jobs/measured/weak-spr-4node.sh", nodes=4, ntasks=416, mem_gb=200, time_h=0.8),
    Step("S5 sim weak-spr-4node", TC / "stage5-scaling/jobs/simulated/weak-spr-4node.sh", nodes=4, ntasks=416, mem_gb=130, time_h=1.95),
    Step("S5 measured strong-icx-8node", TC / "stage5-scaling/jobs/measured/strong-icx-8node.sh", nodes=8, ntasks=576, mem_gb=130, time_h=0.17),
    Step("S5 sim strong-icx-8node", TC / "stage5-scaling/jobs/simulated/strong-icx-8node.sh", nodes=8, ntasks=576, mem_gb=130, time_h=3.4),
    Step("S5 measured weak-icx-8node", TC / "stage5-scaling/jobs/measured/weak-icx-8node.sh", nodes=8, ntasks=576, mem_gb=130, time_h=1.1),
    Step("S5 sim weak-icx-8node", TC / "stage5-scaling/jobs/simulated/weak-icx-8node.sh", nodes=8, ntasks=576, mem_gb=130, time_h=3.38),
    Step("S5 measured strong-spr-8node", TC / "stage5-scaling/jobs/measured/strong-spr-8node.sh", nodes=8, ntasks=832, mem_gb=130, time_h=0.17),
    Step("S5 sim strong-spr-8node", TC / "stage5-scaling/jobs/simulated/strong-spr-8node.sh", nodes=8, ntasks=832, mem_gb=130, time_h=1.0),
    Step("S5 measured weak-spr-8node", TC / "stage5-scaling/jobs/measured/weak-spr-8node.sh", nodes=8, ntasks=832, mem_gb=200, time_h=0.8),
    Step("S5 sim weak-spr-8node", TC / "stage5-scaling/jobs/simulated/weak-spr-8node.sh", nodes=8, ntasks=832, mem_gb=130, time_h=1.0),
    Step("S5 measured strong-icx-16node", TC / "stage5-scaling/jobs/measured/strong-icx-16node.sh", nodes=16, ntasks=1152, mem_gb=130, time_h=0.17),
    Step("S5 sim strong-icx-16node", TC / "stage5-scaling/jobs/simulated/strong-icx-16node.sh", nodes=16, ntasks=1152, mem_gb=130, time_h=1.0),
    Step("S5 measured weak-icx-16node", TC / "stage5-scaling/jobs/measured/weak-icx-16node.sh", nodes=16, ntasks=1152, mem_gb=130, time_h=1.1),
    Step("S5 sim weak-icx-16node", TC / "stage5-scaling/jobs/simulated/weak-icx-16node.sh", nodes=16, ntasks=1152, mem_gb=130, time_h=1.0),
    Step("S5 measured strong-icx-32node", TC / "stage5-scaling/jobs/measured/strong-icx-32node.sh", nodes=32, ntasks=2304, mem_gb=130, time_h=0.17),
    Step("S5 sim strong-icx-32node", TC / "stage5-scaling/jobs/simulated/strong-icx-32node.sh", nodes=32, ntasks=2304, mem_gb=130, time_h=2.0),
    Step("S5 measured weak-icx-32node", TC / "stage5-scaling/jobs/measured/weak-icx-32node.sh", nodes=32, ntasks=2304, mem_gb=130, time_h=1.1),
    Step("S5 sim weak-icx-32node", TC / "stage5-scaling/jobs/simulated/weak-icx-32node.sh", nodes=32, ntasks=2304, mem_gb=130, time_h=2.0),
]


def _bench_out_ok(path: Path, ntimes: int | None = None) -> bool:
    if not path.is_file() or path.stat().st_size < 20:
        return False
    text = path.read_text(errors="replace")
    if "wall_seconds=" not in text or "check=" not in text:
        return False
    if ntimes is not None and f"NTIMES={ntimes}" not in text:
        return False
    return True


def _tsv_has_config(tsv: Path, config: str) -> bool:
    if not tsv.is_file():
        return False
    for line in tsv.read_text(errors="replace").splitlines()[1:]:
        if line.startswith(f"{config}\t"):
            return True
    return False


def _stf_ok(path: Path) -> bool:
    return path.is_file() and path.stat().st_size >= 50


def _sim_ok(sim_dir: Path, base: str, *, require_stf: bool = False) -> bool:
    csv = sim_dir / f"{base}.csv"
    dms = sim_dir / f"{base}.dms"
    pass1 = sim_dir / "slurm-discostic-pass1.out"
    if not csv.is_file() or csv.stat().st_size < 50:
        return False
    if not dms.is_file() or dms.stat().st_size < 50:
        return False
    if require_stf:
        stf = sim_dir / f"{base}.single.stf"
        if not stf.is_file() or stf.stat().st_size < 50:
            return False
    if pass1.is_file():
        text = pass1.read_text(errors="replace")
        if "Traceback" in text or "Exiting from Simulation" in text:
            return False
    return True


def _verify_s5_measured(name: str) -> bool:
    out = TC / "stage5-scaling/results/measured"
    return _bench_out_ok(out / name / f"{name}.out", 5000) and _tsv_has_config(
        out / "stage5-measured.tsv", name
    )


def _verify_s4_measured(arch: str) -> bool:
    """Accept legacy Chrome DMS (stream-triad-actual-*) or new bench .out + TSV."""
    out = TC / "stage4-idlewave/results/measured" / arch
    legacy = {
        "icx": out / "stream-triad-actual-icx.dms",
        "spr": out / "stream-triad-actual-spr.dms",
    }[arch]
    if legacy.is_file() and legacy.stat().st_size > 50:
        return True
    return _bench_out_ok(out / f"{arch}.out", 45) and _tsv_has_config(
        TC / "stage4-idlewave/results/measured/stage4-measured.tsv", arch
    )


def _verify_prebuild() -> bool:
    o1 = BENCH_BIN.parent / "mpi_stream_bench_comm_O1"
    return (
        BENCH_BIN.is_file() and os.access(BENCH_BIN, os.X_OK)
        and o1.is_file() and os.access(o1, os.X_OK)
    )


def _verify_s3_measured_icx_socket() -> bool:
    out = TC / "stage3-multi-timestep/results/measured"
    tsv = out / "stage3-measured.tsv"
    for iters in (100, 1000, 10000):
        if not _bench_out_ok(out / f"icx-socket-{iters}.out", iters):
            return False
        if not _tsv_has_config(tsv, f"icx-socket-{iters}"):
            return False
    return True


def _verify_s3_measured_spr() -> bool:
    out = TC / "stage3-multi-timestep/results/measured"
    tsv = out / "stage3-measured.tsv"
    for iters in (100, 1000, 10000):
        if not _bench_out_ok(out / f"spr-socket-{iters}.out", iters):
            return False
        if not _tsv_has_config(tsv, f"spr-socket-{iters}"):
            return False
    return True


def _verify_s3_measured_icx_tpn1() -> bool:
    out = TC / "stage3-multi-timestep/results/measured"
    tsv = out / "stage3-measured.tsv"
    for iters in (100, 1000, 10000):
        if not _bench_out_ok(out / f"icx-tpn1-{iters}.out", iters):
            return False
        if not _tsv_has_config(tsv, f"icx-tpn1-{iters}"):
            return False
    return True


def _verify_s5(name: str) -> bool:
    """Verify Stage 5 scaling sim outputs (CSV + DMS + STF + clean pass1)."""
    return _sim_ok(
        TC / f"stage5-scaling/results/simulated/{name}",
        f"stream-triad-{name}",
    )


VERIFY: dict[str, Callable[[], bool]] = {
    "Pre-build benchmark": _verify_prebuild,
    "S1 measured ICX socket": lambda: _bench_out_ok(
        TC / "stage1-ecm-modeling/results/measured/icx-socket-100.out", 100
    )
    and _tsv_has_config(
        TC / "stage1-ecm-modeling/results/measured/stage1-measured.tsv", "icx-socket-100"
    ),
    "S2 measured ICX socket": lambda: _bench_out_ok(
        TC / "stage2-single-timestep/results/measured/icx-socket-100.out", 100
    )
    and _tsv_has_config(
        TC / "stage2-single-timestep/results/measured/stage2-measured-100.tsv", "icx-socket-100"
    ),
    "S2 sim ICX socket": lambda: _sim_ok(
        TC / "stage2-single-timestep/results/simulated/icx-socket-100", "icx-socket-100",
        require_stf=True,
    ),
    "S3 sim ICX socket 100": lambda: _sim_ok(
        TC / "stage3-multi-timestep/results/simulated/icx-socket-100", "icx-socket-100",
        require_stf=True,
    ),
    "S3 sim ICX socket 1000": lambda: _sim_ok(
        TC / "stage3-multi-timestep/results/simulated/icx-socket-1000", "icx-socket-1000",
        require_stf=True,
    ),
    "S3 sim ICX socket 10000": lambda: _sim_ok(
        TC / "stage3-multi-timestep/results/simulated/icx-socket-10000", "icx-socket-10000",
        require_stf=True,
    ),
    "S3 measured ICX socket": _verify_s3_measured_icx_socket,
    "S1 measured SPR socket": lambda: _bench_out_ok(
        TC / "stage1-ecm-modeling/results/measured/spr-socket-100.out", 100
    ),
    "S2 sim SPR socket": lambda: _sim_ok(
        TC / "stage2-single-timestep/results/simulated/spr-socket-100", "spr-socket-100",
        require_stf=True,
    ),
    "S2 measured SPR": lambda: _bench_out_ok(
        TC / "stage2-single-timestep/results/measured/spr-socket-100.out", 100
    )
    and _tsv_has_config(
        TC / "stage2-single-timestep/results/measured/stage2-measured-100.tsv", "spr-socket-100"
    ),
    "S3 sim SPR socket 100": lambda: _sim_ok(
        TC / "stage3-multi-timestep/results/simulated/spr-socket-100", "spr-socket-100",
        require_stf=True,
    ),
    "S3 sim SPR socket 1000": lambda: _sim_ok(
        TC / "stage3-multi-timestep/results/simulated/spr-socket-1000", "spr-socket-1000",
        require_stf=True,
    ),
    "S3 sim SPR socket 10000": lambda: _sim_ok(
        TC / "stage3-multi-timestep/results/simulated/spr-socket-10000", "spr-socket-10000",
        require_stf=True,
    ),
    "S3 measured SPR": _verify_s3_measured_spr,
    "S1 measured ICX tpn1": lambda: _bench_out_ok(
        TC / "stage1-ecm-modeling/results/measured/icx-tpn1-100.out", 100
    ),
    "S2 measured ICX tpn1": lambda: _bench_out_ok(
        TC / "stage2-single-timestep/results/measured/icx-tpn1-100.out", 100
    )
    and _tsv_has_config(
        TC / "stage2-single-timestep/results/measured/stage2-measured-100.tsv", "icx-tpn1-100"
    ),
    "S2 sim ICX tpn1": lambda: _sim_ok(
        TC / "stage2-single-timestep/results/simulated/icx-tpn1-100", "icx-tpn1-100",
        require_stf=True,
    ),
    "S3 sim ICX tpn1 100": lambda: _sim_ok(
        TC / "stage3-multi-timestep/results/simulated/icx-tpn1-100", "icx-tpn1-100",
        require_stf=True,
    ),
    "S3 sim ICX tpn1 1000": lambda: _sim_ok(
        TC / "stage3-multi-timestep/results/simulated/icx-tpn1-1000", "icx-tpn1-1000",
        require_stf=True,
    ),
    "S3 sim ICX tpn1 10000": lambda: _sim_ok(
        TC / "stage3-multi-timestep/results/simulated/icx-tpn1-10000", "icx-tpn1-10000",
        require_stf=True,
    ),
    "S3 measured ICX tpn1": _verify_s3_measured_icx_tpn1,
    "S2-O1 measured ICX socket": lambda: _bench_out_ok(
        TC / "stage2-single-timestep/results/measured-o1/icx-socket-100.out", 100
    )
    and _tsv_has_config(
        TC / "stage2-single-timestep/results/measured-o1/stage2-measured-100.tsv", "icx-socket-100"
    ),
    "S2-O1 sim ICX socket": lambda: _sim_ok(
        TC / "stage2-single-timestep/results/simulated-o1/icx-socket-100", "icx-socket-100-O1",
        require_stf=True,
    ),
    "S2-O1 sim SPR socket": lambda: _sim_ok(
        TC / "stage2-single-timestep/results/simulated-o1/spr-socket-100", "spr-socket-100-O1",
        require_stf=True,
    ),
    "S2-O1 measured SPR": lambda: _bench_out_ok(
        TC / "stage2-single-timestep/results/measured-o1/spr-socket-100.out", 100
    )
    and _tsv_has_config(
        TC / "stage2-single-timestep/results/measured-o1/stage2-measured-100.tsv", "spr-socket-100"
    ),
    "S2-O1 measured ICX tpn1": lambda: _bench_out_ok(
        TC / "stage2-single-timestep/results/measured-o1/icx-tpn1-100.out", 100
    )
    and _tsv_has_config(
        TC / "stage2-single-timestep/results/measured-o1/stage2-measured-100.tsv", "icx-tpn1-100"
    ),
    "S2-O1 sim ICX tpn1": lambda: _sim_ok(
        TC / "stage2-single-timestep/results/simulated-o1/icx-tpn1-100", "icx-tpn1-100-O1",
        require_stf=True,
    ),
    "S3-O1 sim ICX socket 100": lambda: _sim_ok(
        TC / "stage3-multi-timestep/results/simulated-o1/icx-socket-100", "icx-socket-100-O1",
        require_stf=True,
    ),
    "S3-O1 sim ICX socket 1000": lambda: _sim_ok(
        TC / "stage3-multi-timestep/results/simulated-o1/icx-socket-1000", "icx-socket-1000-O1",
        require_stf=True,
    ),
    "S3-O1 sim ICX socket 10000": lambda: _sim_ok(
        TC / "stage3-multi-timestep/results/simulated-o1/icx-socket-10000", "icx-socket-10000-O1",
        require_stf=True,
    ),
    "S3-O1 measured ICX socket": lambda: all(
        _bench_out_ok(TC / f"stage3-multi-timestep/results/measured-o1/icx-socket-{n}.out", n)
        for n in (100, 1000, 10000)
    ),
    "S3-O1 sim SPR socket 100": lambda: _sim_ok(
        TC / "stage3-multi-timestep/results/simulated-o1/spr-socket-100", "spr-socket-100-O1",
        require_stf=True,
    ),
    "S3-O1 sim SPR socket 1000": lambda: _sim_ok(
        TC / "stage3-multi-timestep/results/simulated-o1/spr-socket-1000", "spr-socket-1000-O1",
        require_stf=True,
    ),
    "S3-O1 sim SPR socket 10000": lambda: _sim_ok(
        TC / "stage3-multi-timestep/results/simulated-o1/spr-socket-10000", "spr-socket-10000-O1",
        require_stf=True,
    ),
    "S3-O1 measured SPR": lambda: all(
        _bench_out_ok(TC / f"stage3-multi-timestep/results/measured-o1/spr-socket-{n}.out", n)
        for n in (100, 1000, 10000)
    ),
    "S3-O1 sim ICX tpn1 100": lambda: _sim_ok(
        TC / "stage3-multi-timestep/results/simulated-o1/icx-tpn1-100", "icx-tpn1-100-O1",
        require_stf=True,
    ),
    "S3-O1 sim ICX tpn1 1000": lambda: _sim_ok(
        TC / "stage3-multi-timestep/results/simulated-o1/icx-tpn1-1000", "icx-tpn1-1000-O1",
        require_stf=True,
    ),
    "S3-O1 sim ICX tpn1 10000": lambda: _sim_ok(
        TC / "stage3-multi-timestep/results/simulated-o1/icx-tpn1-10000", "icx-tpn1-10000-O1",
        require_stf=True,
    ),
    "S3-O1 measured ICX tpn1": lambda: all(
        _bench_out_ok(TC / f"stage3-multi-timestep/results/measured-o1/icx-tpn1-{n}.out", n)
        for n in (100, 1000, 10000)
    ),
    "S4 measured ICX": lambda: _verify_s4_measured("icx"),
    "S4 sim ICX idlewave": lambda: _sim_ok(
        TC / "stage4-idlewave/results/simulated/icx", "stream-triad-idlewave", require_stf=True
    ),
    "S4 measured SPR": lambda: _verify_s4_measured("spr"),
    "S4 sim SPR idlewave": lambda: _sim_ok(
        TC / "stage4-idlewave/results/simulated/spr", "stream-triad-idlewave-spr", require_stf=True
    ),
    "S4 measured-trace ICX": lambda: _stf_ok(
        TC / "stage4-idlewave/results/measured-trace/icx/stream-triad-idlewave-measured.single.stf"
    ),
    "S4 measured-trace SPR": lambda: _stf_ok(
        TC / "stage4-idlewave/results/measured-trace/spr/stream-triad-idlewave-spr-measured.single.stf"
    ),
    "S5 measured strong-icx-1node": lambda n="strong-icx-1node": _verify_s5_measured(n),
    "S5 measured weak-icx-1node": lambda n="weak-icx-1node": _verify_s5_measured(n),
    "S5 measured strong-spr-1node": lambda n="strong-spr-1node": _verify_s5_measured(n),
    "S5 measured weak-spr-1node": lambda n="weak-spr-1node": _verify_s5_measured(n),
    "S5 measured strong-icx-2node": lambda n="strong-icx-2node": _verify_s5_measured(n),
    "S5 measured weak-icx-2node": lambda n="weak-icx-2node": _verify_s5_measured(n),
    "S5 measured strong-spr-2node": lambda n="strong-spr-2node": _verify_s5_measured(n),
    "S5 measured weak-spr-2node": lambda n="weak-spr-2node": _verify_s5_measured(n),
    "S5 measured strong-icx-4node": lambda n="strong-icx-4node": _verify_s5_measured(n),
    "S5 measured weak-icx-4node": lambda n="weak-icx-4node": _verify_s5_measured(n),
    "S5 measured strong-spr-4node": lambda n="strong-spr-4node": _verify_s5_measured(n),
    "S5 measured weak-spr-4node": lambda n="weak-spr-4node": _verify_s5_measured(n),
    "S5 measured strong-icx-8node": lambda n="strong-icx-8node": _verify_s5_measured(n),
    "S5 measured weak-icx-8node": lambda n="weak-icx-8node": _verify_s5_measured(n),
    "S5 measured strong-spr-8node": lambda n="strong-spr-8node": _verify_s5_measured(n),
    "S5 measured weak-spr-8node": lambda n="weak-spr-8node": _verify_s5_measured(n),
    "S5 measured strong-icx-16node": lambda n="strong-icx-16node": _verify_s5_measured(n),
    "S5 measured weak-icx-16node": lambda n="weak-icx-16node": _verify_s5_measured(n),
    "S5 measured strong-icx-32node": lambda n="strong-icx-32node": _verify_s5_measured(n),
    "S5 measured weak-icx-32node": lambda n="weak-icx-32node": _verify_s5_measured(n),
    "S5 sim strong-icx-1node": lambda n="strong-icx-1node": _verify_s5(n),
    "S5 sim weak-icx-1node": lambda n="weak-icx-1node": _verify_s5(n),
    "S5 sim strong-spr-1node": lambda n="strong-spr-1node": _verify_s5(n),
    "S5 sim weak-spr-1node": lambda n="weak-spr-1node": _verify_s5(n),
    "S5 sim strong-icx-2node": lambda n="strong-icx-2node": _verify_s5(n),
    "S5 sim weak-icx-2node": lambda n="weak-icx-2node": _verify_s5(n),
    "S5 sim strong-spr-2node": lambda n="strong-spr-2node": _verify_s5(n),
    "S5 sim weak-spr-2node": lambda n="weak-spr-2node": _verify_s5(n),
    "S5 sim strong-icx-4node": lambda n="strong-icx-4node": _verify_s5(n),
    "S5 sim weak-icx-4node": lambda n="weak-icx-4node": _verify_s5(n),
    "S5 sim strong-spr-4node": lambda n="strong-spr-4node": _verify_s5(n),
    "S5 sim weak-spr-4node": lambda n="weak-spr-4node": _verify_s5(n),
    "S5 sim strong-icx-8node": lambda n="strong-icx-8node": _verify_s5(n),
    "S5 sim weak-icx-8node": lambda n="weak-icx-8node": _verify_s5(n),
    "S5 sim strong-spr-8node": lambda n="strong-spr-8node": _verify_s5(n),
    "S5 sim weak-spr-8node": lambda n="weak-spr-8node": _verify_s5(n),
    "S5 sim strong-icx-16node": lambda n="strong-icx-16node": _verify_s5(n),
    "S5 sim weak-icx-16node": lambda n="weak-icx-16node": _verify_s5(n),
    "S5 sim strong-icx-32node": lambda n="strong-icx-32node": _verify_s5(n),
    "S5 sim weak-icx-32node": lambda n="weak-icx-32node": _verify_s5(n),
}


def step_verified(step: Step) -> bool:
    check = VERIFY.get(step.label)
    return check() if check else False


def log(msg: str) -> None:
    ts = datetime.now(timezone.utc).astimezone().strftime("%Y-%m-%d %H:%M:%S %Z")
    print(f"[{ts}] {msg}", flush=True)


def run(cmd: list[str], *, cwd: Path = ROOT) -> subprocess.CompletedProcess[str]:
    return subprocess.run(cmd, cwd=cwd, text=True, capture_output=True, check=False)


def submit_batch(script: Path) -> int:
    if not script.is_file():
        raise FileNotFoundError(f"Job script not found: {script}")
    proc = run(["sbatch", str(script)], cwd=TC)
    if proc.returncode != 0:
        raise RuntimeError(
            f"sbatch failed ({proc.returncode}): {proc.stderr.strip() or proc.stdout.strip()}"
        )
    match = re.search(r"Submitted batch job (\d+)", proc.stdout)
    if not match:
        raise RuntimeError(f"Could not parse job id from sbatch output: {proc.stdout.strip()!r}")
    return int(match.group(1))


def run_local(command: str) -> None:
    proc = run(["bash", command])
    if proc.returncode != 0:
        raise RuntimeError(
            f"Command failed ({proc.returncode}): {command}\n"
            f"{proc.stderr.strip() or proc.stdout.strip()}"
        )


def in_squeue(job_id: int) -> bool:
    proc = run(["squeue", "-j", str(job_id), "-h"])
    return proc.returncode == 0 and bool(proc.stdout.strip())


def sacct_main(job_id: int) -> tuple[str, str]:
    proc = run(
        [
            "sacct",
            "-j",
            str(job_id),
            "--format=JobID,State,ExitCode",
            "-P",
            "-n",
            "--noheader",
        ]
    )
    if proc.returncode != 0:
        return "UNKNOWN", "?"

    for line in proc.stdout.splitlines():
        parts = line.split("|")
        if len(parts) >= 3 and parts[0] == str(job_id):
            state = parts[1].split()[0]  # COMPLETED (exit code ...) -> COMPLETED
            return state, parts[2]
    return "UNKNOWN", "?"


def wait_for_job(job_id: int, label: str, poll_interval: int, heartbeat: int) -> None:
    log(f"Waiting for job {job_id} ({label})")
    started = time.monotonic()
    last_heartbeat = started

    while True:
        if in_squeue(job_id):
            now = time.monotonic()
            if now - last_heartbeat >= heartbeat:
                state, exit_code = sacct_main(job_id)
                elapsed = int(now - started)
                log(f"  still active after {elapsed}s (sacct: {state}, exit={exit_code})")
                last_heartbeat = now
            time.sleep(poll_interval)
            continue

        state, exit_code = sacct_main(job_id)
        if state in ACTIVE_STATES:
            time.sleep(poll_interval)
            continue

        if state in SUCCESS_STATES:
            log(f"Job {job_id} COMPLETED (exit={exit_code})")
            return

        raise RuntimeError(
            f"Job {job_id} ({label}) ended with state={state!r}, exit={exit_code}. "
            f"Chain halted."
        )


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Run test-cases SLURM jobs sequentially (one at a time)."
    )
    parser.add_argument(
        "--start",
        type=int,
        default=0,
        metavar="N",
        help="Start at step index N (0 = pre-build benchmark, default: 0).",
    )
    parser.add_argument(
        "--poll-interval",
        type=int,
        default=30,
        help="Seconds between status checks (default: 30).",
    )
    parser.add_argument(
        "--heartbeat",
        type=int,
        default=300,
        help="Seconds between heartbeat log lines while waiting (default: 300).",
    )
    parser.add_argument(
        "--force",
        action="store_true",
        help="Re-run steps even when output artifacts already verify OK.",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Print steps without submitting or waiting.",
    )
    parser.add_argument(
        "--stages",
        type=str,
        default=None,
        metavar="N[,N...]",
        help="Comma-separated stage numbers to run (e.g. '2,3'). Pre-build is always included.",
    )
    parser.add_argument(
        "--match",
        type=str,
        default=None,
        metavar="SUBSTR",
        help="Only steps whose label contains SUBSTR (case-insensitive), e.g. 'icx'.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()

    active_steps: list[Step] = STEPS
    if args.stages:
        stage_nums = {int(s.strip()) for s in args.stages.split(",")}
        active_steps = [
            s for s in STEPS
            if s.label.startswith("Pre-build")
            or any(re.match(rf"^S{n}\b", s.label) for n in stage_nums)
        ]
        log(f"Filtering to stages {sorted(stage_nums)}: {len(active_steps)} steps selected")

    if args.match:
        needle = args.match.lower()
        active_steps = [
            s for s in active_steps
            if s.label.startswith("Pre-build") or needle in s.label.lower()
        ]
        log(f"Filtering to label match {args.match!r}: {len(active_steps)} steps selected")

    if args.start < 0 or args.start >= len(active_steps):
        print(f"Invalid --start {args.start}; must be 0..{len(active_steps) - 1}", file=sys.stderr)
        return 2

    log(f"Job chain: {len(active_steps)} steps, starting at index {args.start}")

    for idx, step in enumerate(active_steps[args.start :], start=args.start):
        log(f"Step {idx + 1}/{len(active_steps)}: {step.label}")

        if not args.force and step_verified(step):
            log("  already verified — skipping")
            continue

        if args.dry_run:
            if step.command:
                log(f"  would run: bash {step.command}")
            elif step.nodes:
                log(
                    f"  would sbatch: {step.script} "
                    f"[{step.nodes}n/{step.ntasks}t/{step.mem_gb}G/{step.time_h}h]"
                )
            else:
                log(f"  would sbatch: {step.script}")
            continue

        if step.command:
            run_local(step.command)
            if not args.force and not step_verified(step):
                raise RuntimeError(f"Step failed verification after run: {step.label}")
            log("  local command finished OK")
            continue

        job_id = submit_batch(step.script)
        log(f"  submitted job {job_id}")
        wait_for_job(job_id, step.label, args.poll_interval, args.heartbeat)
        if not args.force and not step_verified(step):
            for _delay in (5, 10, 15):
                time.sleep(_delay)
                if step_verified(step):
                    break
            else:
                raise RuntimeError(
                    f"Job {job_id} ({step.label}) completed but output verification failed."
                )

    log("All steps completed successfully.")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (RuntimeError, FileNotFoundError) as exc:
        log(f"ERROR: {exc}")
        raise SystemExit(1)
