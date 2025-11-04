#!/bin/bash
set -euo pipefail

# --- Config ---
ROOT_DIR=".."                 # Project root
SRC="$ROOT_DIR/src/main.c"
BUILD_DIR="$ROOT_DIR/build"
BIN="$BUILD_DIR/horse_race_mutex"
TIMEOUT_SEC=15

EXPECTED_SUM=37550402023
EXPECTED_COUNT=78498

TOTAL_TESTS=6
SCORE=0

OUT_FILE="perf_stdout.txt"
ERR_FILE="perf_stderr.txt"

GREEN='\033[0;32m'; RED='\033[0;31m'; YEL='\033[0;33m'; NC='\033[0m'

# ---------------- Helpers ----------------
run_with_timeout() {
  local t="$1"; shift
  set +e
  ( "$@" >"$OUT_FILE" 2>"$ERR_FILE" ) &
  local pid=$!
  local elapsed=0
  while kill -0 "$pid" 2>/dev/null; do
    sleep 1
    elapsed=$((elapsed+1))
    if [ "$elapsed" -ge "$t" ]; then
      kill -TERM "$pid" 2>/dev/null || true
      sleep 1
      kill -KILL "$pid" 2>/dev/null || true
      wait "$pid" 2>/dev/null || true
      echo -e "${RED}Timeout reached (${t}s)${NC}"
      set -e
      return 124
    fi
  done
  wait "$pid"
  local rc=$?
  set -e
  return "$rc"
}

bin_has_symbol() {
  local sym="$1"
  if command -v nm >/dev/null 2>&1; then
    { nm -u "$BIN"; nm -D "$BIN"; } 2>/dev/null | grep -Eiq "\b${sym}\b" && return 0
  fi
  if command -v readelf >/dev/null 2>&1; then
    readelf -Ws "$BIN" 2>/dev/null | grep -Eiq "\b${sym}\b" && return 0
  fi
  if command -v strings >/dev/null 2>&1; then
    strings "$BIN" 2>/dev/null | grep -Eiq "\b${sym}\b" && return 0
  fi
  return 1
}

bin_has_any_mutex_symbol() {
  local re='\bpthread_mutex_(init|lock|unlock|destroy)\b'
  if command -v nm >/dev/null 2>&1; then
    { nm -u "$BIN"; nm -D "$BIN"; } 2>/dev/null | grep -Eiq "$re" && return 0
  fi
  if command -v readelf >/dev/null 2>&1; then
    readelf -Ws "$BIN" 2>/dev/null | grep -Eiq "$re" && return 0
  fi
  if command -v strings >/dev/null 2>&1; then
    strings "$BIN" 2>/dev/null | grep -Eiq "$re" && return 0
  fi
  return 1
}

# Print all numeric tokens (with thousands separators), normalized without , .
extract_numbers() {
  if [ -t 0 ]; then
    grep -Eo '[0-9][0-9,\.]*' "$1" | tr -d ',.'
  else
    grep -Eo '[0-9][0-9,\.]*' | tr -d ',.'
  fi
}

# Prefer the block after "=== Result ==="; take the LAST number as total SUM.
# Fallback: take the maximum number seen in the entire output.
extract_sum_number() {
  if grep -q '^=== Result ===' "$OUT_FILE"; then
    awk 'found{print} /^=== Result ===$/{found=1}' "$OUT_FILE" \
      | extract_numbers /dev/stdin \
      | tail -n 1
    return 0
  fi
  mapfile -t nums < <(extract_numbers "$OUT_FILE")
  if [ "${#nums[@]}" -eq 0 ]; then echo ""; return 0; fi
  local max=0 n
  for n in "${nums[@]}"; do
    [[ "$n" =~ ^[0-9]+$ ]] || continue
    (( n > max )) && max="$n"
  done
  echo "$max"
}

# Prefer the block after "=== Result ==="; take the FIRST number as COUNT.
# Fallback: pick a plausible count (smallest number >100 and <=1e6).
extract_count_number() {
  if grep -q '^=== Result ===' "$OUT_FILE"; then
    awk 'found{print; exit} /^=== Result ===$/{found=1}' "$OUT_FILE" \
      | extract_numbers /dev/stdin \
      | head -n 1
    return 0
  fi
  mapfile -t nums < <(extract_numbers "$OUT_FILE")
  local best=""
  for n in "${nums[@]}"; do
    [[ "$n" =~ ^[0-9]+$ ]] || continue
    if (( n > 100 && n <= 1000000 )); then
      if [ -z "$best" ] || (( n < best )); then best="$n"; fi
    fi
  done
  echo "${best:-}"
}

