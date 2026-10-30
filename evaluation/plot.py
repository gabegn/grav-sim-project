#!/usr/bin/env python3
"""Makes the figures and summary tables of the performance and energy evaluation.

Reads evaluation/results.csv (written by measure.sh on avignon) and writes:
  evaluation/figures/objects.{pdf,png}     time, energy and power vs number of objects
  evaluation/figures/iterations.{pdf,png}  time, energy and power vs number of iterations
  evaluation/figures/official.{pdf,png}    the two official use cases, AOS vs SOA
  evaluation/summary.csv, evaluation/summary.md  mean and standard deviation per configuration

Every point is the mean of the repetitions of one configuration.
Usage: python3 evaluation/plot.py   (needs matplotlib)
"""

import csv
import statistics
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402

HERE = Path(__file__).resolve().parent
RESULTS = HERE / "results.csv"
FIGURES = HERE / "figures"

# Colors: categorical slots 1 and 2 of the reference palette (validated for color vision
# deficiencies on white). AOS and SOA also differ in marker shape, so the figures still
# work in black-and-white print.
SURFACE = "#ffffff"
TEXT = "#0b0b0b"
TEXT_SECONDARY = "#52514e"
GRID = "#e4e3df"
REFERENCE = "#9a9893"
STYLE = {
    "aos": {"label": "AOS", "color": "#2a78d6", "marker": "o"},
    "soa": {"label": "SOA", "color": "#eb6834", "marker": "s"},
}
VERSIONS = ("aos", "soa")
TIME_LIMITS = {1000: 3.0, 10000: 50.0}  # minimum execution time to pass, official cases

plt.rcParams.update({
    "font.size": 8,
    "axes.titlesize": 9,
    "axes.labelsize": 8,
    "axes.edgecolor": GRID,
    "axes.labelcolor": TEXT_SECONDARY,
    "axes.titlecolor": TEXT,
    "axes.titleweight": "bold",
    "axes.titlelocation": "left",
    "xtick.color": TEXT_SECONDARY,
    "ytick.color": TEXT_SECONDARY,
    "xtick.major.size": 0,
    "ytick.major.size": 0,
    "axes.grid": True,
    "axes.grid.axis": "y",
    "grid.color": GRID,
    "grid.linewidth": 0.6,
    "axes.spines.top": False,
    "axes.spines.right": False,
    "figure.facecolor": SURFACE,
    "axes.facecolor": SURFACE,
    "legend.frameon": False,
    "pdf.fonttype": 42,
})


def load(path):
    """Groups the runs by (experiment, version, objects, iterations)."""
    groups = {}
    with open(path, newline="") as file:
        for row in csv.DictReader(file):
            key = (row["experiment"], row["version"], int(row["objects"]), int(row["iterations"]))
            groups.setdefault(key, []).append(row)
    return groups


def summarize(runs):
    """Mean and standard deviation of time, energy and power for one configuration."""
    seconds = [float(run["seconds"]) for run in runs]
    pkg = [float(run["energy_pkg_j"]) for run in runs]
    cores = [float(run["energy_cores_j"]) for run in runs]
    power = [e / t for e, t in zip(pkg, seconds)]

    def sd(values):
        return statistics.stdev(values) if len(values) > 1 else 0.0

    return {
        "runs": len(runs),
        "seconds": statistics.mean(seconds), "seconds_sd": sd(seconds),
        "pkg": statistics.mean(pkg), "pkg_sd": sd(pkg),
        "cores": statistics.mean(cores), "cores_sd": sd(cores),
        "power": statistics.mean(power), "power_sd": sd(power),
    }


def series(stats, version, axis):
    """Points of one scalability series, sorted by objects or iterations.

    The 10000-object point of the objects series is the official 10K use case
    (same input and 100 iterations), so it is taken from there.
    """
    points = {}
    for (experiment, ver, objects, iterations), value in stats.items():
        if ver != version:
            continue
        if axis == "objects" and (experiment == "objects" or (experiment == "official" and objects == 10000)):
            points[objects] = value
        if axis == "iterations" and experiment == "iterations":
            points[iterations] = value
    return sorted(points.items())


def plot_line(ax, points, version, field, label_end=False):
    style = STYLE[version]
    xs = [x for x, _ in points]
    ys = [value[field] for _, value in points]
    ax.plot(xs, ys, color=style["color"], linewidth=2, solid_capstyle="round",
            solid_joinstyle="round", marker=style["marker"], markersize=5,
            markeredgecolor=SURFACE, markeredgewidth=1.2, label=style["label"], zorder=3)
    if label_end:
        ax.annotate(style["label"], (xs[-1], ys[-1]), xytext=(6, 0), textcoords="offset points",
                    va="center", color=TEXT_SECONDARY, fontsize=8)


