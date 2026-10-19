#!/usr/bin/env python3
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: Submit collectives-demo SLURM jobs sequentially; advance on COMPLETED and halt on failure.
#          Do not invoke this chain until the user asks to run jobs.

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

TC = Path(__file__).resolve().parent
ROOT = Path(os.environ.get("DISCOSTIC_ROOT") or os.environ.get("DISCOSTIC_EXTENSION_ROOT") or str(TC.parent.parent))  # DisCostiC-Sim/

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
    script: Path | None = None
    command: str | None = None
    nodes: int = 0
    ntasks: int = 0
    mem_gb: int = 0
    time_h: float = 0.0


STEPS: list[Step] = [
    Step("S1 sim bcast-binomial", TC / "stage1-collective-cost/jobs/simulated/bcast-binomial.sh", nodes=2, ntasks=73, mem_gb=130, time_h=0.28),
    Step("S1 measured bcast-binomial", TC / "stage1-collective-cost/jobs/measured/bcast-binomial.sh", nodes=1, ntasks=72, mem_gb=130, time_h=0.33),
    Step("S1 sim bcast-ring", TC / "stage1-collective-cost/jobs/simulated/bcast-ring.sh", nodes=2, ntasks=73, mem_gb=130, time_h=0.28),
    Step("S1 measured bcast-ring", TC / "stage1-collective-cost/jobs/measured/bcast-ring.sh", nodes=1, ntasks=72, mem_gb=130, time_h=0.33),
    Step("S1 sim reduce-binomial", TC / "stage1-collective-cost/jobs/simulated/reduce-binomial.sh", nodes=2, ntasks=73, mem_gb=130, time_h=0.28),
    Step("S1 measured reduce-binomial", TC / "stage1-collective-cost/jobs/measured/reduce-binomial.sh", nodes=1, ntasks=72, mem_gb=130, time_h=0.33),
    Step("S1 sim allreduce-recdbl", TC / "stage1-collective-cost/jobs/simulated/allreduce-recdbl.sh", nodes=2, ntasks=73, mem_gb=130, time_h=0.28),
    Step("S1 measured allreduce-recdbl", TC / "stage1-collective-cost/jobs/measured/allreduce-recdbl.sh", nodes=1, ntasks=72, mem_gb=130, time_h=0.33),
    Step("S2 sim ICX Bcast binomial", TC / "stage2-traces/jobs/simulated/icx.sh", nodes=2, ntasks=73, mem_gb=130, time_h=0.28),
    Step("S2 measured-trace ICX", TC / "stage2-traces/jobs/measured-trace/icx.sh", nodes=1, ntasks=72, mem_gb=130, time_h=0.33),
]


def _pass2_ok(sim_dir: Path) -> bool:
    pass2 = sim_dir / "slurm-discostic-pass2.out"
    if not pass2.is_file() or pass2.stat().st_size < 50:
        return False
    text = pass2.read_text(errors="replace")
    if "Traceback" in text or "Exiting from Simulation" in text:
        return False
    return "Collective Performance Analysis (Multiple Sizes):" in text


def _stf_ok(path: Path) -> bool:
    return path.is_file() and path.stat().st_size >= 50


def _imb_ok(algo: str) -> bool:
    out = TC / f"stage1-collective-cost/results/measured/{algo}/imb.out"
    if not out.is_file() or out.stat().st_size < 50:
        return False
    text = out.read_text(errors="replace")
    return "#bytes" in text or "t_min" in text


def _sim_ok(algo: str) -> bool:
    return _pass2_ok(TC / f"stage1-collective-cost/results/simulated/{algo}")


VERIFY: dict[str, Callable[[], bool]] = {
    "S1 sim bcast-binomial": lambda: _sim_ok("bcast-binomial"),
    "S1 measured bcast-binomial": lambda: _imb_ok("bcast-binomial"),
    "S1 sim bcast-ring": lambda: _sim_ok("bcast-ring"),
    "S1 measured bcast-ring": lambda: _imb_ok("bcast-ring"),
    "S1 sim reduce-binomial": lambda: _sim_ok("reduce-binomial"),
    "S1 measured reduce-binomial": lambda: _imb_ok("reduce-binomial"),
    "S1 sim allreduce-recdbl": lambda: _sim_ok("allreduce-recdbl"),
    "S1 measured allreduce-recdbl": lambda: _imb_ok("allreduce-recdbl"),
    "S2 sim ICX Bcast binomial": lambda: _stf_ok(
        TC / "stage2-traces/results/simulated/icx/bcast-binomial.single.stf"
    ) and _pass2_ok(TC / "stage2-traces/results/simulated/icx"),
    "S2 measured-trace ICX": lambda: _stf_ok(
        TC / "stage2-traces/results/measured-trace/icx/bcast-binomial-measured.single.stf"
    ),
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
    if command.endswith(".py"):
        proc = run(["python3", command], cwd=TC)
    else:
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
            state = parts[1].split()[0]
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
        description="Run collectives-demo SLURM jobs sequentially (one at a time)."
    )
    parser.add_argument("--start", type=int, default=0, metavar="N", help="Start at step index N.")
    parser.add_argument("--poll-interval", type=int, default=30, help="Seconds between status checks.")
    parser.add_argument("--heartbeat", type=int, default=300, help="Seconds between heartbeat logs.")
    parser.add_argument("--force", action="store_true", help="Re-run steps even when artifacts verify.")
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Print steps without submitting or waiting (does not shell out).",
    )
    parser.add_argument(
        "--stages",
        type=str,
        default=None,
        metavar="N[,N...]",
        help="Comma-separated stage numbers (e.g. '1,2').",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    active_steps: list[Step] = STEPS
    if args.stages:
        stage_nums = {int(s.strip()) for s in args.stages.split(",")}
        active_steps = [
            s for s in STEPS if any(re.match(rf"^S{n}\b", s.label) for n in stage_nums)
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
                runner = "python3" if step.command.endswith(".py") else "bash"
                log(f"  would run: {runner} {step.command}")
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
            # Compute-node NFS can lag a few seconds after squeue drops the job.
            for attempt in range(8):
                time.sleep(5)
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
