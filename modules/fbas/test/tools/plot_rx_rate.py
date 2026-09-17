import matplotlib.pyplot as plt
import numpy as np
import csv
import os
import re

pattern = re.compile(r"rx_rate_(\d+)\.csv")

datasets = []
for fname in os.listdir("."):
    m = pattern.match(fname)
    if m:
        datasets.append((int(m.group(1)), fname))

datasets.sort(key=lambda x: x[0])

if not datasets:
    raise FileNotFoundError("No rx_rate_<i>.csv files found")

cmap = plt.colormaps["tab10"]

plt.figure(figsize=(12, 6))

x = None
xticks = None

for k, (idx, file) in enumerate(datasets):

    color = cmap(k % 10)

    rates, avg, minv, maxv = [], [], [], []

    with open(file, "r") as f:
        reader = csv.DictReader(f)
        for row in reader:
            rates.append(float(row["msg_rate_hz"]))
            avg.append(float(row["eca_avg_us"]))
            minv.append(float(row["eca_min_us"]))
            maxv.append(float(row["eca_max_us"]))

    if x is None:
        x = np.arange(len(rates))
        xticks = rates

    offset = (k - len(datasets) / 2) * 0.15

    label = f"Emitter {idx}"

    # plot min-max + mean
    for i in range(len(x)):
        xi = x[i] + offset

        plt.vlines(
            xi,
            minv[i],
            maxv[i],
            color=color,
            alpha=0.6,
            linewidth=3
        )

        plt.scatter(
            xi,
            avg[i],
            color=color,
            s=50,
            edgecolors="black"
        )

        # -------- value label --------
        plt.text(
            xi + 0.05,
            avg[i] + 12,
            f"{int(round(avg[i]))}",
            ha="left",
            fontsize=8,
            color="black"
        )

# Axis formatting
plt.xticks(x, xticks)
plt.xlabel("Message Rate (Hz)")
plt.ylabel("Latency (us)")
plt.title("C2 Messaging Latency (6K messages by DM)")
plt.grid(True, linestyle="--", alpha=0.5)

# legend proxy (avoid clutter)
for k, (idx, _) in enumerate(datasets):
    plt.scatter([], [], color=cmap(k % 10), label=f"{idx} Emitter(s)")

plt.legend()

plt.tight_layout()
plt.show()
