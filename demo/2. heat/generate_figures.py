#!/usr/bin/env python3
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: HEAT Jacobi 2D demo figures (Stages 1–5). figures/ = meas vs sim-base; figures-penalty/ = three-series.

from __future__ import annotations

import argparse
import csv
import re
import sys
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import matplotlib.ticker as mticker
from matplotlib.lines import Line2D

SCRIPT = Path(__file__).resolve()
TC_ROOT = SCRIPT.parent
DEMO_ROOT = TC_ROOT.parent
REPO_ROOT = TC_ROOT.parent.parent
if str(DEMO_ROOT) not in sys.path:
    sys.path.insert(0, str(DEMO_ROOT))
import metrics as dmetrics  # noqa: E402

ECM_COLORS = {
    "T_comp": "#f0c040",
    "T_RegL1": "#a8d8a8",
    "T_L1L2": "#a8c8e8",
    "T_L2L3": "#c8a8e8",
    "T_L3Mem": "#e8c8c8",
}
C_ECM_LINE = "#2166ac"
C_SIM = "#3070b0"
C_MEAS = "#d04040"
C_ERR = "#b0b0b0"
C_PEN = "#2e7d4f"
LEGEND_SIM = "DisCostiC simulation"
LEGEND_MEAS = "MPI benchmark"
LEGEND_PEN = "DisCostiC + penalty"
LEGEND_ERR = "|% error|"

ECM_HEAT_ICX = {"T_comp": 2.0, "T_RegL1": 2.0, "T_L1L2": 5.0, "T_L2L3": 8.0, "T_L3Mem": 4.1}


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
            "legend.fontsize": 9,
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


def comparison_handles(include_penalty: bool = False):
    handles = [
        mpatches.Patch(color=C_MEAS, label=LEGEND_MEAS),
        mpatches.Patch(color=C_SIM, label=LEGEND_SIM),
    ]
    if include_penalty:
        handles.append(mpatches.Patch(color=C_PEN, label=LEGEND_PEN))
    return handles


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


def maybe_load_ecm_from_cfg(path: Path, fallback: dict) -> dict:
    if not path.is_file():
        return dict(fallback)
    text = path.read_text(errors="replace")
    out = dict(fallback)
    mapping = {
        "T_OL": "T_comp",
        "T_nOL": "T_RegL1",
        "T_L1L2": "T_L1L2",
        "T_L2L3": "T_L2L3",
        "T_L3Mem": "T_L3Mem",
    }
    for src, dst in mapping.items():
        m = re.search(rf"^{src}\s*=\s*([0-9.]+)", text, re.M)
        if m:
            out[dst] = float(m.group(1))
    return out


def t_ecm(vals: dict) -> float:
    data = vals["T_RegL1"] + vals["T_L1L2"] + vals["T_L2L3"] + vals["T_L3Mem"]
    return max(vals["T_comp"], data)


def draw_ecm_stack(
    ax,
    arch: str,
    vals: dict,
    *,
    subtitle: str = "",
    kernel_label: str = "Jacobi 2D (HEAT)",
) -> None:
    """Overlapping ECM: T_ECM = max(T_comp, data-path). Two columns, not a sum."""
    t_comp = vals["T_comp"]
    data_segs = [
        ("T_RegL1", vals["T_RegL1"]),
        ("T_L1L2", vals["T_L1L2"]),
        ("T_L2L3", vals["T_L2L3"]),
        ("T_L3Mem", vals["T_L3Mem"]),
    ]
    ax.bar(
        0, t_comp, width=0.55, color=ECM_COLORS["T_comp"],
        edgecolor="#444", linewidth=0.8, label=f"T_comp = {t_comp:g}",
    )
    if t_comp >= 0.5:
        ax.text(0, t_comp / 2, f"{t_comp:g}", ha="center", va="center", fontsize=9)
    bottom = 0.0
    for name, value in data_segs:
        if value <= 0:
            continue
        ax.bar(
            1, value, bottom=bottom, width=0.55, color=ECM_COLORS[name],
            edgecolor="#444", linewidth=0.8, label=f"{name} = {value:g}",
        )
        if value >= 0.5:
            ax.text(1, bottom + value / 2, f"{value:g}", ha="center", va="center", fontsize=9)
        bottom += value
    tecm = t_ecm(vals)
    ax.axhline(tecm, color=C_ECM_LINE, ls="--", lw=2, label=f"T_ECM = {tecm:.1f} cy/CL")
    ax.set_xticks([0, 1])
    ax.set_xticklabels(["in-core\nT_comp", "data path"])
    ax.set_xlim(-0.55, 1.55)
    title = f"{arch}"
    if subtitle:
        title = f"{arch} — {subtitle}"
    elif kernel_label:
        title = f"{arch} — {kernel_label}"
    ax.set_title(title, fontsize=10)
    ax.set_ylabel("ECM [cy/CL]")
    ax.set_ylim(0, max(bottom, t_comp, tecm) * 1.28)
    ax.legend(fontsize=7, loc="upper right")
    ax.grid(True, axis="y", alpha=0.3)


def resolve_ecm_cfg(primary: Path, *fallbacks: Path) -> Path | None:
    for path in (primary, *fallbacks):
        if path.is_file():
            return path
    return None


def parse_measured_tsv(path: Path) -> dict[str, dict]:
    """Last TSV row for a given config wins (Stage 4 has a pre-idlewave duplicate)."""
    out: dict[str, dict] = {}
    for cfg, row in dmetrics.last_row_tsv(path).items():
        try:
            wall_s = float(row["wall_seconds"])
        except (KeyError, TypeError, ValueError):
            continue
        rates = dmetrics.heat_from_tsv_row(row, wall_s)
        out[cfg] = {"wall_s": wall_s, **rates, "raw": row}
    return out


