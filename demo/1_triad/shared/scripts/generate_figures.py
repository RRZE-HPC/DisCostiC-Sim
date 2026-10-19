#!/usr/bin/env python3
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: STREAM Triad demo figures (Stages 1–5). figures/ = MPI -O3 vs DisCostiC -O3;
#          figures-o1/ = MPI/DisCostiC -O3 and -O1 (Stages 2–3). Stage 4: ITAC STF.

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

try:
    from PIL import Image
except ImportError:
    Image = None  # type: ignore
    import numpy as np  # used only if PIL missing in verify_outputs
else:
    np = None  # type: ignore

SCRIPT = Path(__file__).resolve()
TC_ROOT = SCRIPT.parents[2]  # demo/1_triad/
REPO_ROOT = TC_ROOT  # stage-local paths are relative to the demo root
DEMO_ROOT = TC_ROOT.parent
if str(DEMO_ROOT) not in sys.path:
    sys.path.insert(0, str(DEMO_ROOT))
import metrics as dmetrics  # noqa: E402

# ── Established palette (matches doc/figures methodology) ─────────────────────
ECM_COLORS = {
    "T_comp": "#f0c040",
    "T_RegL1": "#a8d8a8",
    "T_L1L2": "#a8c8e8",
    "T_L2L3": "#c8a8e8",
    "T_L3Mem": "#e8c8c8",
}
C_ECM_LINE = "#2166ac"
C_MEASURED = "#d73027"
C_SIM = "#3070b0"
C_MEAS = "#d04040"
C_MEAS_O1 = "#e8a838"
C_ERR = "#b0b0b0"
C_O1 = "#2e7d4f"

# Measured 1-socket STREAM bandwidth (Stage 1 reference; stage1-ecm-modeling.md)
MEASURED_BW_GBS = {"ICX": 328.5, "SPR": 402.3}


# Fallbacks match live STREAM_FILE_ecm.cfg (stage2 simulated / simulated-o1).
ECM_DATA = {
    "ICX": {"T_comp": 1.3, "T_RegL1": 1.0, "T_L1L2": 4.0, "T_L2L3": 12.0, "T_L3Mem": 5.5},
    "SPR": {"T_comp": 1.1, "T_RegL1": 0.0, "T_L1L2": 4.0, "T_L2L3": 12.0, "T_L3Mem": 7.2},
}
ECM_DATA_O3 = ECM_DATA
ECM_DATA_O1 = {
    "ICX": {"T_comp": 8.0, "T_RegL1": 8.0, "T_L1L2": 4.0, "T_L2L3": 12.0, "T_L3Mem": 5.5},
    "SPR": {"T_comp": 8.0, "T_RegL1": 0.0, "T_L1L2": 4.0, "T_L2L3": 12.0, "T_L3Mem": 7.2},
}


LEGEND_SIM = "DisCostiC -O3"
LEGEND_MEAS = "MPI -O3"
LEGEND_SIM_O1 = "DisCostiC -O1"
LEGEND_MEAS_O1 = "MPI -O1"
LEGEND_O1 = LEGEND_SIM_O1
LEGEND_ERR = "|% error|"

STAGE1_MEAS_LABELS = {
    "icx-socket-100": "ICX socket",
    "spr-socket-100": "SPR socket",
    "icx-tpn1-100": "ICX tpn=1",
}


def opt_title_tag(opt: str | None) -> str:
    """Return a short compiler-opt tag for figure titles (e.g. ' · -O3')."""
    if not opt:
        return ""
    return f" · -{opt}" if not opt.startswith("-") else f" · {opt}"


def comparison_handles() -> list:
    return [
        mpatches.Patch(color=C_SIM, label=LEGEND_SIM),
        mpatches.Patch(color=C_MEAS, label=LEGEND_MEAS),
    ]