def scalability_figure(stats, axis, xlabel, filename, quadratic=False):
    fig, axes = plt.subplots(1, 3, figsize=(7.2, 2.5), constrained_layout=True)
    panels = [("seconds", "Execution time", "seconds"),
              ("pkg", "Energy (package)", "joules"),
              ("power", "Average power (package)", "watts")]
    for ax, (field, title, unit) in zip(axes, panels):
        for version in VERSIONS:
            # The power lines almost coincide, so their end labels would overlap: the legend
            # identifies them there
            plot_line(ax, series(stats, version, axis), version, field, label_end=field != "power")
        ax.set_title(title)
        ax.set_ylabel(unit)
        ax.set_xlabel(xlabel)
        ax.set_ylim(bottom=0)
        ax.margins(x=0.12)
        if field == "power":
            ax.set_ylim(0, 12)
    if quadratic:
        # Gray reference: time growing with the square of the number of objects,
        # anchored at the AOS time for 1000 objects
        points = series(stats, "aos", axis)
        x0, t0 = points[0][0], points[0][1]["seconds"]
        xs = [x for x, _ in points]
        axes[0].plot(xs, [t0 * (x / x0) ** 2 for x in xs], color=REFERENCE, linewidth=1, zorder=2)
        axes[0].annotate("quadratic\n(from AOS at 1000)", (xs[3], t0 * (xs[3] / x0) ** 2),
                         xytext=(-4, 4), textcoords="offset points", ha="right",
                         color=TEXT_SECONDARY, fontsize=7)
        axes[0].set_ylim(0, max(value["seconds"] for _, value in series(stats, "soa", axis)) * 1.25)
    handles, labels = axes[0].get_legend_handles_labels()
    fig.legend(handles, labels, loc="outside upper right", ncol=2)
    save(fig, filename)


def official_figure(stats, filename):
    fig, axes = plt.subplots(1, 4, figsize=(7.2, 2.3), constrained_layout=True)
    cases = [(1000, 500, "1K objects, 500 it."), (10000, 100, "10K objects, 100 it.")]
    panels = [(case, field) for field in ("seconds", "pkg") for case in cases]
    for ax, ((objects, iterations, name), field) in zip(axes, panels):
        values = [stats[("official", version, objects, iterations)][field] for version in VERSIONS]
        positions = range(len(VERSIONS))
        ax.bar(positions, values, width=0.55, color=[STYLE[v]["color"] for v in VERSIONS],
               edgecolor=SURFACE, linewidth=2, zorder=3)
        unit = "s" if field == "seconds" else "J"
        for position, value in zip(positions, values):
            text = f"{value:.2f} {unit}" if value < 100 else f"{value:.0f} {unit}"
            ax.annotate(text, (position, value), xytext=(0, 3), textcoords="offset points",
                        ha="center", va="bottom", color=TEXT, fontsize=7.5)
        ax.set_xticks(list(positions), [STYLE[v]["label"] for v in VERSIONS])
        ax.set_title(name, fontsize=8)
        top = max(values) * 1.3
        if field == "seconds":
            limit = TIME_LIMITS[objects]
            ax.axhline(limit, color=REFERENCE, linewidth=1, zorder=2)
            ax.annotate(f"limit {limit:g} s", (1.45, limit), xytext=(0, 3), textcoords="offset points",
                        ha="right", va="bottom", color=TEXT_SECONDARY, fontsize=7)
            top = max(top, limit * 1.15)
            ax.set_ylabel("seconds")
        else:
            ax.set_ylabel("joules (package)")
        ax.set_ylim(0, top)
        ax.set_xlim(-0.6, 1.6)
    fig.suptitle("Official use cases: execution time (left) and energy (right)", x=0.01,
                 ha="left", fontsize=9, fontweight="bold", color=TEXT)
    save(fig, filename)


def save(fig, name):
    FIGURES.mkdir(exist_ok=True)
    for extension in ("pdf", "png"):
        fig.savefig(FIGURES / f"{name}.{extension}", dpi=200)
    plt.close(fig)


def write_tables(stats):
    rows = []
    for (experiment, version, objects, iterations), value in sorted(stats.items()):
        rows.append({"experiment": experiment, "version": version, "objects": objects,
                     "iterations": iterations, **value})
    fields = list(rows[0].keys())
    with open(HERE / "summary.csv", "w", newline="") as file:
        writer = csv.DictWriter(file, fieldnames=fields)
        writer.writeheader()
        for row in rows:
            writer.writerow({k: (f"{v:.4f}" if isinstance(v, float) else v) for k, v in row.items()})

    lines = ["# Measurement summary (avignon1, mean of repetitions)", "",
             "| Experiment | Version | Objects | Iterations | Runs | Time (s) | SD (s) "
             "| Package energy (J) | Core energy (J) | Power (W) |",
             "|---|---|---:|---:|---:|---:|---:|---:|---:|---:|"]
    for row in rows:
        lines.append(f"| {row['experiment']} | {row['version'].upper()} | {row['objects']} "
                     f"| {row['iterations']} | {row['runs']} | {row['seconds']:.3f} "
                     f"| {row['seconds_sd']:.3f} | {row['pkg']:.2f} | {row['cores']:.2f} "
                     f"| {row['power']:.2f} |")
    lines += ["", "## SOA compared with AOS", "",
              "| Experiment | Objects | Iterations | Time SOA / AOS | Energy SOA / AOS |",
              "|---|---:|---:|---:|---:|"]
    for (experiment, version, objects, iterations), value in sorted(stats.items()):
        if version != "aos":
            continue
        soa = stats.get((experiment, "soa", objects, iterations))
        if soa:
            lines.append(f"| {experiment} | {objects} | {iterations} "
                         f"| {soa['seconds'] / value['seconds']:.2f} | {soa['pkg'] / value['pkg']:.2f} |")
    (HERE / "summary.md").write_text("\n".join(lines) + "\n")


def main():
    stats = {key: summarize(runs) for key, runs in load(RESULTS).items()}
    scalability_figure(stats, "objects", "number of objects (100 iterations)", "objects", quadratic=True)
    scalability_figure(stats, "iterations", "number of iterations (1K objects)", "iterations")
    official_figure(stats, "official")
    write_tables(stats)
    print(f"Figures written to {FIGURES}, tables to {HERE / 'summary.md'}")


if __name__ == "__main__":
    main()