def make_stage1() -> list[Path]:
    apply_thesis_style()
    fig_dir = TC_ROOT / "stage1-ecm-modeling/figures"
    paths: list[Path] = []

    heat_cfg = resolve_ecm_cfg(
        TC_ROOT / "stage2-single-timestep/results/simulated/icx-socket-100/HEAT_FILE_ecm.cfg",
        TC_ROOT / "stage3-multi-timestep/results/simulated/icx-socket-100/HEAT_FILE_ecm.cfg",
        REPO_ROOT / "nodelevel/configs/HEAT_FILE.cfg",
    )
    heat_vals = maybe_load_ecm_from_cfg(heat_cfg or Path(), ECM_HEAT_ICX)

    combined = fig_dir / "stage1-ecm-breakdown.png"
    fig, ax = plt.subplots(figsize=(5.6, 5.2))
    src = str(heat_cfg) if heat_cfg else "fallback (run a sim to generate HEAT_FILE ECM)"
    draw_ecm_stack(
        ax,
        "ICX",
        heat_vals,
        subtitle="Kerncraft HEAT_FILE · memory-bound 5-pt",
        kernel_label="Jacobi 2D (HEAT)",
    )
    fig.suptitle("Stage 1 — ECM modeling (HEAT) · ICX · -O3", fontsize=12)
    fig.tight_layout()
    save_figure(fig, combined)
    paths.append(combined)

    csv_path = fig_dir / "stage1_ecm_data.csv"
    with csv_path.open("w", newline="") as fh:
        w = csv.writer(fh)
        w.writerow(["kernel", "arch", "cfg_source", "T_comp", "T_RegL1", "T_L1L2", "T_L2L3", "T_L3Mem", "T_ECM"])
        w.writerow(
            [
                "HEAT",
                "ICX",
                src,
                heat_vals["T_comp"],
                heat_vals["T_RegL1"],
                heat_vals["T_L1L2"],
                heat_vals["T_L2L3"],
                heat_vals["T_L3Mem"],
                f"{t_ecm(heat_vals):.4f}",
            ]
        )
    print(f"  wrote {csv_path}")
    paths.append(csv_path)

    meas = parse_measured_tsv(TC_ROOT / "stage1-ecm-modeling/results/measured/stage1-measured.tsv")
    if meas:
        labels = list(meas.keys())
        walls = [meas[k]["wall_s"] for k in labels]
        fig, ax = plt.subplots(figsize=(7.2, 4.2))
        ax.bar(labels, walls, color=C_MEAS, edgecolor="#333", width=0.55, zorder=3)
        ax.set_ylabel("Wall time [s]")
        ax.set_title("Stage 1 — Measured wall time (HEAT Jacobi 2D)")
        ax.tick_params(axis="x", rotation=15)
        save_figure(fig, fig_dir / "stage1-measured-wall.png")
        paths.append(fig_dir / "stage1-measured-wall.png")
    return paths


def _heat_rates(row: dict, wall_s: float) -> dict[str, float]:
    return dmetrics.heat_from_tsv_row(
        {
            "dim_x": row.get("dim_x", 16384),
            "dim_y": row.get("dim_y", 92160),
            "ntimes": row.get("ntimes", 0),
        },
        wall_s,
    )


def sim_wall(sim_dir: Path) -> float | None:
    for name in ("slurm-discostic-pass2.out", "slurm-discostic-pass1.out"):
        times = parse_rank_times(sim_dir / name)
        if times:
            return max(times)
    return None


def rate_err_pct(sim_rate: float, meas_rate: float) -> float:
    return dmetrics.signed_error_pct(sim_rate, meas_rate)


def grouped_bars(ax, labels, series, ylabel, title, group_errors=None):
    """series: list of (name, values_or_None, color). None values skip that bar."""
    n = len(labels)
    live = [(name, vals, color) for name, vals, color in series if any(v is not None for v in vals)]
    nser = max(len(live), 1)
    width = 0.8 / nser
    xs = list(range(n))
    ymax = 0.0
    for i, (name, vals, color) in enumerate(live):
        offs = [x - 0.4 + width * (i + 0.5) for x in xs]
        plot_x = [x for x, v in zip(offs, vals) if v is not None]
        plot_y = [v for v in vals if v is not None]
        ax.bar(plot_x, plot_y, width * 0.92, color=color, edgecolor="#333", label=name, zorder=3)
        for x, v in zip(offs, vals):
            if v is None:
                continue
            ymax = max(ymax, v)
            ax.text(x, v, f"{v:.2f}" if v < 100 else f"{v:.1f}", ha="center", va="bottom", fontsize=7)
    ax.set_xticks(xs)
    ax.set_xticklabels(labels, fontsize=8)
    ax.set_ylabel(ylabel)
    ax.set_title(title, fontsize=10)
    two_line = bool(group_errors and any(e and "\n" in e for e in group_errors))
    headroom = 1.42 if two_line else 1.32 if group_errors else 1.22
    if ymax > 0:
        ax.set_ylim(0, ymax * headroom)
    if group_errors:
        y_txt = ymax * (1.12 if two_line else 1.06)
        for i, txt in enumerate(group_errors):
            if not txt:
                continue
            ax.text(
                i, y_txt, txt, ha="center", va="bottom", fontsize=8, color="#222",
                bbox={"boxstyle": "round,pad=0.18", "facecolor": "white", "edgecolor": "#888", "alpha": 0.92},
                zorder=6,
            )
    ax.legend(fontsize=8)


