#!/usr/bin/env python3
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: After the O3 / penalty=0 base campaign, freeze a per-case COMP
#          penalty (ns/timestep) for every case with |wall error| > 5%.
#          penalty_ns = (meas − sim) / ntimes × 1e9  (may be negative).

from __future__ import annotations

import csv
import re
import sys
from pathlib import Path

TC = Path(__file__).resolve().parent
DEMO_ROOT = TC.parent
if str(DEMO_ROOT) not in sys.path:
    sys.path.insert(0, str(DEMO_ROOT))
import metrics as dmetrics  # noqa: E402

ERR_THRESHOLD = 0.05
ACTIVE_TSV = TC / "penalty_active.tsv"


def parse_rank_times(path: Path) -> list[float]:
    if not path.is_file():
        return []
    text = path.read_text(errors="replace")
    blocks = text.split("FULL APPLICATION PERFORMANCE (for all ranks):")
    if len(blocks) < 2:
        return []
    section = blocks[-1].split("FULL APPLICATION PERFORMANCE")[0]
    times: list[float] = []
    for line in section.splitlines():
        match = re.match(r"\s*(\d+)\s+\S+\s+([\d.eE+-]+)\s*$", line)
        if match:
            times.append(float(match.group(2)))
    return times


def sim_wall(sim_dir: Path) -> float | None:
    for name in ("slurm-discostic-pass2.out", "slurm-discostic-pass1.out"):
        times = parse_rank_times(sim_dir / name)
        if times:
            return max(times)
    return None


def last_meas(tsv: Path, config: str) -> tuple[float, int] | None:
    rows = dmetrics.last_row_tsv(tsv)
    row = rows.get(config)
    if not row:
        return None
    try:
        wall = float(row["wall_seconds"])
    except (KeyError, TypeError, ValueError):
        return None
    ntimes = int(float(row["ntimes"])) if row.get("ntimes") else 0
    return wall, ntimes


def set_penalty(cfg: Path, penalty_ns: float) -> None:
    text = cfg.read_text()
    line = f"penalty = {penalty_ns:.0f}"
    if re.search(r"^penalty\s*=", text, re.M):
        text = re.sub(r"^penalty\s*=.*$", line, text, count=1, flags=re.M)
    else:
        text += "\n" + line + "\n"
    cfg.write_text(text)


# (stage, active_config, tsv_config, ntimes_fallback, meas_tsv, sim_dir, penalty_cfg)
CASES: list[tuple[str, str, str, int, Path, Path, Path]] = [
    (
        "2",
        "icx-tpn1-100",
        "icx-tpn1-100",
        100,
        TC / "stage2-single-timestep/results/measured/stage2-measured-100.tsv",
        TC / "stage2-single-timestep/results/simulated/icx-tpn1-100",
        TC / "stage2-single-timestep/configs-penalty/icx-tpn1-100.cfg",
    ),
    (
        "3",
        "icx-tpn1-100",
        "icx-tpn1-100",
        100,
        TC / "stage3-multi-timestep/results/measured/stage3-measured.tsv",
        TC / "stage3-multi-timestep/results/simulated/icx-tpn1-100",
        TC / "stage3-multi-timestep/configs-penalty/icx-tpn1-100.cfg",
    ),
    (
        "3",
        "icx-tpn1-1000",
        "icx-tpn1-1000",
        1000,
        TC / "stage3-multi-timestep/results/measured/stage3-measured.tsv",
        TC / "stage3-multi-timestep/results/simulated/icx-tpn1-1000",
        TC / "stage3-multi-timestep/configs-penalty/icx-tpn1-1000.cfg",
    ),
    (
        "3",
        "icx-tpn1-10000",
        "icx-tpn1-10000",
        10000,
        TC / "stage3-multi-timestep/results/measured/stage3-measured.tsv",
        TC / "stage3-multi-timestep/results/simulated/icx-tpn1-10000",
        TC / "stage3-multi-timestep/configs-penalty/icx-tpn1-10000.cfg",
    ),
    (
        "4",
        "icx",
        "icx",
        45,
        TC / "stage4-idlewave/results/measured/stage4-measured.tsv",
        TC / "stage4-idlewave/results/simulated/icx",
        TC / "stage4-idlewave/configs-penalty/icx-idlewave.cfg",
    ),
]
for kind in ("strong", "weak"):
    for nodes in (1, 2, 4, 8, 16, 32):
        name = f"{kind}-icx-{nodes}node"
        CASES.append(
            (
                "5",
                name,
                name,
                5000,
                TC / "stage5-scaling/results/measured/stage5-measured.tsv",
                TC / f"stage5-scaling/results/simulated/{name}",
                TC / f"stage5-scaling/configs-penalty/{name}.cfg",
            )
        )


def main() -> int:
    active: list[list[str]] = []
    skipped: list[str] = []
    for stage, cfg, tsv_cfg, n_fallback, tsv, sim_dir, pen_cfg in CASES:
        meas = last_meas(tsv, tsv_cfg)
        sim = sim_wall(sim_dir)
        if meas is None or sim is None:
            skipped.append(f"{stage}:{cfg} (missing meas or sim)")
            continue
        meas_s, ntimes = meas
        if ntimes <= 0:
            ntimes = n_fallback
        if meas_s <= 0 or ntimes <= 0:
            skipped.append(f"{stage}:{cfg} (bad ntimes/wall)")
            continue
        err = (sim - meas_s) / meas_s
        if abs(err) <= ERR_THRESHOLD:
            skipped.append(f"{stage}:{cfg} |err|={err*100:.2f}% ≤ 5%")
            continue
        penalty_ns = (meas_s - sim) / ntimes * 1e9
        if not pen_cfg.is_file():
            skipped.append(f"{stage}:{cfg} missing {pen_cfg}")
            continue
        set_penalty(pen_cfg, penalty_ns)
        active.append(
            [
                stage,
                cfg,
                str(ntimes),
                f"{meas_s:.6f}",
                f"{sim:.6f}",
                f"{err*100:.4f}",
                f"{penalty_ns:.0f}",
                str(pen_cfg.relative_to(TC)),
            ]
        )
        print(
            f"  ACTIVE {stage}:{cfg} meas={meas_s:.4f}s sim={sim:.4f}s "
            f"err={err*100:+.2f}% penalty={penalty_ns:.0f} ns/step"
        )

    ACTIVE_TSV.write_text("")
    with ACTIVE_TSV.open("w", newline="") as fh:
        w = csv.writer(fh, delimiter="\t")
        w.writerow(
            [
                "stage",
                "config",
                "ntimes",
                "meas_s",
                "sim_s",
                "error_pct",
                "penalty_ns",
                "cfg_path",
            ]
        )
        w.writerows(active)
    print(f"wrote {ACTIVE_TSV} ({len(active)} cases)")
    for line in skipped:
        print(f"  skip {line}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
