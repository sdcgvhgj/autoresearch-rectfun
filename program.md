# autoresearch-rectfun

This is an experiment to have the LLM autonomously improve a C++ solution for the **RectFun — Minimum Rectangle Partition** problem.

## Problem

Given `n` non-overlapping axis-aligned rectangles on a 2D grid, find the **minimum rectangle partition** of their union. Output `m` rectangles that cover exactly the same area with no overlaps, minimizing `m`.

**Score per test case**: `(n - m) / n * 100` (range 0–100, higher is better)
**Total score**: sum across all 30 test cases (max theoretical: variable per case)

Constraints:
- `1 ≤ n ≤ 10^6`
- `1 ≤ x1 ≤ x2 ≤ 16384`, `1 ≤ y1 ≤ y2 ≤ 1048576`
- Time limit: 1 second per test case

## Setup

To set up a new experiment, work with the user to:

1. **Agree on a run tag**: propose a tag based on today's date (e.g. `jun03`). The branch `autoresearch/<tag>` must not already exist — this is a fresh run.
2. **Create the branch**: `git checkout -b autoresearch/<tag>` from current master.
3. **Read the in-scope files**:
   - `program.md` — this file. Your instructions.
   - `runner.py` — the fixed evaluation harness. Compiles `solve.cpp`, runs the benchmark, reports score. **Do not modify**.
   - `solve.cpp` — **the file you modify**. Contains the entire solution algorithm.
   - `benchmark/README.md` — problem statement and constraints.
4. **Verify the benchmark exists**: Check that `benchmark/evaluate.sh`, `benchmark/checker`, and `benchmark/data/` exist.
5. **Initialize results.tsv**: Create `results.tsv` with just the header row. The baseline will be recorded after the first run.
6. **Confirm and go**.

Once you get confirmation, kick off the experimentation.

## Experimentation

**What you CAN do:**
- Modify `solve.cpp` — this is the only file you edit. Everything is fair game: the algorithm, data structures, sorting strategies, output format, etc.

**What you CANNOT do:**
- Modify `runner.py`. It is read-only. It contains the fixed compilation and evaluation pipeline.
- Modify benchmark files (`benchmark/`). These are fixed test data.
- Install new packages or change the build toolchain.

**The goal is simple: maximize the total score.** The only constraint is that each test case must run within 1 second and the output must be correct (area-preserving, non-overlapping).

**Performance criterion**: Speed matters — if any case takes >1000ms it counts as TLE and scores 0. Algorithms with better time complexity are rewarded.

**Correctness is critical**: All output rectangles must be pairwise non-overlapping, cover exactly the input area, and stay within coordinate bounds. Any WA/PE/Fail on any case = 0 points for that case.

**The first run**: Your very first run should always be to establish the baseline, so you will run the solution as-is.

## Output format

Once the runner finishes it prints a summary like this:

```
---
score:            145.67
passed_cases:     30/30
failed_cases:     0/30
crash_count:      0
peak_ms:          234
total_seconds:    8.3
```

You can extract the key metric from the runner log:

```
grep "^score:" run.log
```

## Experiment loop

The experiment runs on a dedicated branch (e.g. `autoresearch/jun03`).

LOOP FOREVER:

1. Look at the git state: the current branch/commit.
2. Read in-scope files for context: `program.md`, `runner.py`, `solve.cpp`, `benchmark/README.md`.
3. Come up with an experimental idea (algorithmic improvement, better data structure, etc).
4. Modify `solve.cpp` with the idea.
5. git commit with a brief description.
6. Run: `python runner.py > run.log 2>&1` (redirect everything).
7. Read results: `grep "^score:" run.log`
8. If the grep output is empty, the run crashed. Read `tail -n 30 run.log` for the error.
9. Log results to `results.tsv` (tab-separated). Do NOT commit results.tsv.
10. If score improved (higher), keep the commit ("advance" the branch).
11. If score is equal or worse, git reset back to the previous commit.

## Logging results

The TSV has a header row and 5 columns:

```
commit	score	peak_ms	status	description
```

1. git commit hash (short, 7 chars)
2. total score (e.g. 145.67) — use 0.00 for crashes
3. peak time in ms (e.g. 234) — use 0 for crashes
4. status: `keep`, `discard`, or `crash`
5. short text description of what this experiment tried

Example:

```
commit	score	peak_ms	status	description
a1b2c3d	145.67	234	keep	baseline
b2c3d4e	152.30	280	keep	add horizontal merge pass
c3d4e5f	145.67	230	discard	revert — try interval tree
d4e5f6g	0.00	0	crash	compile error: missing include
```

## Crashes

If a run crashes (compile error, segfault, TLE on many cases, etc.):
- If it's something dumb and easy to fix (typo, missing include), fix it and re-run.
- If the idea is fundamentally broken, log "crash" and move on.

## NEVER STOP

Once the experiment loop has begun, do NOT pause to ask if you should continue. Do NOT ask "should I keep going?". The human might be asleep. You are autonomous. The loop runs until the human interrupts you, period.


