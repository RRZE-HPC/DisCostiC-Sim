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

TC = Path(__file__).resolve().parent                                          # raytracing-opticalflow-demo/
ROOT = Path(os.environ.get("DISCOSTIC_EXTENSION_ROOT", str(TC.parent.parent)))  # discostic-extension/
BENCH_BIN = TC / "shared/rt-of-bench/bin/ray-tracing-optical-flow-base"

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



# Single-node first, then multinode 2 → 4 → 8, then 36 (tpn1). Skip-if-verified still applies.
STEPS: list[Step] = [
    Step("Pre-build benchmark", command=str(TC / "shared/scripts/build_benchmark_all.sh")),
    Step("S1 measured ICX socket", TC / "stage1-ecm-modeling/jobs/measured/icx.sh", nodes=1, ntasks=36, mem_gb=60, time_h=0.1667),
    Step("S2 measured ICX socket", TC / "stage2-single-timestep/jobs/measured/icx-socket.sh", nodes=1, ntasks=36, mem_gb=60, time_h=0.1667),
    Step("S2 sim ICX socket", TC / "stage2-single-timestep/jobs/simulated/icx-socket-100.sh", nodes=1, ntasks=36, mem_gb=60, time_h=0.1667),
    Step("S3 sim ICX socket 100", TC / "stage3-multi-timestep/jobs/simulated/icx-socket-100.sh", nodes=1, ntasks=36, mem_gb=60, time_h=0.1667),
    Step("S3 sim ICX socket 1000", TC / "stage3-multi-timestep/jobs/simulated/icx-socket-1000.sh", nodes=1, ntasks=36, mem_gb=60, time_h=0.1667),
    Step("S3 sim ICX socket 10000", TC / "stage3-multi-timestep/jobs/simulated/icx-socket-10000.sh", nodes=1, ntasks=36, mem_gb=60, time_h=0.1667),
    Step("S3 measured ICX socket", TC / "stage3-multi-timestep/jobs/measured/icx-socket.sh", nodes=1, ntasks=36, mem_gb=60, time_h=3.2667),
    Step("S4 measured ICX", TC / "stage4-idlewave/jobs/measured/icx.sh", nodes=1, ntasks=72, mem_gb=130, time_h=0.1667),
    Step("S4 measured-trace ICX", TC / "stage4-idlewave/jobs/measured-trace/icx.sh", nodes=1, ntasks=72, mem_gb=130, time_h=0.1667),
    Step("S4 sim ICX idlewave", TC / "stage4-idlewave/jobs/simulated/icx-idlewave.sh", nodes=1, ntasks=72, mem_gb=130, time_h=0.1667),
    Step("S5 measured strong-icx-1node", TC / "stage5-scaling/jobs/measured/strong-icx-1node.sh", nodes=1, ntasks=72, mem_gb=130, time_h=0.1667),
    Step("S5 sim strong-icx-1node", TC / "stage5-scaling/jobs/simulated/strong-icx-1node.sh", nodes=1, ntasks=72, mem_gb=130, time_h=0.1667),
    Step("S5 measured weak-icx-1node", TC / "stage5-scaling/jobs/measured/weak-icx-1node.sh", nodes=1, ntasks=72, mem_gb=130, time_h=0.1667),
    Step("S5 sim weak-icx-1node", TC / "stage5-scaling/jobs/simulated/weak-icx-1node.sh", nodes=1, ntasks=72, mem_gb=130, time_h=0.1667),
    Step("S5 measured strong-icx-2node", TC / "stage5-scaling/jobs/measured/strong-icx-2node.sh", nodes=2, ntasks=144, mem_gb=130, time_h=0.1667),
    Step("S5 sim strong-icx-2node", TC / "stage5-scaling/jobs/simulated/strong-icx-2node.sh", nodes=2, ntasks=144, mem_gb=130, time_h=0.1667),
    Step("S5 measured weak-icx-2node", TC / "stage5-scaling/jobs/measured/weak-icx-2node.sh", nodes=2, ntasks=144, mem_gb=130, time_h=0.1833),
    Step("S5 sim weak-icx-2node", TC / "stage5-scaling/jobs/simulated/weak-icx-2node.sh", nodes=2, ntasks=144, mem_gb=130, time_h=0.1667),
    Step("S5 measured strong-icx-4node", TC / "stage5-scaling/jobs/measured/strong-icx-4node.sh", nodes=4, ntasks=288, mem_gb=130, time_h=0.1667),
    Step("S5 sim strong-icx-4node", TC / "stage5-scaling/jobs/simulated/strong-icx-4node.sh", nodes=4, ntasks=288, mem_gb=130, time_h=0.1667),
    Step("S5 measured weak-icx-4node", TC / "stage5-scaling/jobs/measured/weak-icx-4node.sh", nodes=4, ntasks=288, mem_gb=200, time_h=0.8),
    Step("S5 sim weak-icx-4node", TC / "stage5-scaling/jobs/simulated/weak-icx-4node.sh", nodes=4, ntasks=288, mem_gb=200, time_h=0.1667),
    Step("S5 measured strong-icx-8node", TC / "stage5-scaling/jobs/measured/strong-icx-8node.sh", nodes=8, ntasks=576, mem_gb=130, time_h=0.1667),
    Step("S5 sim strong-icx-8node", TC / "stage5-scaling/jobs/simulated/strong-icx-8node.sh", nodes=8, ntasks=576, mem_gb=130, time_h=0.1667),
    Step("S5 measured weak-icx-8node", TC / "stage5-scaling/jobs/measured/weak-icx-8node.sh", nodes=8, ntasks=576, mem_gb=200, time_h=3.1833),
    Step("S5 sim weak-icx-8node", TC / "stage5-scaling/jobs/simulated/weak-icx-8node.sh", nodes=8, ntasks=576, mem_gb=200, time_h=0.1667),
    Step("S1 measured ICX tpn1", TC / "stage1-ecm-modeling/jobs/measured/icx-tpn1.sh", nodes=36, ntasks=36, mem_gb=10, time_h=0.1667),
    Step("S2 measured ICX tpn1", TC / "stage2-single-timestep/jobs/measured/icx-tpn1.sh", nodes=36, ntasks=36, mem_gb=10, time_h=0.1667),
    Step("S2 sim ICX tpn1", TC / "stage2-single-timestep/jobs/simulated/icx-tpn1-100.sh", nodes=36, ntasks=36, mem_gb=10, time_h=0.1667),
    Step("S3 sim ICX tpn1 100", TC / "stage3-multi-timestep/jobs/simulated/icx-tpn1-100.sh", nodes=36, ntasks=36, mem_gb=10, time_h=0.1667),
    Step("S3 sim ICX tpn1 1000", TC / "stage3-multi-timestep/jobs/simulated/icx-tpn1-1000.sh", nodes=36, ntasks=36, mem_gb=10, time_h=0.1667),
    Step("S3 sim ICX tpn1 10000", TC / "stage3-multi-timestep/jobs/simulated/icx-tpn1-10000.sh", nodes=36, ntasks=36, mem_gb=10, time_h=0.1667),
    Step("S3 measured ICX tpn1", TC / "stage3-multi-timestep/jobs/measured/icx-tpn1.sh", nodes=36, ntasks=36, mem_gb=10, time_h=2.9167),
]