def write_metric_csv(path: Path, rows: list[list], header: list[str]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", newline="") as fh:
        w = csv.writer(fh)
        w.writerow(header)
        w.writerows(rows)
    print(f"  wrote {path}")


CSV_HEADER = [
    "config",
    "meas_wall_s",
    "sim_wall_s",
    "wall_err_pct",
    "meas_mlup_s",
    "sim_mlup_s",
    "meas_gf_s",
    "sim_gf_s",
    "meas_gb_s",
    "sim_gb_s",
    "rate_err_pct",
    "formula",
]
FORMULA = (
    "MLUP/s=dim_x*dim_y*ntimes/wall/1e6; GF/s=5*updates/wall/1e9; "
    "GB/s=32*updates/wall/1e9; wall_err=(sim-meas)/meas; rate_err=-wall_err/(1+wall_err)"
)


def csv_row(cfg: str, meas_s: float, sim_s: float, meas_r: dict, sim_r: dict) -> list:
    wall_err = dmetrics.signed_error_pct(sim_s, meas_s)
    rerr = rate_err_pct(sim_r["mlup_s"], meas_r["mlup_s"])
    return [
        cfg,
        f"{meas_s:.6f}",
        f"{sim_s:.6f}",
        f"{wall_err:.4f}",
        f"{meas_r['mlup_s']:.4f}",
        f"{sim_r['mlup_s']:.4f}",
        f"{meas_r['gf_s']:.4f}",
        f"{sim_r['gf_s']:.4f}",
        f"{meas_r['gb_s']:.4f}",
        f"{sim_r['gb_s']:.4f}",
        f"{rerr:.4f}",
        FORMULA,
    ]


def comparison_subtitle(include_penalty: bool) -> str:
    return "meas vs sim-base vs sim-penalty" if include_penalty else "meas vs sim"


def fmt_err_pct(sim, meas) -> str | None:
    if sim is None or meas is None or meas == 0:
        return None
    return f"{dmetrics.signed_error_pct(sim, meas):+.1f}%"


def annotate_line_errors(ax, xs, meas_y, sim_y, pen_y=None):
    for i, x in enumerate(xs):
        meas = meas_y[i] if i < len(meas_y) else None
        sim = sim_y[i] if i < len(sim_y) else None
        err = fmt_err_pct(sim, meas)
        if err and sim is not None:
            ax.annotate(
                err, (x, sim), textcoords="offset points", xytext=(0, 8),
                ha="center", fontsize=7, color=C_SIM, zorder=5,
            )
        if pen_y is None:
            continue
        pen = pen_y[i] if i < len(pen_y) else None
        err_p = fmt_err_pct(pen, meas)
        if err_p and pen is not None:
            ax.annotate(
                err_p, (x, pen), textcoords="offset points", xytext=(0, -11),
                ha="center", fontsize=7, color=C_PEN, zorder=5,
            )


S2_PANEL_TITLE = {
    "icx-tpn1-100": "(a) ICX, task_per_node=1\n(36 ranks × 36 nodes)",
    "icx-socket-100": "(b) ICX, task_per_node=36\n(full socket, 36 ranks)",
}
S3_PANEL_TITLE = {
    "icx-tpn1": "(a) ICX, task_per_node=1 (36 ranks)",
    "icx-socket": "(b) ICX, task_per_node=36 (full socket)",
}


def _fmt_bar(v: float) -> str:
    return f"{v:.2f}" if abs(v) < 100 else f"{v:.1f}"


def draw_s2_panel(ax, sim_v, meas_v, ylabel, title, err_pct, extra_box: str | None = None) -> None:
    ax.bar(0, sim_v, width=0.62, color=C_SIM, edgecolor="#333", zorder=3)
    ax.bar(1, meas_v, width=0.62, color=C_MEAS, edgecolor="#333", zorder=3)
    ax.set_xticks([0, 1])
    ax.set_xticklabels(["DisCostiC\nsimulation", "MPI\nbenchmark"], fontsize=8)
    ax.set_ylabel(ylabel)
    ax.set_title(title, fontsize=9)
    ymax = max(sim_v, meas_v, 1e-9)
    ax.set_ylim(0, ymax * 1.34)
    ax.text(0, sim_v, _fmt_bar(sim_v), ha="center", va="bottom", fontsize=9, fontweight="bold")
    ax.text(1, meas_v, _fmt_bar(meas_v), ha="center", va="bottom", fontsize=9, fontweight="bold")
    box = f"Error: {err_pct:+.1f}%"
    if extra_box:
        box += f"\n{extra_box}"
    ax.text(
        0.5, 0.97, box, transform=ax.transAxes, ha="center", va="top",
        fontsize=8, fontweight="bold",
        bbox={"boxstyle": "round,pad=0.28", "facecolor": "white", "edgecolor": "black", "linewidth": 0.8},
        zorder=6,
    )


def draw_s2_panel_penalty(ax, sim_v, meas_v, pen_v, ylabel, title, err_sim, err_pen) -> None:
    vals = [sim_v, meas_v]
    colors = [C_SIM, C_MEAS]
    labs = ["DisCostiC\nsimulation", "MPI\nbenchmark"]
    if pen_v is not None:
        vals.append(pen_v)
        colors.append(C_PEN)
        labs.append("DisCostiC\n+ penalty")
    for i, (v, c) in enumerate(zip(vals, colors)):
        ax.bar(i, v, width=0.62, color=c, edgecolor="#333", zorder=3)
        ax.text(i, v, _fmt_bar(v), ha="center", va="bottom", fontsize=8, fontweight="bold")
    ax.set_xticks(list(range(len(vals))))
    ax.set_xticklabels(labs, fontsize=8)
    ax.set_ylabel(ylabel)
    ax.set_title(title, fontsize=9)
    ax.set_ylim(0, max(vals) * 1.42)
    box = f"Error: {err_sim:+.1f}%"
    if err_pen is not None:
        box += f"\nPen: {err_pen:+.1f}%"
    ax.text(
        0.5, 0.97, box, transform=ax.transAxes, ha="center", va="top",
        fontsize=8, fontweight="bold",
        bbox={"boxstyle": "round,pad=0.28", "facecolor": "white", "edgecolor": "black", "linewidth": 0.8},
        zorder=6,
    )


def draw_s3_panel(ax, ts, meas_y, sim_y, ylabel, title, *, value_suffix: str, pen_y=None, loglog: bool = False) -> None:
    xs = [float(t) for t in ts] if loglog else list(range(len(ts)))
    errs = [abs(dmetrics.signed_error_pct(s, m)) for m, s in zip(meas_y, sim_y)]
    ax2 = ax.twinx()
    width = [x * 0.28 for x in xs] if loglog else 0.5
    ax2.bar(xs, errs, width=width, color="#d0d0d0", edgecolor="#9a9a9a", zorder=1)
    emax = max(errs) if errs else 1.0
    ax2.set_ylim(0, max(emax * 1.28, 1.0))
    ax2.set_ylabel("% Error")
    ax.plot(xs, sim_y, "o-", color=C_SIM, lw=1.8, markersize=7, zorder=4)
    ax.plot(xs, meas_y, "s-", color=C_MEAS, lw=1.8, markersize=7, zorder=4)
    if pen_y is not None and any(v is not None for v in pen_y):
        px = [x for x, v in zip(xs, pen_y) if v is not None]
        py = [v for v in pen_y if v is not None]
        ax.plot(px, py, "D-", color=C_PEN, lw=1.8, markersize=7, zorder=4)
    ax.set_zorder(2)
    ax.patch.set_visible(False)
    ax.set_xlabel("Timesteps")
    ax.set_ylabel(ylabel)
    ax.set_title(title + (" · log–log" if loglog else ""), fontsize=9)
    live = list(meas_y) + list(sim_y)
    if pen_y is not None:
        live += [v for v in pen_y if v is not None]
    vals = [v for v in live if v is not None and v > 0]
    ymax = max(vals, default=0.0)
    if loglog:
        ax.set_xscale("log")
        ax.set_yscale("log")
        ax.set_xticks(xs)
        ax.set_xticklabels([str(int(t)) if float(t).is_integer() else str(t) for t in xs])
        lo = min(vals) if vals else 1.0
        hi = max(vals) if vals else 10.0
        ypad = (hi / lo) ** 0.12 if hi > lo else 1.8
        ax.set_xlim(min(xs) / 1.7, max(xs) * 1.7)
        ax.set_ylim(lo / ypad, hi * ypad)
        ax.grid(True, which="both", alpha=0.3)
    else:
        ax.set_xticks(xs)
        ax.set_xticklabels([str(t) for t in ts])
        ax.set_ylim(0, ymax * 1.18 if ymax > 0 else 1)
        ax.grid(True, axis="y", alpha=0.3)
    n = len(xs)
    for i, (x, mv, sv) in enumerate(zip(xs, meas_y, sim_y)):
        near_axis = (not loglog) and ymax > 0 and max(mv, sv) < ymax * 0.18
        if i == 0:
            ha = "right"
            if near_axis:
                sim_off = (-5, 16 if sv >= mv else 4)
                meas_off = (-5, 16 if mv > sv else 4)
            else:
                sim_off, meas_off = (-4, 8), (-4, -11)
        elif i == n - 1:
            ha = "right"
            sim_off, meas_off = (-5, 9), (-5, -12)
        else:
            ha = "left"
            sim_off, meas_off = (6, 8), (6, -12)
            if near_axis:
                sim_off = (6, 14 if sv >= mv else 3)
                meas_off = (6, 14 if mv > sv else 3)
        ax.annotate(
            f"{sv:.1f}{value_suffix}", (x, sv), xytext=sim_off, textcoords="offset points",
            fontsize=7, color=C_SIM, ha=ha, va="center", zorder=5, clip_on=False,
        )
        ax.annotate(
            f"{mv:.1f}{value_suffix}", (x, mv), xytext=meas_off, textcoords="offset points",
            fontsize=7, color=C_MEAS, ha=ha, va="center", zorder=5, clip_on=False,
        )


def s3_legend_handles(include_penalty: bool = False):
    handles = [
        Line2D([0], [0], color=C_SIM, marker="o", lw=1.8, markersize=7, label=LEGEND_SIM),
        Line2D([0], [0], color=C_MEAS, marker="s", lw=1.8, markersize=7, label=LEGEND_MEAS),
        mpatches.Patch(facecolor="#d0d0d0", edgecolor="#9a9a9a", label=LEGEND_ERR),
    ]
    if include_penalty:
        handles.insert(2, Line2D([0], [0], color=C_PEN, marker="D", lw=1.8, markersize=7, label=LEGEND_PEN))
    return handles


def finish_s2_combined(fig) -> None:
    fig.subplots_adjust(top=0.76, left=0.07, right=0.99, bottom=0.16, wspace=0.34)
    fig.suptitle("Stage 2: Single-Timestep Validation — HEAT Jacobi 2D (-O3)", fontsize=12, y=0.985)
    fig.text(0.5, 0.925, r"100 iterations · ICX", ha="center", fontsize=10)
    fig.legend(
        handles=[
            mpatches.Patch(color=C_SIM, label=LEGEND_SIM),
            mpatches.Patch(color=C_MEAS, label=LEGEND_MEAS),
        ],
        loc="upper center", ncol=2, bbox_to_anchor=(0.5, 0.905), frameon=False, fontsize=9,
    )


def finish_s3_combined(fig, *, include_penalty: bool = False) -> None:
    fig.subplots_adjust(top=0.78, left=0.07, right=0.90, bottom=0.14, wspace=0.40)
    fig.suptitle("Stage 3: Predictions Over Larger Timesteps — HEAT Jacobi 2D (-O3)", fontsize=12, y=0.985)
    fig.legend(
        handles=s3_legend_handles(include_penalty),
        loc="upper center", ncol=4 if include_penalty else 3,
        bbox_to_anchor=(0.5, 0.925), frameon=False, fontsize=9,
    )


def emit_metric_set(
    fig_dir: Path,
    stem: str,
    labels,
    meas,
    sim,
    pen,
    title_prefix: str,
    *,
    include_penalty: bool,
) -> list[Path]:
    """One PNG per metric; grouped bars. Penalty series only when include_penalty."""
    paths: list[Path] = []
    specs = [
        ("wall", "Wall time [s]", lambda m, s: m if s is None else s, True),
        ("mlup", "Performance [MLUP/s]", lambda m, s: m["mlup_s"] if s is None else _heat_rates(m, s)["mlup_s"], False),
        ("gf", "Performance [GF/s]", lambda m, s: m["gf_s"] if s is None else _heat_rates(m, s)["gf_s"], False),
        ("gb", "Bandwidth [GB/s]", lambda m, s: m["gb_s"] if s is None else _heat_rates(m, s)["gb_s"], False),
    ]
    has_pen = include_penalty and any(p is not None for p in pen)
    for key, ylabel, getter, is_wall in specs:
        meas_vals = []
        sim_vals = []
        pen_vals = []
        for m, s, p in zip(meas, sim, pen):
            if m is None:
                meas_vals.append(None)
                sim_vals.append(None)
                pen_vals.append(None)
                continue
            meas_vals.append(m["wall_s"] if is_wall else getter(m, None))
            sim_vals.append(None if s is None else (s if is_wall else getter(m, s)))
            pen_vals.append(None if p is None else (p if is_wall else getter(m, p)))
        fig, ax = plt.subplots(figsize=(max(6.5, 2.4 * len(labels)), 4.6))
        series = [(LEGEND_MEAS, meas_vals, C_MEAS), (LEGEND_SIM, sim_vals, C_SIM)]
        if has_pen:
            series.append((LEGEND_PEN, pen_vals, C_PEN))
        group_errors = []
        for mv, sv, pv in zip(meas_vals, sim_vals, pen_vals):
            err = fmt_err_pct(sv, mv)
            if has_pen:
                parts = []
                if err:
                    parts.append(f"base {err}")
                err_p = fmt_err_pct(pv, mv)
                if err_p:
                    parts.append(f"pen {err_p}")
                group_errors.append("\n".join(parts) if parts else None)
            else:
                group_errors.append(err)
        grouped_bars(
            ax,
            labels,
            series,
            ylabel,
            f"{title_prefix} — {ylabel} · {comparison_subtitle(include_penalty)}",
            group_errors=group_errors,
        )
        out = fig_dir / f"{stem}-{key}.png"
        save_figure(fig, out)
        paths.append(out)
    return paths


def make_stage2() -> list[Path]:
    apply_thesis_style()
    fig_dir = TC_ROOT / "stage2-single-timestep/figures"
    fig_pen = TC_ROOT / "stage2-single-timestep/figures-penalty"
    meas = parse_measured_tsv(TC_ROOT / "stage2-single-timestep/results/measured/stage2-measured-100.tsv")
    configs = ["icx-tpn1-100", "icx-socket-100"]
    specs = [
        ("wall", "Wall time [s]", True),
        ("mlup", "Performance [MLUP/s]", False),
        ("gf", "Performance [GF/s]", False),
        ("gb", "Bandwidth [GB/s]", False),
    ]
    rows = []
    base_csv, pen_csv = [], []
    for cfg in configs:
        m = meas.get(cfg)
        s = sim_wall(TC_ROOT / "stage2-single-timestep/results/simulated" / cfg)
        p = sim_wall(TC_ROOT / "stage2-single-timestep/results/simulated-penalty" / cfg)
        if not m or s is None:
            print(f"  skip {cfg}: missing base data")
            continue
        rows.append((cfg, m, s, p))
        sr = _heat_rates(m, s)
        base_csv.append(csv_row(cfg, m["wall_s"], s, m, sr))
        if p is not None:
            pen_csv.append(csv_row(cfg, m["wall_s"], p, m, _heat_rates(m, p)))
        print(f"  {cfg}: meas={m['wall_s']:.3f} sim={s:.3f}" + (f" pen={p:.3f}" if p else ""))
    if not rows:
        return []

    def values(m, s, *, is_wall, key):
        if is_wall:
            return s, m["wall_s"]
        rk = {"mlup": "mlup_s", "gf": "gf_s", "gb": "gb_s"}[key]
        meas_v = m[rk]
        sim_v = _heat_rates(m, s)[rk]
        return sim_v, meas_v

    paths: list[Path] = []

    def emit_s2(outdir: Path, include_penalty: bool) -> None:
        for key, ylabel, is_wall in specs:
            fig, axes = plt.subplots(1, len(rows), figsize=(4.7 * len(rows), 5.15), squeeze=False)
            for ax, (cfg, m, s, p) in zip(axes[0], rows):
                sim_v, meas_v = values(m, s, is_wall=is_wall, key=key)
                err = dmetrics.signed_error_pct(sim_v, meas_v)
                title = S2_PANEL_TITLE.get(cfg, cfg)
                if include_penalty:
                    pen_v = None if p is None else (p if is_wall else values(m, p, is_wall=is_wall, key=key)[0])
                    err_p = None if pen_v is None else dmetrics.signed_error_pct(pen_v, meas_v)
                    draw_s2_panel_penalty(ax, sim_v, meas_v, pen_v, ylabel, title, err, err_p)
                else:
                    draw_s2_panel(ax, sim_v, meas_v, ylabel, title, err)
            if include_penalty:
                fig.subplots_adjust(top=0.76, left=0.07, right=0.99, bottom=0.16, wspace=0.34)
                fig.suptitle("Stage 2: Single-Timestep Validation — HEAT Jacobi 2D (-O3)", fontsize=12, y=0.985)
                fig.text(0.5, 0.925, r"100 iterations · ICX · with penalty", ha="center", fontsize=10)
                fig.legend(
                    handles=[
                        mpatches.Patch(color=C_SIM, label=LEGEND_SIM),
                        mpatches.Patch(color=C_MEAS, label=LEGEND_MEAS),
                        mpatches.Patch(color=C_PEN, label=LEGEND_PEN),
                    ],
                    loc="upper center", ncol=3, bbox_to_anchor=(0.5, 0.905), frameon=False, fontsize=9,
                )
            else:
                finish_s2_combined(fig)
            out = outdir / f"stage2-{key}.png"
            save_figure(fig, out)
            paths.append(out)
            if key == "gf" and not include_penalty:
                alias = outdir / "stage2-sim-vs-measured.png"
                alias.write_bytes(out.read_bytes())
                print(f"  wrote {alias}")
                paths.append(alias)
            for cfg, m, s, p in rows:
                fig_s, ax_s = plt.subplots(figsize=(5.4, 4.8))
                sim_v, meas_v = values(m, s, is_wall=is_wall, key=key)
                err = dmetrics.signed_error_pct(sim_v, meas_v)
                extra = f"wall  sim {s:.2f}s / meas {m['wall_s']:.2f}s"
                stem = cfg.replace("-100", "")
                if include_penalty:
                    pen_v = None if p is None else (p if is_wall else values(m, p, is_wall=is_wall, key=key)[0])
                    err_p = None if pen_v is None else dmetrics.signed_error_pct(pen_v, meas_v)
                    draw_s2_panel_penalty(ax_s, sim_v, meas_v, pen_v, ylabel, S2_PANEL_TITLE.get(cfg, cfg), err, err_p)
                else:
                    draw_s2_panel(ax_s, sim_v, meas_v, ylabel, S2_PANEL_TITLE.get(cfg, cfg), err, extra_box=extra)
                solo = outdir / f"stage2-{stem}-{key}.png"
                save_figure(fig_s, solo)
                paths.append(solo)

    emit_s2(fig_dir, include_penalty=False)
    if any(p is not None for _c, _m, _s, p in rows):
        emit_s2(fig_pen, include_penalty=True)
    write_metric_csv(fig_dir / "stage2_data.csv", base_csv, CSV_HEADER)
    paths.append(fig_dir / "stage2_data.csv")
    if pen_csv:
        write_metric_csv(fig_pen / "stage2_data.csv", pen_csv, CSV_HEADER)
        paths.append(fig_pen / "stage2_data.csv")
    return paths


def make_stage3() -> list[Path]:
    apply_thesis_style()
    fig_dir = TC_ROOT / "stage3-multi-timestep/figures"
    fig_pen = TC_ROOT / "stage3-multi-timestep/figures-penalty"
    meas = parse_measured_tsv(TC_ROOT / "stage3-multi-timestep/results/measured/stage3-measured.tsv")
    platforms = [("icx-tpn1", "ICX tpn=1"), ("icx-socket", "ICX socket")]
    timesteps = [100, 1000, 10000]
    base_csv, pen_csv = [], []
    pack: dict[str, dict] = {}
    for plat, _lab in platforms:
        pack[plat] = {"meas_w": [], "sim_w": [], "pen_w": [], "meas_m": [], "sim_m": [], "pen_m": [],
                       "meas_g": [], "sim_g": [], "pen_g": [], "meas_b": [], "sim_b": [], "pen_b": []}
        for nt in timesteps:
            cfg = f"{plat}-{nt}"
            m = meas.get(cfg)
            s = sim_wall(TC_ROOT / f"stage3-multi-timestep/results/simulated/{cfg}")
            p = sim_wall(TC_ROOT / f"stage3-multi-timestep/results/simulated-penalty/{cfg}")
            if not m or s is None:
                print(f"  skip {cfg}: missing base data")
                continue
            sr = _heat_rates({**m, "ntimes": nt}, s)
            pack[plat]["meas_w"].append(m["wall_s"])
            pack[plat]["sim_w"].append(s)
            pack[plat]["pen_w"].append(p)
            pack[plat]["meas_m"].append(m["mlup_s"])
            pack[plat]["sim_m"].append(sr["mlup_s"])
            pack[plat]["pen_m"].append(None if p is None else _heat_rates({**m, "ntimes": nt}, p)["mlup_s"])
            pack[plat]["meas_g"].append(m["gf_s"])
            pack[plat]["sim_g"].append(sr["gf_s"])
            pack[plat]["pen_g"].append(None if p is None else _heat_rates({**m, "ntimes": nt}, p)["gf_s"])
            pack[plat]["meas_b"].append(m["gb_s"])
            pack[plat]["sim_b"].append(sr["gb_s"])
            pack[plat]["pen_b"].append(None if p is None else _heat_rates({**m, "ntimes": nt}, p)["gb_s"])
            base_csv.append(csv_row(cfg, m["wall_s"], s, m, sr))
            if p is not None:
                pen_csv.append(csv_row(cfg, m["wall_s"], p, m, _heat_rates({**m, "ntimes": nt}, p)))
            print(f"  {cfg}: meas={m['wall_s']:.2f} sim={s:.2f}" + (f" pen={p:.2f}" if p else ""))

    paths: list[Path] = []
    metric_map = [
        ("wall", "Wall time [s]", "meas_w", "sim_w", "pen_w", "s"),
        ("mlup", "Performance [MLUP/s]", "meas_m", "sim_m", "pen_m", ""),
        ("gf", "Performance [GF/s]", "meas_g", "sim_g", "pen_g", ""),
        ("gb", "Bandwidth [GB/s]", "meas_b", "sim_b", "pen_b", ""),
    ]
    usable = [p for p, _ in platforms if pack[p]["meas_w"]]
    if not usable:
        print("stage3: insufficient points, skip")
        return paths

    def emit_s3(outdir: Path, include_penalty: bool) -> None:
        none_pen = [None] * max(len(timesteps), 1)
        for key, ylabel, mk, sk, pk, val_sfx in metric_map:
            fig, axes = plt.subplots(1, len(usable), figsize=(5.4 * len(usable), 5.15), squeeze=False)
            for ax, plat in zip(axes[0], usable):
                n = len(pack[plat][mk])
                py = pack[plat][pk] if include_penalty else none_pen[:n]
                draw_s3_panel(
                    ax, timesteps[:n], pack[plat][mk], pack[plat][sk], ylabel,
                    S3_PANEL_TITLE.get(plat, plat),
                    value_suffix=val_sfx,
                    pen_y=py if include_penalty else None,
                    loglog=(key == "wall"),
                )
            finish_s3_combined(fig, include_penalty=include_penalty)
            out = outdir / f"stage3-{key}.png"
            save_figure(fig, out)
            paths.append(out)
            if key == "wall" and not include_penalty:
                alias = outdir / "stage3-sim-vs-measured.png"
                alias.write_bytes(out.read_bytes())
                print(f"  wrote {alias}")
                paths.append(alias)
            for plat in usable:
                n = len(pack[plat][mk])
                py = pack[plat][pk] if include_penalty else none_pen[:n]
                if include_penalty and not any(v is not None for v in py):
                    continue
                fig_s, ax_s = plt.subplots(figsize=(6.4, 5.1))
                draw_s3_panel(
                    ax_s, timesteps[:n], pack[plat][mk], pack[plat][sk],
                    ylabel, S3_PANEL_TITLE.get(plat, plat),
                    value_suffix=val_sfx,
                    pen_y=py if include_penalty else None,
                    loglog=(key == "wall"),
                )
                fig_s.subplots_adjust(top=0.82, left=0.12, right=0.88, bottom=0.12)
                fig_s.legend(
                    handles=s3_legend_handles(include_penalty),
                    loc="upper center", ncol=4 if include_penalty else 3,
                    bbox_to_anchor=(0.5, 0.98), frameon=False, fontsize=9,
                )
                solo = outdir / f"stage3-{plat}-{key}.png"
                save_figure(fig_s, solo)
                paths.append(solo)

    emit_s3(fig_dir, include_penalty=False)
    if any(v is not None for plat in usable for v in pack[plat]["pen_w"]):
        emit_s3(fig_pen, include_penalty=True)
    if base_csv:
        write_metric_csv(fig_dir / "stage3_data.csv", base_csv, CSV_HEADER)
        paths.append(fig_dir / "stage3_data.csv")
    if pen_csv:
        write_metric_csv(fig_pen / "stage3_data.csv", pen_csv, CSV_HEADER)
        paths.append(fig_pen / "stage3_data.csv")
    return paths


def make_stage4() -> list[Path]:
    print("  Stage 4: no Gantt PNG (use Intel Trace Analyzer on STF).")
    fig_dir = TC_ROOT / "stage4-idlewave/figures"
    fig_pen = TC_ROOT / "stage4-idlewave/figures-penalty"
    fig_dir.mkdir(parents=True, exist_ok=True)
    meas = parse_measured_tsv(TC_ROOT / "stage4-idlewave/results/measured/stage4-measured.tsv")
    s = sim_wall(TC_ROOT / "stage4-idlewave/results/simulated/icx")
    p = sim_wall(TC_ROOT / "stage4-idlewave/results/simulated-penalty/icx")
    m = meas.get("icx")
    paths: list[Path] = []
    if m and s is not None:
        paths.extend(
            emit_metric_set(
                fig_dir, "stage4", ["ICX idlewave"], [m], [s], [p], "Stage 4", include_penalty=False
            )
        )
        write_metric_csv(fig_dir / "stage4_metrics.csv", [csv_row("icx", m["wall_s"], s, m, _heat_rates(m, s))], CSV_HEADER)
        paths.append(fig_dir / "stage4_metrics.csv")
        if p is not None:
            paths.extend(
                emit_metric_set(
                    fig_pen, "stage4", ["ICX idlewave"], [m], [s], [p], "Stage 4", include_penalty=True
                )
            )
            write_metric_csv(fig_pen / "stage4_metrics.csv", [csv_row("icx", m["wall_s"], p, m, _heat_rates(m, p))], CSV_HEADER)
            paths.append(fig_pen / "stage4_metrics.csv")
        print(f"  stage4: meas={m['wall_s']:.3f} sim={s:.3f}" + (f" pen={p:.3f}" if p else ""))
    for label, path in (
        ("STF measured", TC_ROOT / "stage4-idlewave/results/measured-trace/icx/heat-idlewave-measured.single.stf"),
        ("STF base", TC_ROOT / "stage4-idlewave/results/simulated/icx/heat-idlewave.single.stf"),
        ("STF penalty", TC_ROOT / "stage4-idlewave/results/simulated-penalty/icx/heat-idlewave-penalty.single.stf"),
    ):
        print(f"  {label}: {path}" + ("  [OK]" if path.is_file() else "  [MISSING]"))
    return paths


def make_stage5() -> list[Path]:
    apply_thesis_style()
    fig_dir = TC_ROOT / "stage5-scaling/figures"
    fig_pen = TC_ROOT / "stage5-scaling/figures-penalty"
    meas = parse_measured_tsv(TC_ROOT / "stage5-scaling/results/measured/stage5-measured.tsv")
    nodes_all = (1, 2, 4, 8, 16, 32)
    series: dict[str, dict] = {}
    base_csv, pen_csv = [], []
    for kind in ("strong", "weak"):
        series[kind] = {k: [] for k in ("n", "meas_w", "sim_w", "pen_w", "meas_m", "sim_m", "pen_m",
                                        "meas_g", "sim_g", "pen_g", "meas_b", "sim_b", "pen_b",
                                        "meas_e", "sim_e", "pen_e")}
        walls_meas, walls_sim, walls_pen = {}, {}, {}
        for nodes in nodes_all:
            name = f"{kind}-icx-{nodes}node"
            m = meas.get(name)
            s = sim_wall(TC_ROOT / f"stage5-scaling/results/simulated/{name}")
            p = sim_wall(TC_ROOT / f"stage5-scaling/results/simulated-penalty/{name}")
            if not m or s is None:
                print(f"  skip {name}: missing base data")
                continue
            walls_meas[nodes] = m["wall_s"]
            walls_sim[nodes] = s
            if p is not None:
                walls_pen[nodes] = p
            sr = _heat_rates(m, s)
            series[kind]["n"].append(nodes)
            series[kind]["meas_w"].append(m["wall_s"])
            series[kind]["sim_w"].append(s)
            series[kind]["pen_w"].append(p)
            series[kind]["meas_m"].append(m["mlup_s"])
            series[kind]["sim_m"].append(sr["mlup_s"])
            series[kind]["pen_m"].append(None if p is None else _heat_rates(m, p)["mlup_s"])
            series[kind]["meas_g"].append(m["gf_s"])
            series[kind]["sim_g"].append(sr["gf_s"])
            series[kind]["pen_g"].append(None if p is None else _heat_rates(m, p)["gf_s"])
            series[kind]["meas_b"].append(m["gb_s"])
            series[kind]["sim_b"].append(sr["gb_s"])
            series[kind]["pen_b"].append(None if p is None else _heat_rates(m, p)["gb_s"])
            base_csv.append(csv_row(name, m["wall_s"], s, m, sr))
            if p is not None:
                pen_csv.append(csv_row(name, m["wall_s"], p, m, _heat_rates(m, p)))
            print(f"  {name}: meas={m['wall_s']:.2f} sim={s:.2f}" + (f" pen={p:.2f}" if p else ""))
        t1m, t1s, t1p = walls_meas.get(1), walls_sim.get(1), walls_pen.get(1)
        for nodes in series[kind]["n"]:
            mv, sv, pv = walls_meas[nodes], walls_sim[nodes], walls_pen.get(nodes)
            if kind == "strong":
                series[kind]["meas_e"].append(dmetrics.strong_efficiency(t1m or 0, mv, nodes))
                series[kind]["sim_e"].append(dmetrics.strong_efficiency(t1s or 0, sv, nodes))
                series[kind]["pen_e"].append(
                    None if pv is None or not t1p else dmetrics.strong_efficiency(t1p, pv, nodes)
                )
            else:
                series[kind]["meas_e"].append(dmetrics.weak_efficiency(t1m or 0, mv))
                series[kind]["sim_e"].append(dmetrics.weak_efficiency(t1s or 0, sv))
                series[kind]["pen_e"].append(
                    None if pv is None or not t1p else dmetrics.weak_efficiency(t1p, pv)
                )
        # attach efficiency to CSV by rewriting last 4 rows of this kind — keep efficiency in a sidecar instead
    # rewrite base_csv with efficiency columns
    header_e = CSV_HEADER[:-1] + ["meas_efficiency", "sim_efficiency", "formula"]
    base_rows_e, pen_rows_e = [], []
    idx = 0
    pen_idx = 0
    for kind in ("strong", "weak"):
        for i, nodes in enumerate(series[kind]["n"]):
            row = base_csv[idx]
            idx += 1
            base_rows_e.append(row[:-1] + [f"{series[kind]['meas_e'][i]:.4f}", f"{series[kind]['sim_e'][i]:.4f}", row[-1]])
            if series[kind]["pen_w"][i] is not None:
                prow = pen_csv[pen_idx]
                pen_idx += 1
                pe = series[kind]["pen_e"][i]
                pen_rows_e.append(
                    prow[:-1]
                    + [
                        f"{series[kind]['meas_e'][i]:.4f}",
                        f"{pe:.4f}" if pe is not None else "",
                        prow[-1],
                    ]
                )

    paths: list[Path] = []

    def s5_legend_handles(include_penalty: bool):
        handles = [
            Line2D([0], [0], color=C_MEAS, marker="s", lw=1.8, markersize=7, label=LEGEND_MEAS),
            Line2D([0], [0], color=C_SIM, marker="o", lw=1.8, markersize=7, label=LEGEND_SIM),
        ]
        if include_penalty:
            handles.append(Line2D([0], [0], color=C_PEN, marker="D", lw=1.8, markersize=7, label=LEGEND_PEN))
        return handles

    def line_kind(ax, kind, ymeas, ysim, ypen, ylabel, title, *, zero_y: bool, log_y: bool, annotate: bool):
        xs = series[kind]["n"]
        ax.plot(xs, ymeas, "s-", color=C_MEAS, lw=1.8, markersize=7)
        ax.plot(xs, ysim, "o-", color=C_SIM, lw=1.8, markersize=7)
        if ypen is not None and any(v is not None for v in ypen):
            px = [x for x, v in zip(xs, ypen) if v is not None]
            py = [v for v in ypen if v is not None]
            ax.plot(px, py, "D-", color=C_PEN, lw=1.8, markersize=7)
        ax.set_xscale("log", base=2)
        shown = [n for n in nodes_all if n in set(xs)]
        ax.set_xticks(shown)
        ax.xaxis.set_major_formatter(mticker.FuncFormatter(lambda v, _: f"{int(round(v))}"))
        ax.xaxis.set_minor_locator(mticker.NullLocator())
        ax.xaxis.set_minor_formatter(mticker.NullFormatter())
        ax.set_xlabel("# of nodes")
        ax.set_ylabel(ylabel)
        ax.set_title(title + (" · log–log" if log_y else ""), fontsize=10)
        ax.grid(True, alpha=0.3)
        live = list(ymeas) + list(ysim)
        if ypen is not None:
            live += [v for v in ypen if v is not None]
        if log_y:
            vals = [v for v in live if v is not None and v > 0]
            ax.set_yscale("log")
            if vals:
                lo, hi = min(vals), max(vals)
                ypad = (hi / lo) ** 0.12 if hi > lo else 1.8
                ax.set_ylim(lo / ypad, hi * ypad)
        elif zero_y:
            ymax = max(live, default=0.0)
            ax.set_ylim(0, ymax * 1.22 if ymax > 0 else 1)
        if annotate:
            annotate_line_errors(ax, xs, ymeas, ysim, ypen)

    metric_specs = [
        ("wall", "Wall time [s]", "meas_w", "sim_w", "pen_w"),
        ("mlup", "Performance [MLUP/s]", "meas_m", "sim_m", "pen_m"),
        ("gf", "Performance [GF/s]", "meas_g", "sim_g", "pen_g"),
        ("gb", "Bandwidth [GB/s]", "meas_b", "sim_b", "pen_b"),
        ("efficiency", "Efficiency", "meas_e", "sim_e", "pen_e"),
    ]

    def emit_s5(outdir: Path, include_penalty: bool) -> None:
        for key, ylabel, mk, sk, pk in metric_specs:
            fig, axes = plt.subplots(1, 2, figsize=(11.2, 5.0))
            for ax, kind, title in (
                (axes[0], "strong", f"Strong scaling — {ylabel}"),
                (axes[1], "weak", f"Weak scaling — {ylabel}"),
            ):
                if not series[kind]["n"]:
                    ax.set_visible(False)
                    continue
                ypen = series[kind][pk] if include_penalty else None
                line_kind(
                    ax, kind, series[kind][mk], series[kind][sk], ypen, ylabel, title,
                    zero_y=(key != "efficiency"),
                    log_y=(key in ("mlup", "gf", "gb") or (key == "wall" and kind == "strong")),
                    annotate=True,
                )
                if key == "efficiency":
                    ax.axhline(1.0, color="#888", ls="--", lw=0.8)
                    ax.set_ylim(0, 1.25)
            fig.subplots_adjust(top=0.82, left=0.07, right=0.99, bottom=0.12, wspace=0.28)
            fig.suptitle(f"Stage 5 — {ylabel} · {comparison_subtitle(include_penalty)}", fontsize=12, y=0.98)
            fig.legend(
                handles=s5_legend_handles(include_penalty),
                loc="upper center", ncol=3, bbox_to_anchor=(0.5, 0.92),
                frameon=False, fontsize=9,
            )
            out = outdir / f"stage5-{key}.png"
            save_figure(fig, out)
            paths.append(out)
            for kind, title in (
                ("strong", f"Strong scaling — {ylabel}"),
                ("weak", f"Weak scaling — {ylabel}"),
            ):
                if not series[kind]["n"]:
                    continue
                fig_s, ax_s = plt.subplots(figsize=(6.2, 4.6))
                ypen = series[kind][pk] if include_penalty else None
                line_kind(
                    ax_s, kind, series[kind][mk], series[kind][sk], ypen, ylabel, title,
                    zero_y=(key != "efficiency"),
                    log_y=(key in ("mlup", "gf", "gb") or (key == "wall" and kind == "strong")),
                    annotate=True,
                )
                if key == "efficiency":
                    ax_s.axhline(1.0, color="#888", ls="--", lw=0.8)
                    ax_s.set_ylim(0, 1.25)
                ax_s.legend(handles=s5_legend_handles(include_penalty), fontsize=8)
                solo = outdir / f"stage5-{kind}-{key}.png"
                save_figure(fig_s, solo)
                paths.append(solo)

    emit_s5(fig_dir, include_penalty=False)
    if any(v is not None for kind in ("strong", "weak") for v in series[kind]["pen_w"]):
        emit_s5(fig_pen, include_penalty=True)

    if base_rows_e:
        write_metric_csv(fig_dir / "stage5_scaling_wall.csv", base_rows_e, header_e)
        paths.append(fig_dir / "stage5_scaling_wall.csv")
    if pen_rows_e:
        write_metric_csv(fig_pen / "stage5_scaling_wall.csv", pen_rows_e, header_e)
        paths.append(fig_pen / "stage5_scaling_wall.csv")
    return paths


def main() -> None:
    ap = argparse.ArgumentParser(
        description="Generate heat-demo figures (Stages 1–5; figures/ base, figures-penalty/ three-series)"
    )
    ap.add_argument("--stages", default="1,2,3,4,5", help="Comma-separated stages (default: 1,2,3,4,5)")
    args = ap.parse_args()
    stages = {s.strip() for s in args.stages.split(",") if s.strip()}
    if "1" in stages:
        make_stage1()
    if "2" in stages:
        make_stage2()
    if "3" in stages:
        make_stage3()
    if "4" in stages:
        make_stage4()
    if "5" in stages:
        make_stage5()


if __name__ == "__main__":
    main()