def save_figure(fig, path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(path, dpi=160, bbox_inches="tight", facecolor="white")
    plt.close(fig)
    print(f"  wrote {path}")




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


def t_ecm(vals: dict) -> float:
    data = vals["T_RegL1"] + vals["T_L1L2"] + vals["T_L2L3"] + vals["T_L3Mem"]
    return max(vals["T_comp"], data)


def maybe_load_ecm_from_cfg(path: Path, fallback: dict) -> dict:
    """Override ECM terms from a live STREAM_FILE_ecm.cfg if present."""
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


def draw_ecm_stack(ax, arch: str, vals: dict, *, subtitle: str = "") -> None:
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
    ax.set_title(title, fontsize=10)
    ax.set_ylabel("ECM [cy/CL]")
    ax.set_ylim(0, max(bottom, t_comp, tecm) * 1.28)
    ax.legend(fontsize=7, loc="upper right")
    ax.grid(True, axis="y", alpha=0.3)


def make_stage1() -> list[Path]:
    """Stage 1 ECM breakdown plus the -O3 / -O1 comparison."""
    apply_thesis_style()
    fig_dir = TC_ROOT / "stage1-ecm-modeling/figures"
    paths: list[Path] = []

    icx_o3_cfg = TC_ROOT / "stage2-single-timestep/results/simulated/icx-socket-100/STREAM_FILE_ecm.cfg"
    spr_o3_cfg = TC_ROOT / "stage2-single-timestep/results/simulated/spr-socket-100/STREAM_FILE_ecm.cfg"
    icx_o1_cfg = TC_ROOT / "stage2-single-timestep/results/simulated-o1/icx-socket-100/STREAM_FILE_ecm.cfg"
    spr_o1_cfg = TC_ROOT / "stage2-single-timestep/results/simulated-o1/spr-socket-100/STREAM_FILE_ecm.cfg"
    log_source("ECM ICX -O3", icx_o3_cfg)
    log_source("ECM SPR -O3", spr_o3_cfg)
    log_source("ECM ICX -O1", icx_o1_cfg)
    log_source("ECM SPR -O1", spr_o1_cfg)
    icx_o3 = maybe_load_ecm_from_cfg(icx_o3_cfg, ECM_DATA_O3["ICX"])
    spr_o3 = maybe_load_ecm_from_cfg(spr_o3_cfg, ECM_DATA_O3["SPR"])
    icx_o1 = maybe_load_ecm_from_cfg(icx_o1_cfg, ECM_DATA_O1["ICX"])
    spr_o1 = maybe_load_ecm_from_cfg(spr_o1_cfg, ECM_DATA_O1["SPR"])

    ecm_o3 = {"ICX": icx_o3, "SPR": spr_o3}
    ecm_o1 = {"ICX": icx_o1, "SPR": spr_o1}

    combined = fig_dir / "stage1-ecm-breakdown.png"
    fig, axes = plt.subplots(1, 2, figsize=(10.5, 5.2))
    for ax, arch in zip(axes, ("ICX", "SPR")):
        draw_ecm_stack(ax, arch, ecm_o3[arch], subtitle="Kerncraft @ -O3")
    fig.suptitle("Stage 1 — Simulated ECM breakdown (STREAM Triad) · -O3", fontsize=12)
    fig.tight_layout()
    save_figure(fig, combined)
    paths.append(combined)

    for arch in ("ICX", "SPR"):
        solo = fig_dir / f"stage1-ecm-breakdown-{arch.lower()}.png"
        fig_s, ax_s = plt.subplots(figsize=(5.2, 5.0))
        draw_ecm_stack(ax_s, arch, ecm_o3[arch], subtitle="-O3")
        fig_s.tight_layout()
        save_figure(fig_s, solo)
        paths.append(solo)

    cmp_path = fig_dir / "stage1-ecm-o3-vs-o1.png"
    fig, axes = plt.subplots(2, 2, figsize=(10.5, 9.4))
    draw_ecm_stack(axes[0, 0], "ICX", icx_o3, subtitle=f"-O3  (T_OL={icx_o3['T_comp']:g})")
    draw_ecm_stack(axes[0, 1], "SPR", spr_o3, subtitle=f"-O3  (T_OL={spr_o3['T_comp']:g})")
    draw_ecm_stack(axes[1, 0], "ICX", icx_o1, subtitle=f"-O1  (T_OL={icx_o1['T_comp']:g})")
    draw_ecm_stack(axes[1, 1], "SPR", spr_o1, subtitle=f"-O1  (T_OL={spr_o1['T_comp']:g})")
    fig.suptitle(
        "Stage 1 ECM — overlapping model, T_ECM = max(T_comp, data path)\n"
        "ICX -O1 raises in-core cost so T_ECM tracks tpn=1; SPR stays data-path bound",
        fontsize=11,
    )
    fig.tight_layout()
    save_figure(fig, cmp_path)
    paths.append(cmp_path)

    csv_path = fig_dir / "stage1_ecm_data.csv"
    with csv_path.open("w", newline="") as handle:
        writer = csv.writer(handle)
        writer.writerow(["opt", "arch", "level", "cy_per_CL"])
        for opt, table in (("O3", ecm_o3), ("O1", ecm_o1)):
            for arch, vals in table.items():
                for seg in ("T_comp", "T_RegL1", "T_L1L2", "T_L2L3", "T_L3Mem"):
                    writer.writerow([opt, arch, seg, vals[seg]])
                writer.writerow([opt, arch, "T_ECM", t_ecm(vals)])
    print(f"  wrote {csv_path}")
    paths.append(csv_path)

    meas = parse_measured_tsv(TC_ROOT / "stage1-ecm-modeling/results/measured/stage1-measured.tsv")
    if meas:
        keys = [k for k in ("icx-tpn1-100", "icx-socket-100", "spr-socket-100") if k in meas]
        if not keys:
            keys = list(meas.keys())
        labels = [STAGE1_MEAS_LABELS.get(k, k) for k in keys]
        walls = [meas[k]["wall_s"] for k in keys]
        fig, ax = plt.subplots(figsize=(7.2, 4.2))
        ax.bar(labels, walls, color=C_MEAS, edgecolor="#333", width=0.55, zorder=3)
        ax.set_ylabel("Wall time [s]")
        ax.set_title("Stage 1 — Measured wall time (STREAM Triad) · -O3")
        ax.set_ylim(0, max(walls) * 1.18)
        save_figure(fig, fig_dir / "stage1-measured-wall.png")
        paths.append(fig_dir / "stage1-measured-wall.png")
    return paths

# ── Shared parsers / per-metric emit (HEAT-style; O1 is the third series) ─────


def parse_rank_times(slurm_out: Path) -> list[float]:
    """Parse per-rank wall times [s] from DisCostiC FULL APPLICATION tables."""
    if not slurm_out.is_file():
        return []
    text = slurm_out.read_text(errors="replace")
    blocks = text.split("FULL APPLICATION PERFORMANCE (for all ranks):")
    if len(blocks) < 2:
        return []
    for block in reversed(blocks[1:]):
        section = block.split("FULL APPLICATION PERFORMANCE")[0]
        times: list[float] = []
        for line in section.splitlines():
            match = re.match(r"\s*(\d+)\s+\S+\s+([\d.eE+-]+)\s*$", line)
            if match:
                times.append(float(match.group(2)))
        if times:
            return times
    return []


def sim_wall(sim_dir: Path) -> float | None:
    for name in ("slurm-discostic-pass2.out", "slurm-discostic-pass1.out"):
        times = parse_rank_times(sim_dir / name)
        if times:
            return max(times)
    return None


def parse_measured_tsv(path: Path) -> dict[str, dict]:
    rows: dict[str, dict] = {}
    for cfg, row in dmetrics.last_row_tsv(path).items():
        try:
            wall_s = float(row["wall_seconds"])
        except (KeyError, TypeError, ValueError):
            continue
        rates = dmetrics.stream_from_tsv_row(row, wall_s)
        rows[cfg] = {
            "wall_s": wall_s,
            "gflops": rates["gflops"],
            "gb_s": rates["gb_s"],
            "N": rates["N"],
            "ntimes": rates["ntimes"],
            "nprocs": int(float(row.get("nprocs") or 0)),
        }
    return rows


def _stream_rates(row: dict, wall_s: float) -> dict[str, float]:
    return dmetrics.stream_from_tsv_row(
        {"N_total": row.get("N", 0), "ntimes": row.get("ntimes", 0)},
        wall_s,
    )


def fmt_err_pct(sim, meas) -> str | None:
    """Scoring error (sim − meas) / meas as a signed percentage label."""
    if sim is None or meas is None or meas == 0:
        return None
    return f"{dmetrics.signed_error_pct(sim, meas):+.1f}%"


def group_error_labels(meas_vals, sim_vals, meas_o1_vals=None, o1_vals=None, include_o1=False):
    meas_o1_vals = meas_o1_vals or [None] * len(meas_vals)
    o1_vals = o1_vals or [None] * len(meas_vals)
    labels = []
    for meas, sim, meas_o1, o1 in zip(meas_vals, sim_vals, meas_o1_vals, o1_vals):
        err_o3 = fmt_err_pct(sim, meas)
        if not include_o1:
            labels.append(err_o3)
            continue
        parts = []
        if err_o3:
            parts.append(f"O3 {err_o3}")
        err_o1 = fmt_err_pct(o1, meas_o1)
        if err_o1:
            parts.append(f"O1 {err_o1}")
        labels.append("\n".join(parts) if parts else None)
    return labels


def annotate_line_errors(ax, xs, meas_y, sim_y, meas_o1_y=None, o1_y=None, include_o1=False):
    for i, x in enumerate(xs):
        meas = meas_y[i] if i < len(meas_y) else None
        sim = sim_y[i] if i < len(sim_y) else None
        err_o3 = fmt_err_pct(sim, meas)
        if err_o3 and sim is not None:
            ax.annotate(
                err_o3, (x, sim), textcoords="offset points", xytext=(0, 8),
                ha="center", fontsize=7, color=C_SIM, zorder=5,
            )
        if not include_o1 or o1_y is None or meas_o1_y is None:
            continue
        o1 = o1_y[i] if i < len(o1_y) else None
        meas_o1 = meas_o1_y[i] if i < len(meas_o1_y) else None
        err_o1 = fmt_err_pct(o1, meas_o1)
        if err_o1 and o1 is not None:
            ax.annotate(
                err_o1, (x, o1), textcoords="offset points", xytext=(0, -11),
                ha="center", fontsize=7, color=C_O1, zorder=5,
            )


def log_source(kind: str, path: Path) -> None:
    ok = path.is_file() if path.suffix else path.is_dir()
    print(f"  source {kind}: {path}  [{'OK' if ok else 'MISSING'}]")


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
            ax.text(
                x, v, f"{v:.2f}" if v < 100 else f"{v:.1f}",
                ha="center", va="bottom", fontsize=6 if nser >= 4 else 7,
            )
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


FORMULA = (
    "Gflops=2*N*ntimes/wall/1e9; WA GB/s=4*8*N*ntimes/wall/1e9; "
    "wall_err=(sim-meas)/meas; rate_err=-wall_err/(1+wall_err)"
)
CSV_HEADER = [
    "config",
    "meas_wall_s",
    "sim_wall_s",
    "wall_err_pct",
    "meas_gflops",
    "sim_gflops",
    "meas_gb_s",
    "sim_gb_s",
    "rate_err_pct",
    "formula",
]
CSV_HEADER_O1 = CSV_HEADER[:-1] + [
    "meas_o1_wall_s",
    "sim_o1_wall_s",
    "wall_err_o1_pct",
    "meas_o1_gflops",
    "sim_o1_gflops",
    "meas_o1_gb_s",
    "sim_o1_gb_s",
    "rate_err_o1_pct",
    "formula",
]


def csv_row(cfg: str, meas_s: float, sim_s: float, meas_r: dict, sim_r: dict) -> list:
    wall_err = dmetrics.signed_error_pct(sim_s, meas_s)
    rerr = dmetrics.signed_error_pct(sim_r["gflops"], meas_r["gflops"])
    return [
        cfg,
        f"{meas_s:.6f}",
        f"{sim_s:.6f}",
        f"{wall_err:.4f}",
        f"{meas_r['gflops']:.4f}",
        f"{sim_r['gflops']:.4f}",
        f"{meas_r['gb_s']:.4f}",
        f"{sim_r['gb_s']:.4f}",
        f"{rerr:.4f}",
        FORMULA,
    ]


def csv_row_o1(cfg: str, meas_s: float, sim_s: float, meas_r: dict, sim_r: dict, m1, o) -> list:
    base = csv_row(cfg, meas_s, sim_s, meas_r, sim_r)[:-1]
    if m1 is not None and o is not None:
        o1r = _stream_rates(m1, o)
        werr = dmetrics.signed_error_pct(o, m1["wall_s"])
        rerr = dmetrics.signed_error_pct(o1r["gflops"], m1["gflops"])
        extra = [
            f"{m1['wall_s']:.6f}",
            f"{o:.6f}",
            f"{werr:.4f}",
            f"{m1['gflops']:.4f}",
            f"{o1r['gflops']:.4f}",
            f"{m1['gb_s']:.4f}",
            f"{o1r['gb_s']:.4f}",
            f"{rerr:.4f}",
        ]
    else:
        extra = [
            f"{m1['wall_s']:.6f}" if m1 else "",
            f"{o:.6f}" if o is not None else "",
            "",
            f"{m1['gflops']:.4f}" if m1 else "",
            f"{_stream_rates(m1 or meas_r, o)['gflops']:.4f}" if o is not None else "",
            f"{m1['gb_s']:.4f}" if m1 else "",
            f"{_stream_rates(m1 or meas_r, o)['gb_s']:.4f}" if o is not None else "",
            "",
        ]
    return base + extra + [FORMULA]


def comparison_subtitle(include_o1: bool) -> str:
    return "MPI/DisCostiC · -O3 vs -O1" if include_o1 else "meas vs sim · -O3"


def emit_metric_set(
    fig_dir: Path,
    stem: str,
    labels,
    meas,
    sim,
    o1,
    title_prefix: str,
    *,
    include_o1: bool,
    meas_o1=None,
) -> list[Path]:
    """One PNG per metric; grouped bars. figures-o1 adds MPI -O1 and DisCostiC -O1."""
    paths: list[Path] = []
    if meas_o1 is None:
        meas_o1 = [None] * len(meas)
    specs = [
        ("wall", "Wall time [s]", True),
        ("gflops", "Performance [GFlop/s]", False),
        ("gb", "Bandwidth [GB/s] (write-allocate)", False),
    ]
    has_o1 = include_o1 and (
        any(v is not None for v in o1) or any(v is not None for v in meas_o1)
    )
    for key, ylabel, is_wall in specs:
        meas_vals, sim_vals, meas_o1_vals, o1_vals = [], [], [], []
        for m, s, m1, o in zip(meas, sim, meas_o1, o1):
            if m is None:
                meas_vals.append(None)
                sim_vals.append(None)
                meas_o1_vals.append(None)
                o1_vals.append(None)
                continue
            if is_wall:
                meas_vals.append(m["wall_s"])
                sim_vals.append(None if s is None else s)
                meas_o1_vals.append(None if m1 is None else m1["wall_s"])
                o1_vals.append(None if o is None else o)
            else:
                rk = "gflops" if key == "gflops" else "gb_s"
                meas_vals.append(m[rk])
                sim_vals.append(None if s is None else _stream_rates(m, s)[rk])
                meas_o1_vals.append(None if m1 is None else m1[rk])
                src = m1 if m1 is not None else m
                o1_vals.append(None if o is None else _stream_rates(src, o)[rk])
        fig, ax = plt.subplots(figsize=(max(7.2, 2.8 * len(labels)), 5.0 if has_o1 else 4.8))
        series = [(LEGEND_MEAS, meas_vals, C_MEAS), (LEGEND_SIM, sim_vals, C_SIM)]
        if has_o1:
            series.append((LEGEND_MEAS_O1, meas_o1_vals, C_MEAS_O1))
            series.append((LEGEND_SIM_O1, o1_vals, C_O1))
        grouped_bars(
            ax,
            labels,
            series,
            ylabel,
            f"{title_prefix} — {ylabel} · {comparison_subtitle(include_o1)}",
            group_errors=group_error_labels(
                meas_vals, sim_vals, meas_o1_vals, o1_vals, include_o1=has_o1
            ),
        )
        out = fig_dir / f"{stem}-{key}.png"
        save_figure(fig, out)
        paths.append(out)
    return paths


def log_stf(label: str, path: Path) -> None:
    print(f"  {label}: {path}" + ("  [OK]" if path.is_file() else "  [MISSING]"))


def make_stage2() -> list[Path]:
    apply_thesis_style()
    fig_dir = TC_ROOT / "stage2-single-timestep/figures"
    fig_o1 = TC_ROOT / "stage2-single-timestep/figures-o1"
    meas_tsv = TC_ROOT / "stage2-single-timestep/results/measured/stage2-measured-100.tsv"
    meas_o1_tsv = TC_ROOT / "stage2-single-timestep/results/measured-o1/stage2-measured-100.tsv"
    log_source("measured TSV (O3)", meas_tsv)
    log_source("measured TSV (O1)", meas_o1_tsv)
    meas = parse_measured_tsv(meas_tsv)
    meas_o1 = parse_measured_tsv(meas_o1_tsv)
    configs = [
        ("icx-tpn1-100", "ICX tpn=1\n(36 nodes)"),
        ("icx-socket-100", "ICX socket\n(1 node)"),
        ("spr-socket-100", "SPR socket\n(1 node)"),
    ]
    labels, cfgs, meas_rows, meas_o1_rows, sim_w, o1_w = [], [], [], [], [], []
    base_csv, o1_csv = [], []
    for cfg, lab in configs:
        m = meas.get(cfg)
        sim_dir = TC_ROOT / "stage2-single-timestep/results/simulated" / cfg
        o1_dir = TC_ROOT / "stage2-single-timestep/results/simulated-o1" / cfg
        s = sim_wall(sim_dir)
        o = sim_wall(o1_dir)
        log_source(f"sim-O3 {cfg}", sim_dir / "slurm-discostic-pass2.out")
        log_source(f"sim-O1 {cfg}", o1_dir / "slurm-discostic-pass2.out")
        if not m or s is None:
            print(f"  skip {cfg}: missing base data")
            continue
        labels.append(lab)
        cfgs.append(cfg)
        meas_rows.append(m)
        sim_w.append(s)
        o1_w.append(o)
        m1 = meas_o1.get(cfg)
        meas_o1_rows.append(m1)
        sr = _stream_rates(m, s)
        row = csv_row(cfg, m["wall_s"], s, m, sr)
        base_csv.append(row)
        o1_csv.append(csv_row_o1(cfg, m["wall_s"], s, m, sr, m1, o))
        print(f"  {cfg}: meas={m['wall_s']:.3f} sim={s:.3f}" + (f" o1={o:.3f}" if o else ""))
        log_stf(f"STF sim {cfg}", TC_ROOT / "stage2-single-timestep/results/simulated" / cfg / f"{cfg}.single.stf")
        log_stf(
            f"STF sim-O1 {cfg}",
            TC_ROOT / "stage2-single-timestep/results/simulated-o1" / cfg / f"{cfg}-O1.single.stf",
        )
    if not labels:
        return []
    paths: list[Path] = []
    specs = [
        ("wall", "Wall time [s]", True),
        ("gflops", "Performance [Gflops/s]", False),
        ("gb", "Bandwidth [GB/s] (write-allocate)", False),
    ]

    def emit_s2(outdir: Path, opt: str, rows) -> None:
        """rows: list of (cfg, _m, _s, meas_row_for_err, sim_wall_for_err)."""
        for key, ylabel, is_wall in specs:
            fig, axes = plt.subplots(1, len(rows), figsize=(4.55 * len(rows), 5.15), squeeze=False)
            for ax, (cfg, m, s, m_err, s_err) in zip(axes[0], rows):
                if is_wall:
                    sim_v, meas_v = s_err, m_err["wall_s"]
                    err = dmetrics.signed_error_pct(sim_v, meas_v)
                else:
                    rk = "gflops" if key == "gflops" else "gb_s"
                    meas_v = m_err[rk]
                    sim_v = _stream_rates(m_err, s_err)[rk]
                    err = dmetrics.signed_error_pct(sim_v, meas_v)
                title = S2_PANEL_TITLE.get(cfg, cfg) + _s2_opt_tag(opt)
                draw_s2_panel(ax, sim_v, meas_v, ylabel, title, err, extra_box=None)
            finish_s2_combined(fig, opt)
            out = outdir / f"stage2-{key}.png"
            save_figure(fig, out)
            paths.append(out)
            if key == "gflops" and opt == "O3":
                alias = outdir / "stage2-sim-vs-measured.png"
                alias.write_bytes(out.read_bytes())
                print(f"  wrote {alias}")
                paths.append(alias)
            for cfg, m, s, m_err, s_err in rows:
                fig_s, ax_s = plt.subplots(figsize=(5.4, 4.8))
                if is_wall:
                    sim_v, meas_v = s_err, m_err["wall_s"]
                    err = dmetrics.signed_error_pct(sim_v, meas_v)
                else:
                    rk = "gflops" if key == "gflops" else "gb_s"
                    meas_v = m_err[rk]
                    sim_v = _stream_rates(m_err, s_err)[rk]
                    err = dmetrics.signed_error_pct(sim_v, meas_v)
                extra = f"wall  sim {s_err:.2f}s / meas {m_err['wall_s']:.2f}s"
                stem = cfg.replace("-100", "")
                draw_s2_panel(
                    ax_s, sim_v, meas_v, ylabel,
                    S2_PANEL_TITLE.get(cfg, cfg) + _s2_opt_tag(opt), err, extra_box=extra,
                )
                solo = outdir / f"stage2-{stem}-{key}.png"
                save_figure(fig_s, solo)
                paths.append(solo)

    o3_rows = [(cfg, m, s, m, s) for cfg, m, s in zip(cfgs, meas_rows, sim_w)]
    emit_s2(fig_dir, "O3", o3_rows)
    o1_rows = [
        (cfg, m, s, m1, o)
        for cfg, m, s, m1, o in zip(cfgs, meas_rows, sim_w, meas_o1_rows, o1_w)
        if m1 is not None and o is not None
    ]
    if o1_rows:
        emit_s2(fig_o1, "O1", o1_rows)
    write_metric_csv(fig_dir / "stage2_data.csv", base_csv, CSV_HEADER)
    paths.append(fig_dir / "stage2_data.csv")
    if o1_csv and any(v is not None for v in o1_w):
        write_metric_csv(fig_o1 / "stage2_data.csv", o1_csv, CSV_HEADER_O1)
        paths.append(fig_o1 / "stage2_data.csv")
    return paths


CLASSIC_SIM = "DisCostiC simulation"
CLASSIC_MEAS = "MPI benchmark"

S2_PANEL_TITLE = {
    "icx-tpn1-100": "(a) ICX, task_per_node=1\n(36 ranks × 36 nodes)",
    "icx-socket-100": "(b) ICX, task_per_node=36\n(full socket, 36 ranks)",
    "spr-socket-100": "(c) SPR, task_per_node=52\n(full socket, 52 ranks)",
}
S3_PANEL_TITLE = {
    "icx-tpn1": "(a) ICX, task_per_node=1 (36 ranks)",
    "icx-socket": "(b) ICX, task_per_node=36 (full socket)",
    "spr-socket": "(c) SPR, task_per_node=52 (full socket)",
}


def _fmt_bar(v: float) -> str:
    return f"{v:.2f}" if abs(v) < 100 else f"{v:.1f}"


def draw_s2_panel(ax, sim_v, meas_v, ylabel, title, err_pct, extra_box: str | None = None) -> None:
    """Old Stage 2 style: sim (blue) left, meas (red) right, Error box on top."""
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


def draw_s3_panel(ax, ts, meas_y, sim_y, ylabel, title, *, value_suffix: str, loglog: bool = False) -> None:
    """Stage 3 lines plus |% error| bars. Wall uses log–log; rates stay linear."""
    xs = [float(t) for t in ts] if loglog else list(range(len(ts)))
    errs = [abs(dmetrics.signed_error_pct(s, m)) for m, s in zip(meas_y, sim_y)]
    ax2 = ax.twinx()
    width = [x * 0.28 for x in xs] if loglog else 0.5
    ax2.bar(xs, errs, width=width, color="#d0d0d0", edgecolor="#9a9a9a", zorder=1, label="|% error|")
    emax = max(errs) if errs else 1.0
    ax2.set_ylim(0, max(emax * 1.28, 1.0))
    ax2.set_ylabel("% Error")
    ax.plot(xs, sim_y, "o-", color=C_SIM, lw=1.8, markersize=7, label=CLASSIC_SIM, zorder=4)
    ax.plot(xs, meas_y, "s-", color=C_MEAS, lw=1.8, markersize=7, label=CLASSIC_MEAS, zorder=4)
    ax.set_zorder(2)
    ax.patch.set_visible(False)
    ax.set_xlabel("Timesteps")
    ax.set_ylabel(ylabel)
    ax.set_title(title + (" · log–log" if loglog else ""), fontsize=9)
    vals = [v for v in list(meas_y) + list(sim_y) if v is not None and v > 0]
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


def _s2_opt_tag(opt: str) -> str:
    return f" · -{opt}"


def s3_legend_handles():
    return [
        Line2D([0], [0], color=C_SIM, marker="o", lw=1.8, markersize=7, label=CLASSIC_SIM),
        Line2D([0], [0], color=C_MEAS, marker="s", lw=1.8, markersize=7, label=CLASSIC_MEAS),
        mpatches.Patch(facecolor="#d0d0d0", edgecolor="#9a9a9a", label="|% error|"),
    ]


def finish_s3_combined(fig, opt: str) -> None:
    fig.subplots_adjust(top=0.78, left=0.055, right=0.92, bottom=0.14, wspace=0.40)
    fig.suptitle(
        f"Stage 3: Predictions Over Larger Timesteps — STREAM Triad (-{opt})",
        fontsize=12, y=0.985,
    )
    fig.legend(
        handles=s3_legend_handles(),
        loc="upper center",
        ncol=3,
        bbox_to_anchor=(0.5, 0.925),
        frameon=False,
        fontsize=9,
    )


def finish_s2_combined(fig, opt: str) -> None:
    fig.subplots_adjust(top=0.76, left=0.055, right=0.995, bottom=0.16, wspace=0.34)
    fig.suptitle(
        f"Stage 2: Single-Timestep Validation — STREAM Triad (-{opt})",
        fontsize=12, y=0.985,
    )
    fig.text(
        0.5, 0.925, r"N≈1×10$^9$ elements, 100 iterations",
        ha="center", fontsize=10,
    )
    fig.legend(
        handles=[
            mpatches.Patch(color=C_SIM, label=CLASSIC_SIM),
            mpatches.Patch(color=C_MEAS, label=CLASSIC_MEAS),
        ],
        loc="upper center",
        ncol=2,
        bbox_to_anchor=(0.5, 0.905),
        frameon=False,
        fontsize=9,
    )


def make_stage3() -> list[Path]:
    apply_thesis_style()
    fig_dir = TC_ROOT / "stage3-multi-timestep/figures"
    fig_o1 = TC_ROOT / "stage3-multi-timestep/figures-o1"
    meas_tsv = TC_ROOT / "stage3-multi-timestep/results/measured/stage3-measured.tsv"
    meas_o1_tsv = TC_ROOT / "stage3-multi-timestep/results/measured-o1/stage3-measured.tsv"
    log_source("measured TSV (O3)", meas_tsv)
    log_source("measured TSV (O1)", meas_o1_tsv)
    meas = parse_measured_tsv(meas_tsv)
    meas_o1 = parse_measured_tsv(meas_o1_tsv)
    platforms = [
        ("icx-tpn1", "ICX tpn=1 (36 nodes)"),
        ("icx-socket", "ICX socket (1 node)"),
        ("spr-socket", "SPR socket (1 node)"),
    ]
    timesteps = [100, 1000, 10000]
    base_csv, o1_csv = [], []
    pack: dict[str, dict] = {}
    for plat, _lab in platforms:
        pack[plat] = {
            k: []
            for k in (
                "ts",
                "meas_w", "sim_w", "meas_o1_w", "o1_w",
                "meas_g", "sim_g", "meas_o1_g", "o1_g",
                "meas_b", "sim_b", "meas_o1_b", "o1_b",
            )
        }
        for nt in timesteps:
            cfg = f"{plat}-{nt}"
            m = meas.get(cfg)
            sim_dir = TC_ROOT / f"stage3-multi-timestep/results/simulated/{cfg}"
            o1_dir = TC_ROOT / f"stage3-multi-timestep/results/simulated-o1/{cfg}"
            s = sim_wall(sim_dir)
            o = sim_wall(o1_dir)
            if not m or s is None:
                print(f"  skip {cfg}: missing base data")
                continue
            m1 = meas_o1.get(cfg)
            sr = _stream_rates({**m, "ntimes": nt}, s)
            pack[plat]["ts"].append(nt)
            pack[plat]["meas_w"].append(m["wall_s"])
            pack[plat]["sim_w"].append(s)
            pack[plat]["meas_o1_w"].append(None if m1 is None else m1["wall_s"])
            pack[plat]["o1_w"].append(o)
            pack[plat]["meas_g"].append(m["gflops"])
            pack[plat]["sim_g"].append(sr["gflops"])
            pack[plat]["meas_o1_g"].append(None if m1 is None else m1["gflops"])
            pack[plat]["o1_g"].append(
                None if o is None else _stream_rates({**(m1 or m), "ntimes": nt}, o)["gflops"]
            )
            pack[plat]["meas_b"].append(m["gb_s"])
            pack[plat]["sim_b"].append(sr["gb_s"])
            pack[plat]["meas_o1_b"].append(None if m1 is None else m1["gb_s"])
            pack[plat]["o1_b"].append(
                None if o is None else _stream_rates({**(m1 or m), "ntimes": nt}, o)["gb_s"]
            )
            row = csv_row(cfg, m["wall_s"], s, m, sr)
            base_csv.append(row)
            o1_csv.append(csv_row_o1(cfg, m["wall_s"], s, m, sr, m1, o))
            print(f"  {cfg}: meas={m['wall_s']:.2f} sim={s:.2f}" + (f" o1={o:.2f}" if o else ""))
            log_source(f"sim-O3 {cfg}", sim_dir / "slurm-discostic-pass2.out")
            log_stf(f"STF sim {cfg}", TC_ROOT / f"stage3-multi-timestep/results/simulated/{cfg}/{cfg}.single.stf")

    paths: list[Path] = []
    metric_map = [
        ("wall", "Wall time [s]", "meas_w", "sim_w", "meas_o1_w", "o1_w", "s"),
        ("gflops", "Performance [Gflops/s]", "meas_g", "sim_g", "meas_o1_g", "o1_g", ""),
        ("gb", "Bandwidth [GB/s] (write-allocate)", "meas_b", "sim_b", "meas_o1_b", "o1_b", ""),
    ]
    usable = [p for p, _ in platforms if pack[p]["meas_w"]]
    if not usable:
        print("stage3: insufficient points, skip")
        return paths

    def emit_s3(outdir: Path, opt: str, meas_k, sim_k) -> None:
        suffix = f" · -{opt}"
        for key, ylabel, mk, sk, mok, pk, val_sfx in metric_map:
            mk_use, sk_use = (mok, pk) if opt == "O1" else (meas_k and mk, sim_k and sk)
            if opt == "O1":
                mk_use, sk_use = mok, pk
            else:
                mk_use, sk_use = mk, sk
            fig, axes = plt.subplots(1, len(usable), figsize=(5.15 * len(usable), 5.15), squeeze=False)
            for ax, plat in zip(axes[0], usable):
                ts = pack[plat]["ts"]
                meas_y = pack[plat][mk_use]
                sim_y = pack[plat][sk_use]
                if any(v is None for v in sim_y):
                    ax.set_visible(False)
                    continue
                draw_s3_panel(
                    ax, ts, meas_y, sim_y, ylabel,
                    S3_PANEL_TITLE.get(plat, plat) + suffix,
                    value_suffix=val_sfx,
                    loglog=(key == "wall"),
                )
            finish_s3_combined(fig, opt)
            out = outdir / f"stage3-{key}.png"
            save_figure(fig, out)
            paths.append(out)
            if key == "wall" and opt == "O3":
                alias = outdir / "stage3-sim-vs-measured.png"
                alias.write_bytes(out.read_bytes())
                print(f"  wrote {alias}")
                paths.append(alias)
            for plat in usable:
                fig_s, ax_s = plt.subplots(figsize=(6.4, 5.1))
                draw_s3_panel(
                    ax_s, pack[plat]["ts"], pack[plat][mk_use], pack[plat][sk_use],
                    ylabel, S3_PANEL_TITLE.get(plat, plat) + suffix, value_suffix=val_sfx,
                    loglog=(key == "wall"),
                )
                fig_s.subplots_adjust(top=0.82, left=0.12, right=0.88, bottom=0.12)
                fig_s.legend(
                    handles=s3_legend_handles(),
                    loc="upper center", ncol=3, bbox_to_anchor=(0.5, 0.98),
                    frameon=False, fontsize=9,
                )
                solo = outdir / f"stage3-{plat}-{key}.png"
                save_figure(fig_s, solo)
                paths.append(solo)

    emit_s3(fig_dir, "O3", "meas_w", "sim_w")
    if any(v is not None for plat in usable for v in pack[plat]["o1_w"]):
        emit_s3(fig_o1, "O1", "meas_o1_w", "o1_w")
    if base_csv:
        write_metric_csv(fig_dir / "stage3_data.csv", base_csv, CSV_HEADER)
        paths.append(fig_dir / "stage3_data.csv")
    if o1_csv and any(v is not None for plat in usable for v in pack[plat]["o1_w"]):
        write_metric_csv(fig_o1 / "stage3_data.csv", o1_csv, CSV_HEADER_O1)
        paths.append(fig_o1 / "stage3_data.csv")
    return paths


def make_stage4() -> list[Path]:
    print("  Stage 4: no Gantt PNG (use Intel Trace Analyzer on STF).")
    fig_dir = TC_ROOT / "stage4-idlewave/figures"
    fig_dir.mkdir(parents=True, exist_ok=True)
    meas_tsv = TC_ROOT / "stage4-idlewave/results/measured/stage4-measured.tsv"
    log_source("measured TSV (O3)", meas_tsv)
    meas = parse_measured_tsv(meas_tsv)
    labels, meas_rows, sim_w, dummy = [], [], [], []
    csv_rows = []
    for cfg, lab, stem in (
        ("icx", "ICX idlewave\n(2 nodes, 72 ranks)", "stream-triad-idlewave"),
        ("spr", "SPR idlewave\n(2 nodes, 104 ranks)", "stream-triad-idlewave-spr"),
    ):
        m = meas.get(cfg)
        sim_dir = TC_ROOT / f"stage4-idlewave/results/simulated/{cfg}"
        s = sim_wall(sim_dir)
        log_source(f"sim-O3 {cfg}", sim_dir / "slurm-discostic-pass2.out")
        if not m or s is None:
            print(f"  skip {cfg}: missing base data")
            continue
        labels.append(lab)
        meas_rows.append(m)
        sim_w.append(s)
        dummy.append(None)
        csv_rows.append(csv_row(cfg, m["wall_s"], s, m, _stream_rates(m, s)))
        print(f"  stage4 {cfg}: meas={m['wall_s']:.3f} sim={s:.3f}")
    paths: list[Path] = []
    if labels:
        paths.extend(emit_metric_set(fig_dir, "stage4", labels, meas_rows, sim_w, dummy, "Stage 4", include_o1=False))
        write_metric_csv(fig_dir / "stage4_metrics.csv", csv_rows, CSV_HEADER)
        paths.append(fig_dir / "stage4_metrics.csv")
    for label, path in (
        ("STF measured ICX", TC_ROOT / "stage4-idlewave/results/measured-trace/icx/stream-triad-idlewave-measured.single.stf"),
        ("STF measured SPR", TC_ROOT / "stage4-idlewave/results/measured-trace/spr/stream-triad-idlewave-spr-measured.single.stf"),
        ("STF sim ICX", TC_ROOT / "stage4-idlewave/results/simulated/icx/stream-triad-idlewave.single.stf"),
        ("STF sim SPR", TC_ROOT / "stage4-idlewave/results/simulated/spr/stream-triad-idlewave-spr.single.stf"),
    ):
        log_stf(label, path)
    return paths


def make_stage5() -> list[Path]:
    apply_thesis_style()
    fig_dir = TC_ROOT / "stage5-scaling/figures"
    meas_tsv = TC_ROOT / "stage5-scaling/results/measured/stage5-measured.tsv"
    log_source("measured TSV (O3)", meas_tsv)
    meas = parse_measured_tsv(meas_tsv)
    nodes_icx = (1, 2, 4, 8, 16, 32)
    nodes_spr = (1, 2, 4, 8)  # spr1tb MaxNodes=8; not 16/32
    keys = ("strong-icx", "strong-spr", "weak-icx", "weak-spr")
    series: dict[str, dict] = {}
    base_csv = []
    for key in keys:
        kind, arch = key.split("-")
        nodes_all = nodes_icx if arch == "icx" else nodes_spr
        series[key] = {k: [] for k in ("n", "meas_w", "sim_w", "meas_g", "sim_g", "meas_b", "sim_b", "meas_e", "sim_e")}
        walls_meas, walls_sim = {}, {}
        for nodes in nodes_all:
            name = f"{kind}-{arch}-{nodes}node"
            m = meas.get(name)
            sim_dir = TC_ROOT / f"stage5-scaling/results/simulated/{name}"
            s = sim_wall(sim_dir)
            if not m or s is None:
                print(f"  skip {name}: missing base data")
                continue
            log_source(f"sim-O3 {name}", sim_dir / "slurm-discostic-pass2.out")
            walls_meas[nodes] = m["wall_s"]
            walls_sim[nodes] = s
            sr = _stream_rates(m, s)
            series[key]["n"].append(nodes)
            series[key]["meas_w"].append(m["wall_s"])
            series[key]["sim_w"].append(s)
            series[key]["meas_g"].append(m["gflops"])
            series[key]["sim_g"].append(sr["gflops"])
            series[key]["meas_b"].append(m["gb_s"])
            series[key]["sim_b"].append(sr["gb_s"])
            base_csv.append(csv_row(name, m["wall_s"], s, m, sr))
            print(f"  {name}: meas={m['wall_s']:.2f} sim={s:.2f} nodes={nodes}")
            log_stf(f"STF sim {name}", TC_ROOT / f"stage5-scaling/results/simulated/{name}/stream-triad-{name}.single.stf")
        t1m, t1s = walls_meas.get(1), walls_sim.get(1)
        for nodes in series[key]["n"]:
            mv, sv = walls_meas[nodes], walls_sim[nodes]
            if kind == "strong":
                series[key]["meas_e"].append(dmetrics.strong_efficiency(t1m or 0, mv, nodes))
                series[key]["sim_e"].append(dmetrics.strong_efficiency(t1s or 0, sv, nodes))
            else:
                series[key]["meas_e"].append(dmetrics.weak_efficiency(t1m or 0, mv))
                series[key]["sim_e"].append(dmetrics.weak_efficiency(t1s or 0, sv))

    header_e = CSV_HEADER[:-1] + ["meas_efficiency", "sim_efficiency", "formula"]
    rows_e = []
    idx = 0
    for key in keys:
        for i, nodes in enumerate(series[key]["n"]):
            row = base_csv[idx]
            idx += 1
            rows_e.append(row[:-1] + [f"{series[key]['meas_e'][i]:.4f}", f"{series[key]['sim_e'][i]:.4f}", row[-1]])

    paths: list[Path] = []

    def line_panel(ax, panel, ymeas, ysim, ylabel, title, *, zero_y: bool, log_y: bool = False):
        xs = series[panel]["n"]
        if not xs:
            ax.set_visible(False)
            return
        ax.plot(xs, ymeas, "s-", color=C_MEAS, lw=1.8, markersize=7, label=LEGEND_MEAS)
        ax.plot(xs, ysim, "o-", color=C_SIM, lw=1.8, markersize=7, label=LEGEND_SIM)
        ax.set_xscale("log", base=2)
        ticks = nodes_icx if "icx" in panel else nodes_spr
        shown = [n for n in ticks if n in set(xs)]
        ax.set_xticks(shown)
        ax.xaxis.set_major_formatter(mticker.FuncFormatter(lambda v, _: f"{int(round(v))}"))
        ax.xaxis.set_minor_locator(mticker.NullLocator())
        ax.xaxis.set_minor_formatter(mticker.NullFormatter())
        ax.set_xlabel("# of nodes")
        ax.set_ylabel(ylabel)
        ax.set_title(title + (" · log–log" if log_y else ""), fontsize=10)
        ax.legend(fontsize=8)
        ax.grid(True, alpha=0.3)
        if log_y:
            vals = [v for v in list(ymeas) + list(ysim) if v is not None and v > 0]
            ax.set_yscale("log")
            if vals:
                lo, hi = min(vals), max(vals)
                ypad = (hi / lo) ** 0.12 if hi > lo else 1.8
                ax.set_ylim(lo / ypad, hi * ypad)
        elif zero_y:
            ymax = max(list(ymeas) + list(ysim), default=0.0)
            ax.set_ylim(0, ymax * 1.22 if ymax > 0 else 1)
        annotate_line_errors(ax, xs, ymeas, ysim)

    metric_specs = [
        ("wall", "Wall time [s]", "meas_w", "sim_w"),
        ("gflops", "Performance [GFlop/s]", "meas_g", "sim_g"),
        ("gb", "Bandwidth [GB/s] (write-allocate)", "meas_b", "sim_b"),
        ("efficiency", "Efficiency", "meas_e", "sim_e"),
    ]
    titles = {
        "strong-icx": "Strong ICX (1–32 nodes)",
        "strong-spr": "Strong SPR (1–8 nodes)",
        "weak-icx": "Weak ICX (1–32 nodes)",
        "weak-spr": "Weak SPR (1–8 nodes)",
    }
    # One figure per architecture so ICX 32-node and SPR 8-node ticks never mix.
    arch_panels = {
        "icx": ("strong-icx", "weak-icx"),
        "spr": ("strong-spr", "weak-spr"),
    }
    for metric, ylabel, mk, sk in metric_specs:
        for arch, panels in arch_panels.items():
            fig, axes = plt.subplots(1, 2, figsize=(11, 4.8))
            for ax, panel in zip(axes, panels):
                line_panel(
                    ax, panel, series[panel][mk], series[panel][sk], ylabel, titles[panel],
                    zero_y=(metric != "efficiency"),
                    log_y=(metric in ("gflops", "gb") or (metric == "wall" and panel.startswith("strong"))),
                )
                if metric == "efficiency":
                    ax.axhline(1.0, color="#888", ls="--", lw=0.8)
                    ax.set_ylim(0, 1.25)
            node_note = "1, 2, 4, 8, 16, 32 nodes" if arch == "icx" else "1, 2, 4, 8 nodes (spr1tb MaxNodes=8)"
            fig.suptitle(f"Stage 5 — {ylabel} · {arch.upper()} · {node_note}", fontsize=11)
            fig.tight_layout(rect=[0, 0, 1, 0.92])
            out = fig_dir / f"stage5-{arch}-{metric}.png"
            save_figure(fig, out)
            paths.append(out)
            for panel in panels:
                fig_s, ax_s = plt.subplots(figsize=(6.2, 4.6))
                line_panel(
                    ax_s, panel, series[panel][mk], series[panel][sk], ylabel, titles[panel],
                    zero_y=(metric != "efficiency"),
                    log_y=(metric in ("gflops", "gb") or (metric == "wall" and panel.startswith("strong"))),
                )
                if metric == "efficiency":
                    ax_s.axhline(1.0, color="#888", ls="--", lw=0.8)
                    ax_s.set_ylim(0, 1.25)
                solo = fig_dir / f"stage5-{panel}-{metric}.png"
                save_figure(fig_s, solo)
                paths.append(solo)
    if rows_e:
        write_metric_csv(fig_dir / "stage5_scaling_wall.csv", rows_e, header_e)
        paths.append(fig_dir / "stage5_scaling_wall.csv")
    return paths


def saturated_pixel_fraction(path: Path, threshold: int = 240) -> float:
    if Image is None:
        import numpy as np

        arr = plt.imread(path)
        if arr.ndim == 3:
            white = (arr[..., :3] > threshold / 255.0).all(axis=-1)
        else:
            white = arr > threshold / 255.0
        return float(1.0 - white.mean())
    img = Image.open(path).convert("RGB")
    import numpy as np

    arr = np.asarray(img)
    white = (arr > threshold).all(axis=-1)
    return float(1.0 - white.mean())


def verify_outputs(paths: list[Path], min_fraction: float = 0.04) -> bool:
    ok = True
    print("\nPixel sanity check (non-white fraction):")
    pngs = [p for p in paths if p.suffix.lower() == ".png"]
    if not pngs:
        print("  (no PNGs this pass)")
        return True
    for path in pngs:
        if not path.is_file():
            print(f"  [FAIL] missing {path}")
            ok = False
            continue
        frac = saturated_pixel_fraction(path)
        status = "OK" if frac >= min_fraction else "FAIL"
        print(f"  [{status}] {path.name}: {frac * 100:.2f}% colored")
        if frac < min_fraction:
            ok = False
    return ok


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate STREAM demo stage figures")
    parser.add_argument("--stages", default="1,2,3,4,5", help="Comma-separated stage numbers")
    parser.add_argument("--verify-only", action="store_true", help="Pixel-check existing PNGs")
    parser.add_argument("--opt-cases", default="O3,O1", help="Ignored; O1 series live in figures-o1/")
    args = parser.parse_args()
    stages = {int(s.strip()) for s in args.stages.split(",") if s.strip()}

    if args.verify_only:
        paths: list[Path] = []
        for stage, folder, names in (
            (1, "stage1-ecm-modeling/figures", ["stage1-ecm-breakdown.png"]),
            (2, "stage2-single-timestep/figures", ["stage2-wall.png", "stage2-gflops.png", "stage2-gb.png"]),
            (3, "stage3-multi-timestep/figures", ["stage3-wall.png", "stage3-gflops.png", "stage3-gb.png"]),
            (4, "stage4-idlewave/figures", ["stage4-wall.png", "stage4-gflops.png", "stage4-gb.png"]),
            (5, "stage5-scaling/figures", [
                "stage5-icx-wall.png", "stage5-spr-wall.png",
                "stage5-icx-gflops.png", "stage5-spr-gflops.png",
            ]),
        ):
            if stage in stages:
                base = TC_ROOT / folder
                paths.extend(base / n for n in names)
        return 0 if verify_outputs(paths) else 1

    makers = {1: make_stage1, 2: make_stage2, 3: make_stage3, 4: make_stage4, 5: make_stage5}
    written: list[Path] = []
    print(f"REPO_ROOT={REPO_ROOT}")
    print(f"TC_ROOT={TC_ROOT}")
    for stage in sorted(stages):
        maker = makers.get(stage)
        if not maker:
            print(f"  Stage {stage}: unknown, skipping")
            continue
        print(f"\n=== Stage {stage} ===")
        written.extend(maker())
    if not verify_outputs(written):
        print("\nERROR: One or more figures appear blank.", file=sys.stderr)
        return 1
    print("\nAll figures passed pixel sanity check.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
