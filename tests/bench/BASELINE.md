# Notification Bench Baseline

## Status

`baseline.json` currently contains **placeholder values (zeros)** because the
development host that initialised Phase 0 does not ship google/benchmark
or the Tizen runtime dependencies required to build the bench binaries.

Once the bench binaries can be built (on a Tizen GBS target image or any
host that exposes the required pkg-config modules) the real numbers
should be captured and committed using the procedure below.

## Host snapshot at the time of placeholder generation

| Field | Value |
|---|---|
| Date | 2026-04-22 |
| Kernel | 5.15.0-139-generic |
| OS | Ubuntu 20.04.6 LTS (Focal Fossa) |
| Compiler | gcc 9.4.0 |
| google/benchmark | **not installed** |
| Tizen runtime deps (aul, bundle, dlog, rpc-port, ...) | **not installed** |

## How to produce a real baseline

1. Build on a host with google/benchmark + Tizen deps:

   ```bash
   gbs build -A x86_64 --include-all --define "ENABLE_BENCH 1"
   # or, for a native build on a Tizen-like host:
   cmake -B build -DENABLE_BENCH=ON -DBUILD_TESTS=ON
   cmake --build build -j
   ```

2. Warm the CPU governor to `performance`:

   ```bash
   sudo cpupower frequency-set -g performance
   ```

3. Stop background services that compete for DB / D-Bus RTT (network-manager,
   snapd, etc.) so the measurement is repeatable.

4. Run the suite three times and keep the median:

   ```bash
   for i in 1 2 3; do
     tests/bench/run_bench.sh build/tests/bench tests/bench/results/run-$i
   done
   ```

5. Merge the five per-bench JSONs into `tests/bench/baseline.json`:

   ```bash
   python3 - <<'PY'
   import json, glob
   merged = {"context": {}, "benchmarks": []}
   for p in sorted(glob.glob("tests/bench/results/run-2/*.json")):
       with open(p) as f:
           data = json.load(f)
       merged["context"] = data.get("context", merged["context"])
       merged["benchmarks"].extend(data.get("benchmarks", []))
   with open("tests/bench/baseline.json", "w") as f:
       json.dump(merged, f, indent=2)
   PY
   ```

6. Validate reproducibility: compare runs 1 and 3 with
   `scripts/compare_bench.py --max-regression 0.10`. If any bench exceeds
   10 % variance between runs, re-run on a quieter host.

## Schema contract (assumed by scripts/compare_bench.py)

Each entry in `benchmarks` MUST expose:

- `name` (string)
- `cpu_time` (number, lower-is-better)
- `real_time` (number)
- `items_per_second` (number, higher-is-better)

The current placeholder honours that contract with zero values so
`compare_bench.py baseline.json baseline.json` exits 0 (regression == 0).
