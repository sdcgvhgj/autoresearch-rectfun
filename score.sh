#!/bin/bash
# Deterministic scorer: runs ./solve on all cases, sums (n-m)/n*100, reports max time.
BIN="${1:-./solve}"
total=0; peak=0; out=""
for i in $(seq -w 1 30); do
  f="benchmark/data/data_${i}.txt"
  n=$(head -1 "$f")
  s=$(date +%s%N)
  m=$("$BIN" < "$f" | head -1)
  e=$(( ($(date +%s%N) - s)/1000000 ))
  [ "$e" -gt "$peak" ] && peak=$e
  sc=$(awk "BEGIN{printf \"%.2f\",($n-$m)*100.0/$n}")
  total=$(awk "BEGIN{printf \"%.2f\",$total+$sc}")
  out="${out}${i} n=$n m=$m ${e}ms $sc\n"
done
printf "$out"
echo "TOTAL=$total PEAK=${peak}ms"