# Find time.{txt|xls|csv|png|jpg} under test/ (case-insensitive base name)
find_time_artifact() {
  local test_dir="$ROOT_DIR/test"
  local ext f
  for ext in txt xls csv png jpg; do
    for f in "$test_dir"/[Tt][Ii][Mm][Ee]."$ext"; do
      [ -e "$f" ] || continue
      [ -f "$f" ] || continue
      echo "$f|$ext"
      return 0
    done
  done
  return 1
}

# ---------------- Setup: Build (no score impact) ----------------
echo "--- Setup: Building (no score impact) ---"
mkdir -p "$BUILD_DIR"
if gcc -O2 -Wall -Wextra -std=c11 -pthread "$SRC" -lm -o "$BIN"; then
  echo -e "${GREEN}Build OK${NC}"
else
  echo -e "${RED}Build failed — cannot continue tests${NC}"
  exit 1
fi
echo "-----------------------------------------"

# ---------------- Setup: Run (no score impact) ----------------
echo "--- Setup: Running (timeout ${TIMEOUT_SEC}s) ---"
if run_with_timeout "$TIMEOUT_SEC" "$BIN"; then
  echo -e "${GREEN}Run OK${NC}"
else
  echo -e "${RED}Execution failed or timed out${NC}"
  echo "Tests Passed: $SCORE / $TOTAL_TESTS"
  exit 1
fi
echo "-----------------------------------------------"

# ---------------- Test 1: pthread_create ----------------
if bin_has_symbol "pthread_create"; then
  echo -e "${GREEN}Test 1: pthread_create found${NC}"
  SCORE=$((SCORE+1))
else
  echo -e "${RED}Test 1: pthread_create NOT found${NC}"
fi

# ---------------- Test 2: pthread_join ----------------
if bin_has_symbol "pthread_join"; then
  echo -e "${GREEN}Test 2: pthread_join found${NC}"
  SCORE=$((SCORE+1))
else
  echo -e "${RED}Test 2: pthread_join NOT found${NC}"
fi

# ---------------- Test 3: pthread_mutex_* (init/lock/unlock/destroy) ----------------
if bin_has_any_mutex_symbol; then
  echo -e "${GREEN}Test 3: pthread_mutex usage detected${NC}"
  SCORE=$((SCORE+1))
else
  echo -e "${RED}Test 3: pthread_mutex usage NOT detected${NC}"
fi

# ---------------- Test 4: SUM result ----------------
sum_num="$(extract_sum_number)"
if [ -z "$sum_num" ]; then
  echo -e "${RED}Test 4: could not extract total SUM from output${NC}"
else
  if [ "$sum_num" -eq "$EXPECTED_SUM" ] 2>/dev/null; then
    echo -e "${GREEN}Test 4: total SUM matches expected ($sum_num)${NC}"
    SCORE=$((SCORE+1))
  else
    echo -e "${RED}Test 4: expected $EXPECTED_SUM, got $sum_num${NC}"
  fi
fi

# ---------------- Test 5: COUNT result (NEW) ----------------
count_num="$(extract_count_number)"
if [ -z "$count_num" ]; then
  echo -e "${RED}Test 5: could not extract total COUNT from output${NC}"
else
  if [ "$count_num" -eq "$EXPECTED_COUNT" ] 2>/dev/null; then
    echo -e "${GREEN}Test 5: total COUNT matches expected ($count_num)${NC}"
    SCORE=$((SCORE+1))
  else
    echo -e "${RED}Test 5: expected $EXPECTED_COUNT, got $count_num${NC}"
  fi
fi

# ---------------- Test 6: time.* artifact in test/ ----------------
if found="$(find_time_artifact)"; then
  file_path="${found%%|*}"
  ext_found="${found##*|}"
  echo "Found: $file_path"
  echo -e "${GREEN}Test 6: time.${ext_found} present in test folder${NC}"
  SCORE=$((SCORE+1))
else
  echo -e "${RED}Test 6: missing time.{txt|xls|csv|png|jpg} in test folder${NC}"
  echo -e "${YEL}Hint: include execution time results as 'test/time.ext' (e.g., test/time.txt)${NC}"
fi

# ---------------- Final score ----------------
echo "-------------------"
echo "Tests Passed: $SCORE / $TOTAL_TESTS"
FINAL=$(( (SCORE * 100) / TOTAL_TESTS ))
if [ "$FINAL" -ge 70 ]; then COLOR=$GREEN; else COLOR=$RED; fi
echo -e "Final Grade: ${COLOR}${FINAL}%${NC}"
