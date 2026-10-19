#!/usr/bin/env python3
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: Ray tracing + Optical Flow demo figures (Stages 1–5). Visual language
#          matches STREAM/HEAT/Pi (S2 independent panels + Error box; S3 gray
#          |% error| twin axis + figure-level legend; S5 error annotations, y
#          from 0). Wall and Mpix/s use the combined run. Mpix/s is
#          (N + Nsteps) × nreps / wall.

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
LEGEND_SIM = "DisCostiC simulation"
LEGEND_MEAS = "MPI benchmark"
LEGEND_ERR = "|% error|"

ECM_RT_ICX = {"T_comp": 644.8500772, "T_RegL1": 0.0, "T_L1L2": 0.0, "T_L2L3": 0.0, "T_L3Mem": 0.0}
ECM_OF_ICX = {"T_comp": 71.5375, "T_RegL1": 100.5, "T_L1L2": 303.75, "T_L2L3": 911.25, "T_L3Mem": 420.0}

FORMULA = (
    "Mpix/s=(N+Nsteps)*nreps/wall/1e6; nxOF=2^(MG-1)+2; nxRT=nxOF*SS; "
    "wall_err=(sim-meas)/meas; rate_err=(sim-meas)/meas on the combined wall. "
    "Both kernels share that wall."
)

CSV_HEADER = [
    "config",
    "meas_wall_s",
    "sim_wall_s",
    "wall_err_pct",
    "meas_mpix_s",
    "sim_mpix_s",
    "rate_err_pct",
    "formula",
]


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


def unlink_stale(fig_dir: Path, patterns: list[str]) -> None:
    fig_dir.mkdir(parents=True, exist_ok=True)
    for pat in patterns:
        for stale in fig_dir.glob(pat):
            stale.unlink(missing_ok=True)
            print(f"  removed {stale}")


def parse_rank_times(path: Path) -> list[tuple[int, float]]:
    if not path.is_file():
        return []
    text = path.read_text(errors="replace")
    blocks = text.split("FULL APPLICATION PERFORMANCE (for all ranks):")
    if len(blocks) < 2:
        return []
    section = blocks[-1].split("FULL APPLICATION PERFORMANCE")[0]
    times: list[tuple[int, float]] = []
    for line in section.splitlines():
        match = re.match(r"\s*(\d+)\s+\S+\s+([\d.eE+-]+)\s*$", line)
        if match:
            times.append((int(match.group(1)), float(match.group(2))))
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
    kernel_label: str = "Optical Flow",
) -> None:
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
    if t_comp >= 0.05:
        ax.text(0, t_comp / 2, f"{t_comp:g}", ha="center", va="center", fontsize=9)
    bottom = 0.0
    for name, value in data_segs:
        if value <= 0:
            continue
        ax.bar(
            1, value, bottom=bottom, width=0.55, color=ECM_COLORS[name],
            edgecolor="#444", linewidth=0.8, label=f"{name} = {value:g}",
        )
        if value >= 0.05:
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
    ax.set_ylim(0, max(bottom, t_comp, tecm, 0.5) * 1.28)
    ax.legend(fontsize=7, loc="upper right")
    ax.grid(True, axis="y", alpha=0.3)


def resolve_ecm_cfg(primary: Path, *fallbacks: Path) -> Path | None:
    for path in (primary, *fallbacks):
        if path.is_file():
            return path
    return None


def parse_measured_tsv(path: Path) -> dict[str, dict]:
    out: dict[str, dict] = {}
    for cfg, row in dmetrics.last_row_tsv(path).items():
        try:
            wall_s = float(row["wall_seconds"])
        except (KeyError, TypeError, ValueError):
            continue
        rates = dmetrics.rtof_from_tsv_row(row, wall_s)
        out[cfg] = {"wall_s": wall_s, **rates, "raw": row}
    return out


def sim_wall(sim_dir: Path) -> tuple[float, int] | None:
    """Return (runtime seconds, rank) of the slowest simulated rank."""
    for name in ("slurm-discostic-pass2.out", "slurm-discostic-pass1.out"):
        times = parse_rank_times(sim_dir / name)
        if times:
            rank, runtime = max(times, key=lambda item: item[1])
            return runtime, rank
    return None


def _wall_rates(m: dict, sim_s: float) -> dict:
    """Rate of the whole run from the combined simulated wall."""
    return dmetrics.rtof_from_tsv_row(m["raw"], sim_s)


