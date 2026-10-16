#!/usr/bin/env python3
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: Collectives demo figures: sim vs IMB min/max duration vs message size.
#          Do not run until Stage 1 results exist and the user asks.

from __future__ import annotations

import argparse
import csv
import sys
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt

SCRIPT = Path(__file__).resolve()
TC_ROOT = SCRIPT.parent
DEMO_ROOT = TC_ROOT.parent
if str(DEMO_ROOT) not in sys.path:
    sys.path.insert(0, str(DEMO_ROOT))
import metrics as dmetrics  # noqa: E402

C_SIM = "#3070b0"
C_MEAS = "#d04040"
C_DIFF = "#2ca02c"

# Panel order: (a) Allreduce, (b) Reduce, (c) Bcast binomial, (d) Bcast ring.
ALGOS = (
    ("allreduce-recdbl", "Allreduce recursive doubling", "a"),
    ("reduce-binomial", "Reduce binomial", "b"),
    ("bcast-binomial", "Bcast binomial", "c"),
    ("bcast-ring", "Bcast ring", "d"),
)


def apply_thesis_style() -> None:
    plt.rcParams.update(
        {
            "font.family": "DejaVu Sans",
            "font.size": 10,
            "axes.labelsize": 11,
            "axes.titlesize": 11,
            "axes.linewidth": 1.0,
            "xtick.labelsize": 9,
            "ytick.labelsize": 9,
            "legend.fontsize": 8,
            "figure.facecolor": "white",
            "axes.facecolor": "white",
            "axes.grid": True,
            "grid.alpha": 0.35,
            "grid.linewidth": 0.6,
            "axes.axisbelow": True,
        }
    )


def save_figure(fig, path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(path, dpi=160, bbox_inches="tight", facecolor="white")
    plt.close(fig)
    print(f"  wrote {path}")


def _load_algo(algo: str) -> list[dict[str, float]]:
    sim_path = (
        TC_ROOT
        / "stage1-collective-cost"
        / "results"
        / "simulated"
        / algo
        / "slurm-discostic-pass2.out"
    )
    meas_path = TC_ROOT / "stage1-collective-cost" / "results" / "measured" / algo / "imb.out"
    sim = dmetrics.parse_sim_collective_sizes(sim_path)
    meas = dmetrics.parse_imb_collective_sizes(meas_path)
    return dmetrics.collective_score_rows(sim, meas)


def _write_csv(rows_by_algo: dict[str, list[dict[str, float]]], path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    fields = [
        "algorithm",
        "size_kb",
        "sim_min_us",
        "sim_avg_us",
        "sim_max_us",
        "meas_min_us",
        "meas_avg_us",
        "meas_max_us",
        "min_err_pct",
        "max_err_pct",
        "avg_err_pct",
        "min_abs_diff_us",
        "max_abs_diff_us",
    ]
    with path.open("w", newline="") as fh:
        writer = csv.DictWriter(fh, fieldnames=fields)
        writer.writeheader()
        for algo, rows in rows_by_algo.items():
            for row in rows:
                out = {"algorithm": algo}
                out.update(row)
                writer.writerow(out)
    print(f"  wrote {path}")


def _with_abs_diff(rows: list[dict[str, float]]) -> list[dict[str, float]]:
    out = []
    for row in rows:
        r = dict(row)
        r["min_abs_diff_us"] = abs(r["sim_min_us"] - r["meas_min_us"])
        r["max_abs_diff_us"] = abs(r["sim_max_us"] - r["meas_max_us"])
        out.append(r)
    return out


def _plot_minmax(ax, rows: list[dict[str, float]], which: str, title: str) -> None:
    if not rows:
        ax.set_title(f"{title} (no data)")
        ax.set_xlabel("Message Size (KB)")
        ax.set_ylabel("Time (µs)")
        return
    sizes = [r["size_kb"] for r in rows]
    sim = [r[f"sim_{which}_us"] for r in rows]
    meas = [r[f"meas_{which}_us"] for r in rows]
    diff = [abs(s - m) for s, m in zip(sim, meas)]
    label_which = "Min" if which == "min" else "Max"
    def _log_safe(seq: list[float]) -> list[float]:
        return [v if v > 0 else float("nan") for v in seq]

    ax.plot(sizes, _log_safe(meas), "o-", color=C_MEAS, label=f"IMB {label_which}")
    ax.plot(sizes, _log_safe(sim), "s--", color=C_SIM, label=f"Simulated {label_which}")
    ax.plot(sizes, _log_safe(diff), "^:", color=C_DIFF, label="Absolute Difference")
    ax.set_xscale("log", base=2)
    ax.set_yscale("log")
    ax.set_xticks(list(dmetrics.COLLECTIVE_SCORE_SIZES_KB))
    ax.set_xticklabels([str(int(s)) for s in dmetrics.COLLECTIVE_SCORE_SIZES_KB])
    ax.set_xlabel("Message Size (KB)")
    ax.set_ylabel("Time (µs)")
    ax.set_title(f"{title} · log–log")
    ax.legend(frameon=True, loc="upper left")


def make_stage1() -> None:
    fig_dir = TC_ROOT / "stage1-collective-cost" / "figures"
    rows_by_algo: dict[str, list[dict[str, float]]] = {}
    apply_thesis_style()
    fig61, axes61 = plt.subplots(4, 2, figsize=(11.0, 14.0), sharex=True)
    for row_i, (algo, title, panel) in enumerate(ALGOS):
        rows = _with_abs_diff(_load_algo(algo))
        rows_by_algo[algo] = rows
        fig, axes = plt.subplots(1, 2, figsize=(10.5, 4.2), sharex=True)
        _plot_minmax(axes[0], rows, "min", f"({panel}) {title} — min")
        _plot_minmax(axes[1], rows, "max", f"({panel}) {title} — max")
        fig.suptitle(
            "Comparison of min/max time across processes (simulated vs IMB)",
            y=1.02,
        )
        save_figure(fig, fig_dir / f"stage1-{algo}.png")
        _plot_minmax(axes61[row_i][0], rows, "min", f"({panel}) {title} — min")
        _plot_minmax(axes61[row_i][1], rows, "max", f"({panel}) {title} — max")
    fig61.suptitle(
        "Figure 6.1 — Simulated vs IMB collective duration (ICX, IMB 1 node × 72 ranks)",
        y=0.995,
    )
    fig61.tight_layout(rect=(0, 0, 1, 0.98))
    save_figure(fig61, fig_dir / "stage1-fig6.1.png")
    _write_csv(rows_by_algo, fig_dir / "stage1_collective_cost.csv")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Collectives demo figures (sim vs IMB).")
    parser.add_argument(
        "--stages",
        type=str,
        default="1",
        help="Comma-separated stages (1 is scoring; 2 is ITAC only).",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    stages = {int(s.strip()) for s in args.stages.split(",") if s.strip()}
    if 1 in stages:
        make_stage1()
    if 2 in stages:
        print(
            "Stage 2 is ITAC STF only (no matplotlib Gantt). Open "
            "stage2-traces/results/simulated/icx/bcast-binomial.single.stf and "
            "stage2-traces/results/measured-trace/icx/bcast-binomial-measured.single.stf "
            "in Intel Trace Analyzer."
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
