#!/bin/bash
set -euo pipefail

# ============================================================================
# RectFun 便携评测脚本
# 用法: ./evaluate.sh <选手二进制>
# 示例: ./evaluate.sh ./my_solution
# ============================================================================

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
CHECKER="$SCRIPT_DIR/checker"
DATA_DIR="$SCRIPT_DIR/data"
OUTPUT_DIR="${SCRIPT_DIR}/eval_output"

TIMEOUT_SEC=5

SOLUTION="${1:-}"
if [ -z "$SOLUTION" ]; then
    echo "用法: $0 <选手二进制>"
    echo "示例: $0 ./my_solution"
    exit 1
fi
if [ ! -f "$SOLUTION" ] || [ ! -x "$SOLUTION" ]; then
    echo -e "${RED}错误: 选手程序 '$SOLUTION' 不存在或不可执行${NC}"
    exit 1
fi
if [ ! -f "$CHECKER" ] || [ ! -x "$CHECKER" ]; then
    echo -e "${RED}错误: checker 未找到 ($CHECKER)${NC}"
    exit 1
fi

mkdir -p "$OUTPUT_DIR"

# 收集测试数据 (data_01.txt .. data_30.txt)
DATA_FILES=()
for i in $(seq -w 1 30); do
    f="$DATA_DIR/data_${i}.txt"
    [ -f "$f" ] && DATA_FILES+=("$f")
done
if [ ${#DATA_FILES[@]} -eq 0 ]; then
    echo -e "${RED}错误: 数据目录未找到 data_01.txt ~ data_30.txt${NC}"
    exit 1
fi

TOTAL_CASES=${#DATA_FILES[@]}
echo ""
echo -e "${CYAN}========================================${NC}"
echo -e "${CYAN}  RectFun 自动评测${NC}"
echo -e "${CYAN}========================================${NC}"
echo "选手程序: $SOLUTION"
echo "评估数据: $TOTAL_CASES 组"
echo "单组时限: ${TIMEOUT_SEC}s"
echo ""
printf "%-5s %10s %10s %8s %8s %s\n" "No." "n" "m" "Time(ms)" "Score" "Verdict"
echo "--------------------------------------------------------------------"

TOTAL_SCORE=0
PASSED=0
FAILED=0
TIMED_OUT=0
global_start=$(date +%s%3N)

for idx in "${!DATA_FILES[@]}"; do
    case_no=$((idx + 1))
    input_file="${DATA_FILES[$idx]}"
    output_file="${OUTPUT_DIR}/output_${case_no}.txt"
    err_file="${OUTPUT_DIR}/err_${case_no}.log"

    n=$(head -1 "$input_file" 2>/dev/null || echo "?")
    if [ "$n" = "?" ]; then
        printf "%-5s %10s %10s %8s %8s %s\n" \
            "$case_no" "-" "-" "-" "0.00" "${RED}READ_ERR${NC}"
        FAILED=$((FAILED + 1))
        continue
    fi

    # 1. 运行选手程序
    start_ns=$(date +%s%N)
    timeout "$TIMEOUT_SEC" "$SOLUTION" < "$input_file" > "$output_file" 2>"$err_file"
    exit_code=$?
    end_ns=$(date +%s%N)
    elapsed_ms=$(( (end_ns - start_ns) / 1000000 ))

    # 超时
    if [ $exit_code -eq 124 ] || [ $exit_code -eq 137 ]; then
        printf "%-5s %10s %10s %8s %8s %s\n" \
            "$case_no" "$n" "-" "${elapsed_ms}ms" "0.00" "${RED}TLE${NC}"
        TIMED_OUT=$((TIMED_OUT + 1))
        FAILED=$((FAILED + 1))
        continue
    fi
    # 崩溃
    if [ $exit_code -ne 0 ]; then
        printf "%-5s %10s %10s %8s %8s %s\n" \
            "$case_no" "$n" "-" "${elapsed_ms}ms" "0.00" "${RED}RE($exit_code)${NC}"
        FAILED=$((FAILED + 1))
        continue
    fi

    # 2. 运行 checker (第三个参数传 input 自身, 因 checker 直接验证 output vs input)
    checker_out=$("$CHECKER" "$input_file" "$output_file" "$input_file" 2>&1) || true
    checker_code=$?

    # 3. 解析结果
    if [ "$checker_code" -eq 0 ] || [ "$checker_code" -eq 7 ]; then
        m=$(echo "$checker_out" | grep -oP 'm=\K\d+' | head -1 || true)
        score_raw=$(echo "$checker_out" | grep -oP 'points\s+\K[\d.]+' | head -1 || true)
        if [ -z "$score_raw" ] && [ -n "$m" ] && [ "$n" != "?" ]; then
            score_raw="$(awk "BEGIN { printf \"%.6f\", ($n - $m) * 100.0 / $n }")"
        fi
        score="$(awk "BEGIN { printf \"%.2f\", ${score_raw:-0} }")"

        # 超 1s 算超时
        if [ "$elapsed_ms" -gt 3500 ]; then
            printf "%-5s %10s %10s %8s %8s %s\n" \
                "$case_no" "$n" "$m" "${elapsed_ms}ms" "0.00" "${RED}TLE(>1s)${NC}"
            TIMED_OUT=$((TIMED_OUT + 1))
            FAILED=$((FAILED + 1))
            continue
        fi

        printf "%-5s %10s %10s %8s %8s %s\n" \
            "$case_no" "$n" "${m:-?}" "${elapsed_ms}ms" "$score" "${GREEN}AC${NC}"
        TOTAL_SCORE=$(awk "BEGIN { printf \"%.2f\", $TOTAL_SCORE + $score }")
        PASSED=$((PASSED + 1))
    else
        case "$checker_code" in
            1) verdict="${RED}WA${NC}" ;;
            2) verdict="${RED}PE${NC}" ;;
            3) verdict="${RED}FAIL${NC}" ;;
            *) verdict="${RED}ERR($checker_code)${NC}" ;;
        esac
        m_try=$(head -1 "$output_file" 2>/dev/null || echo "-")
        printf "%-5s %10s %10s %8s %8s %b\n" \
            "$case_no" "$n" "$m_try" "${elapsed_ms}ms" "0.00" "$verdict"
        FAILED=$((FAILED + 1))
    fi
done

global_end=$(date +%s%3N)
global_elapsed=$(( (global_end - global_start) / 1000 ))

echo "--------------------------------------------------------------------"
echo ""
echo -e "${CYAN}========================================${NC}"
echo -e "${CYAN}  评测汇总${NC}"
echo -e "${CYAN}========================================${NC}"
echo -e "总用例数:  ${TOTAL_CASES}"
echo -e "${GREEN}通过:      ${PASSED}${NC}"
echo -e "${RED}失败:      ${FAILED}${NC}  (超时: ${TIMED_OUT})"
echo -e "${YELLOW}总得分:    ${TOTAL_SCORE}${NC}"
echo -e "总耗时:    ${global_elapsed}s"
echo -e "${CYAN}========================================${NC}"