def csv_row(cfg: str, meas_s: float, sim_s: float, meas_r: dict, sim_r: dict) -> list:
    wall_err = dmetrics.signed_error_pct(sim_s, meas_s)
    rate_err = dmetrics.rate_err_pct(sim_r["mpix_s"], meas_r["mpix_s"])
    return [
        cfg,
        f"{meas_s:.6f}",
        f"{sim_s:.6f}",
        f"{wall_err:.4f}",
        f"{meas_r['mpix_s']:.4f}",
        f"{sim_r['mpix_s']:.4f}",
        f"{rate_err:.4f}",
        FORMULA,
    ]


def write_metric_csv(path: Path, rows: list[list], header: list[str]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", newline="") as fh:
        w = csv.writer(fh)
        w.writerow(header)
        w.writerows(rows)
    print(f"  wrote {path}")


S2_PANEL_TITLE = {
    "icx-tpn1-100": "(a) ICX, task_per_node=1\n(36 ranks × 36 nodes)",
    "icx-socket-100": "(b) ICX, task_per_node=36\n(full socket, 36 ranks)",
    "icx": "(a) ICX idlewave\n(full node, 72 ranks)",
}
S3_PANEL_TITLE = {
    "icx-tpn1": "(a) ICX, task_per_node=1 (36 ranks)",
    "icx-socket": "(b) ICX, task_per_node=36 (full socket)",
}


def fmt_err_pct(sim, meas) -> str | None:
    if sim is None or meas is None or meas == 0:
        return None
    return f"{dmetrics.signed_error_pct(sim, meas):+.1f}%"


def annotate_line_errors(ax, xs, meas_y, sim_y):
    for i, x in enumerate(xs):
        meas = meas_y[i] if i < len(meas_y) else None
        sim = sim_y[i] if i < len(sim_y) else None
        err = fmt_err_pct(sim, meas)
        if err and sim is not None:
            ax.annotate(
                err, (x, sim), textcoords="offset points", xytext=(0, 8),
                ha="center", fontsize=7, color=C_SIM, zorder=5,
            )


def _fmt_bar(v: float) -> str:
    if abs(v) < 1:
        return f"{v:.3f}"
    return f"{v:.2f}" if abs(v) < 100 else f"{v:.1f}"


def _fmt_s3_val(v: float, suffix: str) -> str:
    if abs(v) < 1:
        return f"{v:.3f}{suffix}"
    return f"{v:.1f}{suffix}"


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


def draw_s3_panel(ax, ts, meas_y, sim_y, ylabel, title, *, value_suffix: str, loglog: bool = False) -> None:
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
        if i == 0 or i == n - 1:
            ha, xoff = "right", -5
        else:
            ha, xoff = "left", 6
        near_axis = (not loglog) and ymax > 0 and max(mv, sv) < ymax * 0.18
        sim_above = sv >= mv
        if near_axis:
            if sim_above:
                sim_off, meas_off = (xoff, 16), (xoff, 4)
            else:
                sim_off, meas_off = (xoff, 4), (xoff, 16)
        elif sim_above:
            sim_off, meas_off = (xoff, 9), (xoff, -12)
        else:
            sim_off, meas_off = (xoff, -12), (xoff, 9)
        ax.annotate(
            _fmt_s3_val(sv, value_suffix), (x, sv), xytext=sim_off, textcoords="offset points",
            fontsize=7, color=C_SIM, ha=ha, va="center", zorder=5, clip_on=False,
        )
        ax.annotate(
            _fmt_s3_val(mv, value_suffix), (x, mv), xytext=meas_off, textcoords="offset points",
            fontsize=7, color=C_MEAS, ha=ha, va="center", zorder=5, clip_on=False,
        )


def s3_legend_handles():
    return [
        Line2D([0], [0], color=C_SIM, marker="o", lw=1.8, markersize=7, label=LEGEND_SIM),
        Line2D([0], [0], color=C_MEAS, marker="s", lw=1.8, markersize=7, label=LEGEND_MEAS),
        mpatches.Patch(facecolor="#d0d0d0", edgecolor="#9a9a9a", label=LEGEND_ERR),
    ]


def finish_s2_combined(fig) -> None:
    fig.subplots_adjust(top=0.76, left=0.07, right=0.995, bottom=0.16, wspace=0.34)
    fig.suptitle("Stage 2: Single-Timestep Validation — Ray tracing + Optical flow (-O3)", fontsize=12, y=0.985)
    fig.text(
        0.5, 0.925,
        r"MG=9, SS=2, 100 repetitions",
        ha="center", fontsize=10,
    )
    fig.legend(
        handles=[
            mpatches.Patch(color=C_SIM, label=LEGEND_SIM),
            mpatches.Patch(color=C_MEAS, label=LEGEND_MEAS),
        ],
        loc="upper center", ncol=2, bbox_to_anchor=(0.5, 0.905), frameon=False, fontsize=9,
    )


def finish_s3_combined(fig) -> None:
    fig.subplots_adjust(top=0.78, left=0.07, right=0.90, bottom=0.14, wspace=0.40)
    fig.suptitle("Stage 3: Predictions Over Larger Timesteps — Ray tracing + Optical flow (-O3)", fontsize=12, y=0.985)
    fig.legend(
        handles=s3_legend_handles(),
        loc="upper center", ncol=3, bbox_to_anchor=(0.5, 0.925), frameon=False, fontsize=9,
    )


def _s2_values(m, sim_r, s, *, is_wall: bool, key: str):
    if is_wall:
        return s, m["wall_s"]
    rk = {"mpix": "mpix_s"}[key]
    return sim_r[rk], m[rk]


S2_S4_METRICS = [
    ("wall", "Wall time [s]", True),
    ("mpix", "Mpix/s", False),
]


def make_stage1() -> list[Path]:
    apply_thesis_style()
    fig_dir = TC_ROOT / "stage1-ecm-modeling/figures"
    unlink_stale(fig_dir, ["*of_mpix*", "*rt_mpix*"])
    paths: list[Path] = []

    rt_cfg = resolve_ecm_cfg(
        REPO_ROOT / "nodelevel/configs/RT.cfg",
        TC_ROOT / "stage2-single-timestep/results/simulated/icx-socket-100/RT_ecm.cfg",
    )
    of_cfg = resolve_ecm_cfg(
        REPO_ROOT / "nodelevel/configs/OF.cfg",
        TC_ROOT / "stage2-single-timestep/results/simulated/icx-socket-100/OF_ecm.cfg",
    )
    rt_vals = maybe_load_ecm_from_cfg(rt_cfg or Path(), ECM_RT_ICX)
    of_vals = maybe_load_ecm_from_cfg(of_cfg or Path(), ECM_OF_ICX)

    combined = fig_dir / "stage1-ecm-breakdown.png"
    fig, axes = plt.subplots(1, 2, figsize=(10.5, 5.2))
    draw_ecm_stack(axes[0], "ICX", rt_vals, subtitle="Hand LBL RT.cfg · compute-bound", kernel_label="Ray tracing (RT)")
    draw_ecm_stack(axes[1], "ICX", of_vals, subtitle="Hand LBL OF.cfg · memory-bound", kernel_label="Optical flow (OF)")
    fig.suptitle("Stage 1 — ECM modeling (RT + OF) · ICX · -O3", fontsize=12)
    fig.tight_layout()
    save_figure(fig, combined)
    paths.append(combined)

    solo_rt = fig_dir / "stage1-ecm-breakdown-rt-icx.png"
    fig_rt, ax_rt = plt.subplots(figsize=(5.2, 5.0))
    draw_ecm_stack(ax_rt, "ICX", rt_vals, subtitle="Hand LBL RT.cfg", kernel_label="Ray tracing (RT)")
    fig_rt.tight_layout()
    save_figure(fig_rt, solo_rt)
    paths.append(solo_rt)

    solo_of = fig_dir / "stage1-ecm-breakdown-of-icx.png"
    fig_of, ax_of = plt.subplots(figsize=(5.2, 5.0))
    draw_ecm_stack(ax_of, "ICX", of_vals, subtitle="Hand LBL OF.cfg", kernel_label="Optical flow (OF)")
    fig_of.tight_layout()
    save_figure(fig_of, solo_of)
    paths.append(solo_of)

    csv_path = fig_dir / "stage1_ecm_data.csv"
    with csv_path.open("w", newline="") as fh:
        w = csv.writer(fh)
        w.writerow(["kernel", "arch", "cfg_source", "T_comp", "T_RegL1", "T_L1L2", "T_L2L3", "T_L3Mem", "T_ECM"])
        for kernel, vals, src in (
            ("RT", rt_vals, str(rt_cfg or "fallback")),
            ("OF", of_vals, str(of_cfg or "fallback")),
        ):
            w.writerow(
                [
                    kernel,
                    "ICX",
                    src,
                    vals["T_comp"],
                    vals["T_RegL1"],
                    vals["T_L1L2"],
                    vals["T_L2L3"],
                    vals["T_L3Mem"],
                    f"{t_ecm(vals):.4f}",
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
        ax.set_title("Stage 1 — Measured wall time (RT + OF)")
        ax.tick_params(axis="x", rotation=15)
        save_figure(fig, fig_dir / "stage1-measured-wall.png")
        paths.append(fig_dir / "stage1-measured-wall.png")
        if any(meas[k]["N"] and meas[k]["nreps"] for k in labels):
            for key, ylabel, getter in (
                ("mpix", "Mpix/s", lambda m: m["mpix_s"]),
            ):
                fig_r, ax_r = plt.subplots(figsize=(7.2, 4.2))
                vals = [getter(meas[k]) for k in labels]
                ax_r.bar(labels, vals, color=C_MEAS, edgecolor="#333", width=0.55, zorder=3)
                ax_r.set_ylabel(ylabel)
                ax_r.set_title(f"Stage 1 — Measured {ylabel}")
                ax_r.tick_params(axis="x", rotation=15)
                out = fig_dir / f"stage1-measured-{key}.png"
                save_figure(fig_r, out)
                paths.append(out)
    return paths


def make_stage2() -> list[Path]:
    apply_thesis_style()
    fig_dir = TC_ROOT / "stage2-single-timestep/figures"
    unlink_stale(fig_dir, ["stage2-sim-vs-measured*.png", "stage2-wall-error.png", "*of_mpix*", "*rt_mpix*"])
    meas_dir = TC_ROOT / "stage2-single-timestep/results/measured"
    meas = parse_measured_tsv(meas_dir / "stage2-measured-100.tsv")
    configs = ["icx-tpn1-100", "icx-socket-100"]
    rows_data: list[tuple[str, dict, float, dict]] = []
    csv_rows: list[list] = []
    for cfg in configs:
        m = meas.get(cfg)
        sim_dir = TC_ROOT / "stage2-single-timestep/results/simulated" / cfg
        slow = sim_wall(sim_dir)
        if not m or slow is None:
            print(f"  skip {cfg}: missing data")
            continue
        s, _rank = slow
        sr = _wall_rates(m, s)
        rows_data.append((cfg, m, s, sr))
        csv_rows.append(csv_row(cfg, m["wall_s"], s, m, sr))
        print(f"  {cfg}: meas={m['wall_s']:.3f} sim={s:.3f}")
    if not rows_data:
        return []
    paths: list[Path] = []
    for key, ylabel, is_wall in S2_S4_METRICS:
        fig, axes = plt.subplots(1, len(rows_data), figsize=(4.55 * len(rows_data), 5.15), squeeze=False)
        for ax, (cfg, m, s, sr) in zip(axes[0], rows_data):
            sim_v, meas_v = _s2_values(m, sr, s, is_wall=is_wall, key=key)
            err = dmetrics.signed_error_pct(sim_v, meas_v)
            draw_s2_panel(ax, sim_v, meas_v, ylabel, S2_PANEL_TITLE.get(cfg, cfg), err)
        finish_s2_combined(fig)
        out = fig_dir / f"stage2-{key}.png"
        save_figure(fig, out)
        paths.append(out)
        for cfg, m, s, sr in rows_data:
            fig_s, ax_s = plt.subplots(figsize=(5.4, 4.8))
            sim_v, meas_v = _s2_values(m, sr, s, is_wall=is_wall, key=key)
            err = dmetrics.signed_error_pct(sim_v, meas_v)
            extra = f"wall  sim {s:.2f}s / meas {m['wall_s']:.2f}s"
            draw_s2_panel(
                ax_s, sim_v, meas_v, ylabel, S2_PANEL_TITLE.get(cfg, cfg), err, extra_box=extra,
            )
            stem = cfg.replace("-100", "")
            solo = fig_dir / f"stage2-{stem}-{key}.png"
            save_figure(fig_s, solo)
            paths.append(solo)
    write_metric_csv(fig_dir / "stage2_data.csv", csv_rows, CSV_HEADER)
    paths.append(fig_dir / "stage2_data.csv")
    return paths


def make_stage3() -> list[Path]:
    apply_thesis_style()
    fig_dir = TC_ROOT / "stage3-multi-timestep/figures"
    unlink_stale(
        fig_dir,
        [
            "stage3-sim-vs-measured.png",
            "stage3-icx-tpn1.png",
            "stage3-icx-socket.png",
            "stage3-*-wall-scaling.png",
            "stage3-*-error.png",
            "*of_mpix*",
            "*rt_mpix*",
        ],
    )
    meas_dir = TC_ROOT / "stage3-multi-timestep/results/measured"
    meas = parse_measured_tsv(meas_dir / "stage3-measured.tsv")
    platforms = ["icx-tpn1", "icx-socket"]
    timesteps = [100, 1000, 10000]
    base_csv: list[list] = []
    pack: dict[str, dict] = {}
    for plat in platforms:
        pack[plat] = {
            k: [] for k in ("ts", "meas_w", "sim_w", "meas_m", "sim_m")
        }
        for nt in timesteps:
            cfg = f"{plat}-{nt}"
            m = meas.get(cfg)
            sim_dir = TC_ROOT / f"stage3-multi-timestep/results/simulated/{cfg}"
            slow = sim_wall(sim_dir)
            if not m or slow is None:
                print(f"  skip {cfg}: missing data")
                continue
            s, _rank = slow
            sr = _wall_rates(m, s)
            pack[plat]["ts"].append(nt)
            pack[plat]["meas_w"].append(m["wall_s"])
            pack[plat]["sim_w"].append(s)
            pack[plat]["meas_m"].append(m["mpix_s"])
            pack[plat]["sim_m"].append(sr["mpix_s"])
            base_csv.append(csv_row(cfg, m["wall_s"], s, m, sr))
            print(f"  {cfg}: meas={m['wall_s']:.2f} sim={s:.2f}")

    paths: list[Path] = []
    metric_map = [
        ("wall", "Wall time [s]", "meas_w", "sim_w", "s"),
        ("mpix", "Mpix/s", "meas_m", "sim_m", ""),
    ]
    usable = [p for p in platforms if pack[p]["ts"]]
    if not usable:
        print("stage3: insufficient points, skip")
        return paths
    for key, ylabel, mk, sk, suffix in metric_map:
        fig, axes = plt.subplots(1, len(usable), figsize=(5.6 * len(usable), 5.0), squeeze=False)
        for ax, plat in zip(axes[0], usable):
            draw_s3_panel(
                ax, pack[plat]["ts"], pack[plat][mk], pack[plat][sk],
                ylabel, S3_PANEL_TITLE.get(plat, plat), value_suffix=suffix,
                loglog=(key == "wall"),
            )
        finish_s3_combined(fig)
        out = fig_dir / f"stage3-{key}.png"
        save_figure(fig, out)
        paths.append(out)
        for plat in usable:
            fig_s, ax_s = plt.subplots(figsize=(5.8, 4.8))
            draw_s3_panel(
                ax_s, pack[plat]["ts"], pack[plat][mk], pack[plat][sk],
                ylabel, S3_PANEL_TITLE.get(plat, plat), value_suffix=suffix,
                loglog=(key == "wall"),
            )
            fig_s.subplots_adjust(top=0.82, left=0.14, right=0.86, bottom=0.14)
            fig_s.legend(
                handles=s3_legend_handles(),
                loc="upper center", ncol=3, bbox_to_anchor=(0.5, 0.98),
                frameon=False, fontsize=8,
            )
            solo = fig_dir / f"stage3-{plat}-{key}.png"
            save_figure(fig_s, solo)
            paths.append(solo)
    if base_csv:
        write_metric_csv(fig_dir / "stage3_data.csv", base_csv, CSV_HEADER)
        paths.append(fig_dir / "stage3_data.csv")
    return paths


def make_stage4() -> list[Path]:
    print("  Stage 4: no Gantt PNG (use Intel Trace Analyzer on STF).")
    apply_thesis_style()
    fig_dir = TC_ROOT / "stage4-idlewave/figures"
    fig_dir.mkdir(parents=True, exist_ok=True)
    unlink_stale(fig_dir, ["*of_mpix*", "*rt_mpix*"])
    meas_dir = TC_ROOT / "stage4-idlewave/results/measured"
    meas = parse_measured_tsv(meas_dir / "stage4-measured.tsv")
    slow = sim_wall(TC_ROOT / "stage4-idlewave/results/simulated/icx")
    m = meas.get("icx")
    paths: list[Path] = []
    if m and slow is not None:
        s, _rank = slow
        sr = _wall_rates(m, s)
        for key, ylabel, is_wall in S2_S4_METRICS:
            fig, ax = plt.subplots(figsize=(5.4, 4.8))
            sim_v, meas_v = _s2_values(m, sr, s, is_wall=is_wall, key=key)
            err = dmetrics.signed_error_pct(sim_v, meas_v)
            extra = f"wall  sim {s:.2f}s / meas {m['wall_s']:.2f}s"
            draw_s2_panel(
                ax, sim_v, meas_v, ylabel, S2_PANEL_TITLE["icx"], err, extra_box=extra,
            )
            fig.suptitle("Stage 4: Idlewave — Ray tracing + Optical flow (-O3)", fontsize=11, y=0.98)
            fig.subplots_adjust(top=0.82, bottom=0.16)
            out = fig_dir / f"stage4-{key}.png"
            save_figure(fig, out)
            paths.append(out)
        write_metric_csv(
            fig_dir / "stage4_metrics.csv",
            [csv_row("icx", m["wall_s"], s, m, sr)],
            CSV_HEADER,
        )
        paths.append(fig_dir / "stage4_metrics.csv")
        print(f"  stage4: meas={m['wall_s']:.3f} sim={s:.3f}")
    for label, path in (
        ("STF measured", TC_ROOT / "stage4-idlewave/results/measured-trace/icx/rtof-idlewave-measured.single.stf"),
        ("STF sim", TC_ROOT / "stage4-idlewave/results/simulated/icx/rtof-idlewave.single.stf"),
    ):
        print(f"  {label}: {path}" + ("  [OK]" if path.is_file() else "  [MISSING]"))
    return paths


def make_stage5() -> list[Path]:
    apply_thesis_style()
    fig_dir = TC_ROOT / "stage5-scaling/figures"
    unlink_stale(fig_dir, ["stage5-scaling-slowest-rank.png", "stage5-strong-icx.png", "stage5-weak-icx.png", "*of_mpix*", "*rt_mpix*"])
    meas_dir = TC_ROOT / "stage5-scaling/results/measured"
    meas = parse_measured_tsv(meas_dir / "stage5-measured.tsv")
    nodes_all = (1, 2, 4, 8)
    series: dict[str, dict] = {}
    base_csv: list[list] = []
    for kind in ("strong", "weak"):
        series[kind] = {k: [] for k in (
            "n", "meas_w", "sim_w", "meas_m", "sim_m", "meas_e", "sim_e",
        )}
        walls_meas, walls_sim = {}, {}
        for nodes in nodes_all:
            name = f"{kind}-icx-{nodes}node"
            m = meas.get(name)
            sim_dir = TC_ROOT / f"stage5-scaling/results/simulated/{name}"
            slow = sim_wall(sim_dir)
            if not m or slow is None:
                print(f"  skip {name}: missing data")
                continue
            s, _rank = slow
            walls_meas[nodes] = m["wall_s"]
            walls_sim[nodes] = s
            sr = _wall_rates(m, s)
            series[kind]["n"].append(nodes)
            series[kind]["meas_w"].append(m["wall_s"])
            series[kind]["sim_w"].append(s)
            series[kind]["meas_m"].append(m["mpix_s"])
            series[kind]["sim_m"].append(sr["mpix_s"])
            base_csv.append(csv_row(name, m["wall_s"], s, m, sr))
            print(f"  {name}: meas={m['wall_s']:.2f} sim={s:.2f}")
        t1m, t1s = walls_meas.get(1), walls_sim.get(1)
        for nodes in series[kind]["n"]:
            mv, sv = walls_meas[nodes], walls_sim[nodes]
            if kind == "strong":
                series[kind]["meas_e"].append(dmetrics.strong_efficiency(t1m or 0, mv, nodes))
                series[kind]["sim_e"].append(dmetrics.strong_efficiency(t1s or 0, sv, nodes))
            else:
                series[kind]["meas_e"].append(dmetrics.weak_efficiency(t1m or 0, mv))
                series[kind]["sim_e"].append(dmetrics.weak_efficiency(t1s or 0, sv))

    header_e = CSV_HEADER[:-1] + ["meas_efficiency", "sim_efficiency", "formula"]
    rows_e: list[list] = []
    idx = 0
    for kind in ("strong", "weak"):
        for i, _nodes in enumerate(series[kind]["n"]):
            row = base_csv[idx]
            idx += 1
            rows_e.append(row[:-1] + [f"{series[kind]['meas_e'][i]:.4f}", f"{series[kind]['sim_e'][i]:.4f}", row[-1]])

    paths: list[Path] = []

    def s5_legend_handles():
        return [
            Line2D([0], [0], color=C_MEAS, marker="s", lw=1.8, markersize=7, label=LEGEND_MEAS),
            Line2D([0], [0], color=C_SIM, marker="o", lw=1.8, markersize=7, label=LEGEND_SIM),
        ]

    def line_kind(ax, kind, ymeas, ysim, ylabel, title, *, zero_y: bool, log_y: bool, annotate: bool):
        xs = series[kind]["n"]
        ax.plot(xs, ymeas, "s-", color=C_MEAS, lw=1.8, markersize=7)
        ax.plot(xs, ysim, "o-", color=C_SIM, lw=1.8, markersize=7)
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
        if annotate:
            annotate_line_errors(ax, xs, ymeas, ysim)

    metric_specs = [
        ("wall", "Wall time [s]", "meas_w", "sim_w"),
        ("mpix", "Mpix/s", "meas_m", "sim_m"),
        ("efficiency", "Efficiency", "meas_e", "sim_e"),
    ]
    for key, ylabel, mk, sk in metric_specs:
        fig, axes = plt.subplots(1, 2, figsize=(11.2, 5.0))
        for ax, kind, title in (
            (axes[0], "strong", f"Strong scaling — {ylabel}"),
            (axes[1], "weak", f"Weak scaling — {ylabel}"),
        ):
            if not series[kind]["n"]:
                ax.set_visible(False)
                continue
            line_kind(
                ax, kind, series[kind][mk], series[kind][sk], ylabel, title,
                zero_y=(key != "efficiency"),
                log_y=(key == "wall" and kind == "weak"),
                annotate=True,
            )
            if key == "efficiency":
                ax.axhline(1.0, color="#888", ls="--", lw=0.8)
                ax.set_ylim(0, 1.25)
        fig.subplots_adjust(top=0.82, left=0.07, right=0.99, bottom=0.12, wspace=0.28)
        fig.suptitle(f"Stage 5 — {ylabel} · meas vs sim", fontsize=12, y=0.98)
        fig.legend(
            handles=s5_legend_handles(),
            loc="upper center", ncol=2, bbox_to_anchor=(0.5, 0.92),
            frameon=False, fontsize=9,
        )
        out = fig_dir / f"stage5-{key}.png"
        save_figure(fig, out)
        paths.append(out)
        for kind, title in (
            ("strong", f"Strong scaling — {ylabel}"),
            ("weak", f"Weak scaling — {ylabel}"),
        ):
            if not series[kind]["n"]:
                continue
            fig_s, ax_s = plt.subplots(figsize=(6.2, 4.6))
            line_kind(
                ax_s, kind, series[kind][mk], series[kind][sk], ylabel, title,
                zero_y=(key != "efficiency"),
                log_y=(key == "wall" and kind == "weak"),
                annotate=True,
            )
            if key == "efficiency":
                ax_s.axhline(1.0, color="#888", ls="--", lw=0.8)
                ax_s.set_ylim(0, 1.25)
            ax_s.legend(handles=s5_legend_handles(), fontsize=8)
            solo = fig_dir / f"stage5-{kind}-{key}.png"
            save_figure(fig_s, solo)
            paths.append(solo)
    if rows_e:
        write_metric_csv(fig_dir / "stage5_scaling_wall.csv", rows_e, header_e)
        paths.append(fig_dir / "stage5_scaling_wall.csv")
    return paths


def main() -> None:
    ap = argparse.ArgumentParser(
        description="Generate raytracing-opticalflow-demo figures (Stages 1–5; one PNG per metric)"
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
