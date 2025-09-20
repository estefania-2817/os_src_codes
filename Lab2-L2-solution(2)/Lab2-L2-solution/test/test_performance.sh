#!/usr/bin/env bash
# test_performance.sh
# Evaluates:
# 1) Compiles the program
# 2) Director/Actor lines exist (processes created)
# 3) Participation order by roles (not exact text): D,A,D,A,D (collapsed by changes)
# 4) Actor's Hamlet line: content match ignoring punctuation/quotes (word-based)
# 5) Clean termination (exit 0 and Actor not left alive)

set -u

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
NC='\033[0m'

SCORE=0
TOTAL_TESTS=4   # (1) created, (2) participation order, (3) Hamlet content, (4) termination

# Paths (run from excercise22/test)
SRC_DIR="../src"
INC_FLAG="-I ../include"
BIN="./theater"
TIMEOUT_SEC=30

# ------------- Compilation -------------
if [[ ! -f "$SRC_DIR/main.c" || ! -f "$SRC_DIR/functions.c" ]]; then
  echo -e "${RED}main.c and/or functions.c not found in $SRC_DIR${NC}"
  exit 1
fi

echo -e "${YELLOW}--- Compiling the program ---${NC}"
set -o pipefail
if ! gcc -Wall -Werror -O2 $INC_FLAG "$SRC_DIR/main.c" "$SRC_DIR/functions.c" -o "$BIN" 2>&1 | sed 's/^/gcc: /'; then
  echo -e "${RED}Compilation failed.${NC}"
  exit 1
fi
set +o pipefail
echo -e "${GREEN}Compilation successful!${NC}"
echo "----------------------------------------"

# ------------- Run (capture output) -------------
RUN_BIN="$BIN"

echo -e "${YELLOW}--- Running program (capturing output) ---${NC}"

# Enforce line-buffering if stdbuf exists (prevents reordering due to stdio buffering)
if command -v stdbuf >/dev/null 2>&1; then
  RUN_BIN="stdbuf -oL -eL $BIN"
fi
RUN_CMD="$RUN_BIN"
if command -v timeout >/dev/null 2>&1; then
  RUN_CMD="timeout ${TIMEOUT_SEC}s $RUN_BIN"
fi


output="$($RUN_CMD 2>&1)"
exit_code=$?
echo "$output"
echo "----------------------------------------"

# ------------- Helpers -------------
first_line_index() {
  local rx="$1"
  local ln
  ln="$(printf "%s\n" "$output" | grep -n -m1 -E "$rx" | cut -d: -f1)"
  [[ -z "${ln:-}" ]] && echo 0 || echo "$ln"
}

# Build the FULL role sequence from speaker lines (preserve repeats), e.g. D,A,D,A,A,A,D,D,D
role_seq_full="$(
  printf "%s\n" "$output" \
  | grep -E '^(Director|Actor) \(PID:' \
  | sed -E 's/^Director \(PID:.*/D/; s/^Actor \(PID:.*/A/' \
  | paste -sd "," -
)"

# ------------- Test 1: processes created -------------
idx_dir_first=$(first_line_index "^Director \(PID:")
idx_act_first=$(first_line_index "^Actor \(PID:")
if [[ $idx_dir_first -gt 0 && $idx_act_first -gt 0 ]]; then
  echo -e "${GREEN}Test 1 (processes created): PASSED${NC}"
  ((SCORE++))
else
  echo -e "${RED}Test 1 (processes created): FAILED${NC}"
  [[ $idx_dir_first -eq 0 ]] && echo -e "${RED}  - No line starting with 'Director (PID:' was found.${NC}"
  [[ $idx_act_first -eq 0 ]] && echo -e "${RED}  - No line starting with 'Actor (PID:' was found.${NC}"
fi

# ------------- Test 2: exact participation order (roles, not text) -------------
expected_seq="D,A,D,A,A,A,D,D"
if [[ "$role_seq_full" == "$expected_seq" ]]; then
  echo -e "${GREEN}Test 2 (exact participation order ${expected_seq}): PASSED${NC}"
  ((SCORE++))
