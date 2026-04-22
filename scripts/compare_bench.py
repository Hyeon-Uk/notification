#!/usr/bin/env python3
#
# Copyright (c) 2026 Samsung Electronics Co., Ltd.
# SPDX-License-Identifier: Apache-2.0
#
"""compare_bench.py -- Phase 0 N0-T4 bench regression gate.

Compares two Google Benchmark JSON files (baseline vs. new) and exits
non-zero when any benchmark slowed down by more than --max-regression.

Rules:
  * regression_ratio = (new.cpu_time - base.cpu_time) / base.cpu_time
  * base.cpu_time == 0  -> skip comparison (placeholder baseline).
  * missing entries in either file are reported but not failing unless
    --strict is passed.

Usage:
  compare_bench.py baseline.json new.json [--max-regression 0.10]
                   [--key cpu_time|real_time|items_per_second]
                   [--strict]
"""

from __future__ import annotations

import argparse
import json
import sys
from typing import Dict, List


def load(path: str) -> Dict[str, Dict]:
    with open(path, "r", encoding="utf-8") as f:
        data = json.load(f)
    out: Dict[str, Dict] = {}
    for b in data.get("benchmarks", []):
        name = b.get("name")
        if name:
            out[name] = b
    return out


def regression_ratio(base_val: float, new_val: float, higher_is_better: bool) -> float:
    if base_val == 0:
        return 0.0
    if higher_is_better:
        # throughput metric: new < base means regression -> positive ratio
        return (base_val - new_val) / base_val
    # latency metric: new > base means regression -> positive ratio
    return (new_val - base_val) / base_val


def main(argv: List[str]) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("baseline")
    ap.add_argument("new")
    ap.add_argument("--max-regression", type=float, default=0.10,
                    help="fail when ratio > this (default 0.10)")
    ap.add_argument("--key", default="cpu_time",
                    choices=["cpu_time", "real_time", "items_per_second"])
    ap.add_argument("--strict", action="store_true",
                    help="fail when a benchmark is missing from either side")
    args = ap.parse_args(argv)

    higher_is_better = args.key == "items_per_second"

    base = load(args.baseline)
    new = load(args.new)

    all_names = sorted(set(base) | set(new))
    rc = 0
    print(f"[bench] key={args.key} max_regression={args.max_regression:.2%}")
    for name in all_names:
        if name not in base:
            status = "NEW"
            if args.strict:
                rc = 1
            print(f"[bench] {status:5s} {name}")
            continue
        if name not in new:
            status = "GONE"
            if args.strict:
                rc = 1
            print(f"[bench] {status:5s} {name}")
            continue
        base_val = float(base[name].get(args.key, 0.0))
        new_val = float(new[name].get(args.key, 0.0))
        if base_val == 0:
            print(f"[bench] SKIP  {name} (baseline {args.key} == 0)")
            continue
        ratio = regression_ratio(base_val, new_val, higher_is_better)
        status = "OK"
        if ratio > args.max_regression:
            status = "FAIL"
            rc = 1
        print(
            f"[bench] {status:5s} {name} base={base_val:.3f} "
            f"new={new_val:.3f} ratio={ratio:+.2%}"
        )

    if rc:
        print(f"[bench] regression > {args.max_regression:.2%} detected")
    else:
        print("[bench] all benches within tolerance")
    return rc


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
