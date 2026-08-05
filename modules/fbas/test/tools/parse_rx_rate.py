import re
import csv
import argparse

# ---------- CLI arguments ----------
parser = argparse.ArgumentParser(description="Parse ECA delays from log file")
parser.add_argument("input_file", help="Path to input log file")
parser.add_argument("output_file", help="Path to output CSV file")

args = parser.parse_args()

# ---------- Regex patterns ----------
rate_pattern = re.compile(r"msg rate=(\d+\.?\d*)")
eca_pattern = re.compile(r"ECA delay\s*:\s*(\d+)\s+(\d+)\s+(\d+)")

rates = []
eca_avg = []
eca_min = []
eca_max = []

current_rate = None

# ---------- Parse file ----------
with open(args.input_file, "r") as f:
    for line in f:

        rate_match = rate_pattern.search(line)
        if rate_match:
            current_rate = float(rate_match.group(1))

        eca_match = eca_pattern.search(line)
        if eca_match and current_rate is not None:
            rates.append(current_rate)
            eca_avg.append(int(eca_match.group(1)))
            eca_min.append(int(eca_match.group(2)))
            eca_max.append(int(eca_match.group(3)))
            current_rate = None

# ---------- Write CSV ----------
with open(args.output_file, "w", newline="") as f:
    writer = csv.writer(f)
    writer.writerow(["msg_rate_hz", "eca_avg_us", "eca_min_us", "eca_max_us"])

    for i in range(len(rates)):
        writer.writerow([rates[i], eca_avg[i], eca_min[i], eca_max[i]])

print(f"Saved parsed data to {args.output_file}")