def _bench_out_ok(path: Path, ntimes: int | None = None) -> bool:
    """Accept triad-bench CSV timing rows (Rank,NITER,TimePerReps,...)."""
    if not path.is_file() or path.stat().st_size < 20:
        return False
    text = path.read_text(errors="replace")
    rows = [ln for ln in text.splitlines() if re.match(r"^\d+,\d+,", ln.strip())]
    if len(rows) < 10:
        return False
    if ntimes is not None:
        last = max(int(ln.split(",")[1]) for ln in rows)
        if last + 1 != ntimes:
            return False
    return True


def _tsv_has_config(tsv: Path, config: str) -> bool:
    if not tsv.is_file():
        return False
    for line in tsv.read_text(errors="replace").splitlines()[1:]:
        if line.startswith(f"{config}\t"):
            return True
    return False


def _pass2_ok(sim_dir: Path) -> bool:
    pass2 = sim_dir / "slurm-discostic-pass2.out"
    if not pass2.is_file() or pass2.stat().st_size < 50:
        return False
    text = pass2.read_text(errors="replace")
    if "Traceback" in text or "Exiting from Simulation" in text:
        return False
    return "FULL APPLICATION PERFORMANCE (for all ranks):" in text


def _stf_ok(path: Path) -> bool:
    return path.is_file() and path.stat().st_size >= 50


def _sim_ok(sim_dir: Path, base: str, *, require_stf: bool = True) -> bool:
    """Require pass2 wall table and {base}.single.stf (sim stages 2–5)."""
    if not _pass2_ok(sim_dir):
        return False
    csv = sim_dir / f"{base}.csv"
    dms = sim_dir / f"{base}.dms"
    if csv.is_file() and csv.stat().st_size < 50:
        return False
    if dms.is_file() and dms.stat().st_size < 50:
        return False
    if require_stf:
        stf = sim_dir / f"{base}.single.stf"
        if not stf.is_file() or stf.stat().st_size < 50:
            return False
    return True


def _verify_prebuild() -> bool:
    return BENCH_BIN.is_file() and os.access(BENCH_BIN, os.X_OK)


def _verify_s3_measured_icx_socket() -> bool:
    tsv = TC / "stage3-multi-timestep/results/measured/stage3-measured.tsv"
    return all(_tsv_has_config(tsv, f"icx-socket-{iters}") for iters in (100, 1000, 10000))


def _verify_s3_measured_icx_tpn1() -> bool:
    tsv = TC / "stage3-multi-timestep/results/measured/stage3-measured.tsv"
    return all(_tsv_has_config(tsv, f"icx-tpn1-{iters}") for iters in (100, 1000, 10000))


def _verify_s5_sim(name: str) -> bool:
    return _sim_ok(
        TC / f"stage5-scaling/results/simulated/{name}",
        f"rtof-{name}",
    )