else
  echo -e "${RED}Test 2 (exact participation order ${expected_seq}): FAILED${NC}"
  echo -e "${RED}  - Detected sequence: ${role_seq_full:-<empty>}${NC}"
fi

# ------------- Test 3: Hamlet (word-based content; ignore punctuation/quotes; keep spaces) -------------
# Normalize: drop \r, lowercase, map ’/' to space, replace non-alnum with space,
# trim, and squeeze spaces to a single one. Result: words separated by one space.
normalize_words() {
  tr -d '\r' \
  | tr '[:upper:]' '[:lower:]' \
  | sed "s/[’']/ /g" \
  | awk '
    {
      gsub(/[^a-z0-9]+/, " ", $0);   # keep only words and spaces
      gsub(/^[ \t]+|[ \t]+$/, "", $0);
      gsub(/[ \t]+/, " ", $0);
      print
    }'
}

hamlet_plain="To be, or not to be, that is the question: Whether 'tis nobler in the mind to suffer the slings and arrows of outrageous fortune, or to take arms against a sea of troubles, and by opposing end them."
hamlet_norm="$(printf '%s' "$hamlet_plain" | normalize_words)"

hamlet_ok=false
best_sample=""

# Traverse Actor lines; extract text after "): " and compare by CONTENT (substring) after normalization.
while IFS= read -r line; do
  if echo "$line" | grep -q -E "^Actor \(PID: [0-9]+\): "; then
    text="${line#*: }"
    text_norm="$(printf '%s' "$text" | normalize_words)"
    best_sample="$text_norm"
    if printf '%s' "$text_norm" | grep -F -q -- "$hamlet_norm"; then
      hamlet_ok=true
      break
    fi
  fi
done < <(printf "%s\n" "$output")

if $hamlet_ok; then
  echo -e "${GREEN}Test 3 (Hamlet line content): PASSED${NC}"
  ((SCORE++))
else
  echo -e "${RED}Test 3 (Hamlet line content): FAILED${NC}"
  echo -e "${RED}  - No Actor line whose content (ignoring punctuation/quotes) contains the expected verse was found.${NC}"
  echo "      Expected (words): $hamlet_norm"
  if [[ -n "$best_sample" ]]; then
    echo "      First Actor candidate (normalized): $best_sample"
  fi
fi

# ------------- Test 4: clean termination -------------
actor_pid_line="$(printf "%s\n" "$output" | grep -m1 -E "^Actor \(PID: [0-9]+\):" || true)"
actor_pid=""
if [[ -n "$actor_pid_line" ]]; then
  actor_pid="$(printf "%s" "$actor_pid_line" | sed -E 's/.*Actor \(PID: ([0-9]+)\).*/\1/')"
fi

terminated_ok=true
[[ $exit_code -ne 0 ]] && terminated_ok=false
if [[ -n "$actor_pid" ]]; then
  if kill -0 "$actor_pid" 2>/dev/null; then
    terminated_ok=false
    echo -e "${RED}  - Actor PID $actor_pid is still alive after program exit (or PID was reused).${NC}"
  fi
fi

if $terminated_ok; then
  echo -e "${GREEN}Test 4 (clean termination): PASSED${NC}"
  ((SCORE++))
else
  echo -e "${RED}Test 4 (clean termination): FAILED${NC}"
  [[ $exit_code -ne 0 ]] && echo -e "${RED}  - Program exit code: $exit_code (expected 0).${NC}"
fi

# ------------- Summary -------------
echo "----------------------------------------"
echo -e "Tests Passed: ${SCORE} / ${TOTAL_TESTS}"
grade=$(( SCORE * 100 / TOTAL_TESTS ))
if [[ $SCORE -eq $TOTAL_TESTS ]]; then
  echo -e "Final Grade: ${GREEN}${grade}%${NC}"
else
  echo -e "Final Grade: ${YELLOW}${grade}%${NC}"
fi

# Cleanup
rm -f "$BIN"
