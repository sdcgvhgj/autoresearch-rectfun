# RectFun — Minimum Rectangle Partition

## Problem Statement

You are given `n` non-overlapping rectangles on a 2D grid. Each rectangle is described by four integers `x1, x2, y1, y2` (`1 ≤ x1 ≤ x2 ≤ 2^14`, `1 ≤ y1 ≤ y2 ≤ 2^20`), representing all cells with column numbers from `x1` to `x2` (inclusive) and row numbers from `y1` to `y2` (inclusive).

Your task is to find the **minimum rectangle partition** of the union of these rectangles. Output `m` rectangles such that:

- All output rectangles are pairwise non-overlapping (no common cells).
- The union of all output rectangles is exactly equal to the union of all input rectangles.
- `m` is as small as possible.

### Input Format

```
n
x1 x2 y1 y2
x1 x2 y1 y2
...
```

First line: `n` (1 ≤ n ≤ 10^6). Next `n` lines: four integers `x1 x2 y1 y2`.

It is guaranteed that any two input rectangles do not overlap (they may share borders but have no common cells).

### Output Format

```
m
x1 x2 y1 y2
x1 x2 y1 y2
...
```

### Scoring

Score for each test case: `(n - m) / n * 100`

Total score = sum of scores across all 30 test cases.

### Constraints

- Time limit: 1 second per test case
- Valid ranges: `1 ≤ x1 ≤ x2 ≤ 16384`, `1 ≤ y1 ≤ y2 ≤ 1048576`
- Output rectangles must be pairwise non-overlapping and cover exactly the input area

## Evaluation

```bash
# Compile your solution
g++ -std=c++17 -O2 -o my_sol my_sol.cpp

# Run evaluation
./evaluate.sh ./my_sol
```

The evaluator will:
1. Run your program against each of the 30 test cases with a 2-second timeout
2. Validate output correctness (non-overlap, area coverage, coordinate bounds)
3. Report per-case score and total score

### Output directory

Evaluation outputs (your program's stdout/stderr per case) are saved under `eval_output/`.

## Example

**Input:**
```
3
1 1 4 6
2 2 1 6
3 3 1 3
```

**Output:**
```
2
2 3 1 3
1 2 4 6
```

Score: `(3-2)/3 * 100 = 33.33`

---

Based on the algorithm from: [Partition into Rectangles](https://arxiv.org/pdf/0908.3916) (Section 3)