def _verify_s5_meas(name: str) -> bool:
    tsv = TC / "stage5-scaling/results/measured/stage5-measured.tsv"
    out = TC / f"stage5-scaling/results/measured/{name}/{name}.out"
    return _tsv_has_config(tsv, name) or _bench_out_ok(out, 100)


VERIFY: dict[str, Callable[[], bool]] = {
    "Pre-build benchmark": _verify_prebuild,
    "S1 measured ICX socket": lambda: _tsv_has_config(
        TC / "stage1-ecm-modeling/results/measured/stage1-measured.tsv", "icx-socket"
    ),
    "S2 measured ICX socket": lambda: _tsv_has_config(
        TC / "stage2-single-timestep/results/measured/stage2-measured-100.tsv", "icx-socket-100"
    ),
    "S2 sim ICX socket": lambda: _sim_ok(
        TC / "stage2-single-timestep/results/simulated/icx-socket-100", "icx-socket-100"
    ),
    "S3 sim ICX socket 100": lambda: _sim_ok(
        TC / "stage3-multi-timestep/results/simulated/icx-socket-100", "icx-socket-100"
    ),
    "S3 sim ICX socket 1000": lambda: _sim_ok(
        TC / "stage3-multi-timestep/results/simulated/icx-socket-1000", "icx-socket-1000"
    ),
    "S3 sim ICX socket 10000": lambda: _sim_ok(
        TC / "stage3-multi-timestep/results/simulated/icx-socket-10000", "icx-socket-10000"
    ),
    "S3 measured ICX socket": _verify_s3_measured_icx_socket,
    "S1 measured ICX tpn1": lambda: _tsv_has_config(
        TC / "stage1-ecm-modeling/results/measured/stage1-measured.tsv", "icx-tpn1"
    ),
    "S2 measured ICX tpn1": lambda: _tsv_has_config(
        TC / "stage2-single-timestep/results/measured/stage2-measured-100.tsv", "icx-tpn1-100"
    ),
    "S2 sim ICX tpn1": lambda: _sim_ok(
        TC / "stage2-single-timestep/results/simulated/icx-tpn1-100", "icx-tpn1-100"
    ),
    "S3 sim ICX tpn1 100": lambda: _sim_ok(
        TC / "stage3-multi-timestep/results/simulated/icx-tpn1-100", "icx-tpn1-100"
    ),
    "S3 sim ICX tpn1 1000": lambda: _sim_ok(
        TC / "stage3-multi-timestep/results/simulated/icx-tpn1-1000", "icx-tpn1-1000"
    ),
    "S3 sim ICX tpn1 10000": lambda: _sim_ok(
        TC / "stage3-multi-timestep/results/simulated/icx-tpn1-10000", "icx-tpn1-10000"
    ),
    "S3 measured ICX tpn1": _verify_s3_measured_icx_tpn1,
    "S4 measured ICX": lambda: _tsv_has_config(
        TC / "stage4-idlewave/results/measured/stage4-measured.tsv", "icx"
    ),
    "S4 measured-trace ICX": lambda: _stf_ok(
        TC / "stage4-idlewave/results/measured-trace/icx/rtof-idlewave-measured.single.stf"
    ),
    "S4 sim ICX idlewave": lambda: _sim_ok(
        TC / "stage4-idlewave/results/simulated/icx", "rtof-idlewave"
    ),
    "S5 measured strong-icx-1node": lambda: _verify_s5_meas("strong-icx-1node"),
    "S5 sim strong-icx-1node": lambda: _verify_s5_sim("strong-icx-1node"),
    "S5 measured weak-icx-1node": lambda: _verify_s5_meas("weak-icx-1node"),
    "S5 sim weak-icx-1node": lambda: _verify_s5_sim("weak-icx-1node"),
    "S5 measured strong-icx-2node": lambda: _verify_s5_meas("strong-icx-2node"),
    "S5 sim strong-icx-2node": lambda: _verify_s5_sim("strong-icx-2node"),
    "S5 measured weak-icx-2node": lambda: _verify_s5_meas("weak-icx-2node"),
    "S5 sim weak-icx-2node": lambda: _verify_s5_sim("weak-icx-2node"),
    "S5 measured strong-icx-4node": lambda: _verify_s5_meas("strong-icx-4node"),
    "S5 sim strong-icx-4node": lambda: _verify_s5_sim("strong-icx-4node"),
    "S5 measured weak-icx-4node": lambda: _verify_s5_meas("weak-icx-4node"),
    "S5 sim weak-icx-4node": lambda: _verify_s5_sim("weak-icx-4node"),
    "S5 measured strong-icx-8node": lambda: _verify_s5_meas("strong-icx-8node"),
    "S5 sim strong-icx-8node": lambda: _verify_s5_sim("strong-icx-8node"),
    "S5 measured weak-icx-8node": lambda: _verify_s5_meas("weak-icx-8node"),
    "S5 sim weak-icx-8node": lambda: _verify_s5_sim("weak-icx-8node"),
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
