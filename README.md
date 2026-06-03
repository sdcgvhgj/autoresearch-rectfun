# AutoResearch RectFun

Autonomous algorithm optimization via LLM agent on the **RectFun — Minimum Rectangle Partition** benchmark.

## Problem

Given `n` non-overlapping axis-aligned rectangles, find a **minimum rectangle partition** of their union — the smallest number `m` of non-overlapping rectangles that cover exactly the same area.

**Score per test case**: `(n - m) / n × 100`

**30 test cases**, each up to 10⁶ rectangles with coordinates in `[1, 16384] × [1, 1048576]`. 1-second time limit per case.

## How it works

```
┌─────────────┐     ┌──────────┐     ┌──────────────┐
│  program.md  │────▸│ LLM Agent │────▸│   runner.py  │
│ (instructions)│     │ edits     │     │ compile + eval│
└─────────────┘     │ solve.cpp │     └──────┬───────┘
                    └──────────┘            │
                         ▲                  │
                         │    score  ◁──────┘
                         │
                    ┌────┴─────┐
                    │ keep/discard │
                    └──────────────┘
```

- **`program.md`** — Agent instructions. Human edits this to tune research strategy.
- **`solve.cpp`** — C++ solution file. The agent modifies this autonomously.
- **`runner.py`** — Fixed harness. Compiles `solve.cpp`, runs the benchmark evaluator, reports the score. Read-only.

Inspired by [Karpathy's autoresearch](https://github.com/karpathy/autoresearch).

## Quick start

```bash
# Clone this repo
git clone <this-repo>
cd autoresearch-rectfun

# Run a baseline evaluation
python runner.py

# Expected: score 0.00 (trivial baseline)
```

Then point your LLM agent at `program.md` to begin autonomous optimization.

## Project structure

```
.
├── program.md          # Agent instructions
├── runner.py           # Evaluation harness (do not modify)
├── solve.cpp           # Solution code (agent modifies)
├── benchmark/          # The RectFun benchmark
│   ├── evaluate.sh     # Batch evaluator
│   ├── checker         # Output validator binary
│   └── data/           # 30 test cases
├── rectfun_benchmark.zip  # Original benchmark archive
└── .gitignore
```
