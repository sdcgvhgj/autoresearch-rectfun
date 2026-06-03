"""
RectFun evaluation runner.
Compiles solve.cpp, runs the benchmark evaluator, reports score.
DO NOT MODIFY — this is the fixed evaluation harness.
Usage: python runner.py
"""

import os
import re
import subprocess
import sys
import time

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
SOLVE_CPP = os.path.join(SCRIPT_DIR, "solve.cpp")
SOLVE_BIN = os.path.join(SCRIPT_DIR, "solve")
BENCHMARK_DIR = os.path.join(SCRIPT_DIR, "benchmark")
EVALUATE_SH = os.path.join(BENCHMARK_DIR, "evaluate.sh")

def main():
    t_start = time.time()

    # 1. Compile solve.cpp
    compile_cmd = ["g++", "-std=c++17", "-O2", "-o", SOLVE_BIN, SOLVE_CPP]
    result = subprocess.run(compile_cmd, capture_output=True, text=True)
    if result.returncode != 0:
        print("---")
        print("compile_status:   FAIL")
        print(f"compile_error:    {result.stderr.strip().split(chr(10))[0]}")
        print("score:            0.00")
        print(f"total_seconds:    {time.time() - t_start:.1f}")
        sys.exit(0)

    # 2. Run benchmark evaluation
    os.chmod(SOLVE_BIN, 0o755)
    run_cmd = ["bash", EVALUATE_SH, SOLVE_BIN]
    result = subprocess.run(run_cmd, capture_output=True, text=True)

    t_end = time.time()
    total_seconds = t_end - t_start

    output = result.stdout

    # 3. Parse results
    compile_ok = True
    passed = 0
    failed = 0
    total_score = 0.0
    peak_time_ms = 0
    crash_count = 0

    output = re.sub(r'\x1b\[[0-9;]*m', '', output)
    for line in output.split("\n"):
        line = line.strip()

        # Parse per-case row: "1    1000000     999500    12ms    5.00 \033[0;32mAC\033[0m"
        parts = line.split()
        if len(parts) >= 6 and parts[0].isdigit():
            case_score = float(parts[4])
            time_str = parts[3].replace("ms", "")
            try:
                elapsed = int(time_str)
                if elapsed > peak_time_ms:
                    peak_time_ms = elapsed
            except ValueError:
                pass
            verdict = parts[5]
            if verdict in ("AC",):
                passed += 1
            else:
                failed += 1
                if verdict in ("TLE", "RE", "RE(0)", "RE(1)", "RE(2)", "RE(3)", "RE(4)"):
                    crash_count += 1

        # Parse summary
        if "通过:" in line:
            try:
                passed = int(line.split(":")[1].strip())
            except (ValueError, IndexError):
                pass
        if "失败:" in line:
            try:
                failed_part = line.split(":")[1].strip()
                failed = int(failed_part.split()[0])
            except (ValueError, IndexError):
                pass
        if "总得分:" in line:
            try:
                total_score = float(line.split()[-1])
            except (ValueError, IndexError):
                pass

    print("---")
    print(f"score:            {total_score:.2f}")
    print(f"passed_cases:     {passed}/30")
    print(f"failed_cases:     {failed}/30")
    print(f"crash_count:      {crash_count}")
    print(f"peak_ms:          {peak_time_ms}")
    print(f"total_seconds:    {total_seconds:.1f}")


if __name__ == "__main__":
    main()
