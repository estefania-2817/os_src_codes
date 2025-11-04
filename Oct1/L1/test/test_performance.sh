# #!/bin/bash
# # Evaluates the "horse race" program without reading/validating any stdout text.
# # Project layout:
# #   build/      -> output binary goes here
# #   src/main.c  -> student source
# #   test/       -> this script

# set -euo pipefail

# # ---------------- Config ----------------
# SRC="../src/main.c"
# BUILD_DIR="../build"
# BIN="$BUILD_DIR/horse_race"
# TIMEOUT_SEC=10

# TOTAL_TESTS=4
# SCORE=0

# OUT="horse_output.txt"
# ERR="horse_stderr.txt"

# GREEN='\033[0;32m'; RED='\033[0;31m'; YEL='\033[0;33m'; NC='\033[0m'

# # -------------- Helpers --------------
# cleanup() { rm -f "$OUT" "$ERR"; }
# trap cleanup EXIT

# run_with_timeout() {
#   local t="$1"; shift
#   set +e
#   ( "$@" >"$OUT" 2>"$ERR" ) &
#   local pid=$!
#   local elapsed=0
#   while kill -0 "$pid" 2>/dev/null; do
#     sleep 1
#     elapsed=$((elapsed+1))
#     if [ "$elapsed" -ge "$t" ]; then
#       kill -TERM "$pid" 2>/dev/null || true
#       sleep 1
#       kill -KILL "$pid" 2>/dev/null || true
#       wait "$pid" 2>/dev/null || true
#       echo -e "${RED}Timeout reached (${t}s)${NC}"
#       return 124
#     fi
#   done
#   wait "$pid"
#   local rc=$?
#   set -e
#   return "$rc"
# }

# # symbol lookup that works with nm or readelf
# bin_has_symbol() {
#   local sym="$1"
#   # Try nm (undefined & dynamic); fall back to readelf
#   if command -v nm >/dev/null 2>&1; then
#     if nm -u "$BIN" 2>/dev/null | grep -Eiq "\b${sym}\b"; then return 0; fi
#     if nm -D "$BIN" 2>/dev/null | grep -Eiq "\b${sym}\b"; then return 0; fi
#   fi
#   if command -v readelf >/dev/null 2>&1; then
#     if readelf -Ws "$BIN" 2>/dev/null | grep -Eiq "\b${sym}\b"; then return 0; fi
#   fi
#   return 1
# }

# # Count threads of a process (Linux/macOS)
# thread_count() {
#   local _pid="$1"
#   case "$(uname -s)" in
#     Linux)
#       if [ -d "/proc/${_pid}/task" ]; then
#         ls -1 "/proc/${_pid}/task" 2>/dev/null | wc -l | tr -d ' '
#       else
#         echo 0
#       fi
#       ;;
#     Darwin)
#       ps -M "$_pid" 2>/dev/null | tail -n +2 | wc -l | tr -d ' '
#       ;;
#     *)
#       echo 0
#       ;;
#   esac
# }

# # -------------- SETUP (not graded) --------------
# echo "--- Setup: Building ---"
# mkdir -p "$BUILD_DIR"
# if ! gcc -O2 -Wall -Wextra -std=c11 -pthread "$SRC" -o "$BIN"; then
#   echo -e "${RED}Build failed${NC}"
#   exit 1
# fi
# echo -e "${GREEN}Build OK${NC}"
# echo "------------------------"

# echo "--- Setup: Running once (timeout ${TIMEOUT_SEC}s) ---"
# if ! run_with_timeout "$TIMEOUT_SEC" "$BIN"; then
#   echo -e "${RED}Execution failed or timed out${NC}"
#   exit 1
# fi
# echo -e "${GREEN}Execution OK${NC}"
# echo "-----------------------------------------------"

# # -------------- Test 1: Uses pthread_create (binary symbol check) --------------
# if bin_has_symbol "pthread_create"; then
#   echo -e "${GREEN}Test 1 OK: pthread_create found in binary${NC}"
#   SCORE=$((SCORE+1))
# else
#   echo -e "${RED}Test 1 FAIL: pthread_create symbol not found in binary${NC}"
# fi
# echo "-------------------"

# # -------------- Test 2: Uses pthread_join (binary symbol check) --------------
# if bin_has_symbol "pthread_join"; then
#   echo -e "${GREEN}Test 2 OK: pthread_join found in binary${NC}"
#   SCORE=$((SCORE+1))
# else
#   echo -e "${RED}Test 2 FAIL: pthread_join symbol not found in binary${NC}"
# fi
# echo "-------------------"

# # -------------- Test 3: Runtime -> more than one thread observed --------------
# # Run again only for sampling; do NOT read/require stdout.
# ( "$BIN" >/dev/null 2>/dev/null ) &
# pid=$!

# max_threads_seen=0
# # Sample ~3s every 20ms (150 samples)
# for ((i=0; i<150; i++)); do
#   kill -0 "$pid" 2>/dev/null || break
#   nlwp=$(thread_count "$pid"); [[ "$nlwp" =~ ^[0-9]+$ ]] || nlwp=0
#   (( nlwp > max_threads_seen )) && max_threads_seen="$nlwp"
#   # portable short sleep; bash supports decimals on most systems
#   sleep 0.02
# done

# # stop sampler if still running
# kill -TERM "$pid" 2>/dev/null || true
# sleep 0.1
# kill -KILL "$pid" 2>/dev/null || true
# wait "$pid" 2>/dev/null || true

# if [ "$max_threads_seen" -ge 2 ]; then
#   echo -e "${GREEN}Test 3 OK: concurrent threads observed (max=${max_threads_seen})${NC}"
#   SCORE=$((SCORE+1))
# else
#   echo -e "${RED}Test 3 FAIL: could not observe >1 thread (max=${max_threads_seen})${NC}"
#   echo -e "${YEL}Hint: if your program finishes too fast, increase DISTANCE or sleep duration for grading.${NC}"
# fi
# echo "-------------------"

# # -------------- Test 4: No forbidden synchronization primitives --------------
# # Reject classic sync APIs (exercise is "no sync"): mutex/cond/semaphore/barrier/futex.
# FORBIDDEN_RE='pthread_mutex_|pthread_cond_|pthread_barrier_|\bsem_(init|wait|post|timedwait|trywait)\b|futex'
# if ( nm -u "$BIN" 2>/dev/null; nm -D "$BIN" 2>/dev/null; readelf -Ws "$BIN" 2>/dev/null ) \
#    | grep -Eiq "$FORBIDDEN_RE"; then
#   echo -e "${RED}Test 4 FAIL: forbidden synchronization symbol(s) detected${NC}"
# else
#   echo -e "${GREEN}Test 4 OK: no forbidden synchronization symbols detected${NC}"
#   SCORE=$((SCORE+1))
# fi

# # -------------- Final score --------------
# echo "-------------------"
# echo "Tests Passed: $SCORE / $TOTAL_TESTS"
# FINAL=$(( (SCORE * 100) / TOTAL_TESTS ))
# if [ "$FINAL" -ge 70 ]; then COLOR=$GREEN; else COLOR=$RED; fi
# echo -e "Final Grade: ${COLOR}${FINAL}%${NC}"

# # Debug tips (disabled):
# # nm -u "$BIN" | sort
# # readelf -Ws "$BIN" | grep -Ei 'pthread|sem|futex'

#!/bin/bash
# Evaluates the "horse race" program without reading/validating any stdout text.
# Project layout:
#   build/      -> output binary goes here
#   src/main.c  -> student source
#   test/       -> this script

set -euo pipefail

# ---------------- Config ----------------
SRC="../src/main.c"
BUILD_DIR="../build"
BIN="$BUILD_DIR/horse_race"
TIMEOUT_SEC=10

TOTAL_TESTS=3
SCORE=0

GREEN='\033[0;32m'; RED='\033[0;31m'; YEL='\033[0;33m'; NC='\033[0m'

# -------------- Helpers --------------
run_with_timeout() {
  local t="$1"; shift
  set +e
  ( "$@" >/dev/null 2>/dev/null ) &
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
  # Prefer nm; fall back to readelf; as last resort, strings (looser)
  if command -v nm >/dev/null 2>&1; then
    nm -u "$BIN" 2>/dev/null | grep -Eiq "\b${sym}\b" && return 0
    nm -D "$BIN" 2>/dev/null | grep -Eiq "\b${sym}\b" && return 0
  fi
  if command -v readelf >/dev/null 2>&1; then
    readelf -Ws "$BIN" 2>/dev/null | grep -Eiq "\b${sym}\b" && return 0
  fi
  if command -v strings >/dev/null 2>&1; then
    strings "$BIN" 2>/dev/null | grep -Eiq "\b${sym}\b" && return 0
  fi
  return 1
}

thread_count() {
  local _pid="$1"
  case "$(uname -s)" in
    Linux)
      [ -d "/proc/${_pid}/task" ] && ls -1 "/proc/${_pid}/task" 2>/dev/null | wc -l | tr -d ' ' || echo 0
      ;;
    Darwin)
      ps -M "$_pid" 2>/dev/null | tail -n +2 | wc -l | tr -d ' '
      ;;
    *)
      echo 0
      ;;
  esac
}

# -------------- SETUP (not graded) --------------
echo "--- Setup: Building ---"
mkdir -p "$BUILD_DIR"
if ! gcc -O2 -Wall -Wextra -std=c11 -pthread "$SRC" -o "$BIN"; then
  echo -e "${RED}Build failed${NC}"; exit 1
fi
echo -e "${GREEN}Build OK${NC}"
echo "------------------------"

echo "--- Setup: (timeout ${TIMEOUT_SEC}s) ---"
if ! run_with_timeout "$TIMEOUT_SEC" "$BIN"; then
  echo -e "${RED}Execution failed or timed out${NC}"; exit 1
fi
echo -e "${GREEN}Execution OK${NC}"
echo "-----------------------------------------------"

# -------------- Test 1: Thread creation confirmed (symbol + runtime) --------------
sym_ok=0
bin_has_symbol "pthread_create" && sym_ok=1

# run again only for sampling; do NOT rely on stdout
( "$BIN" >/dev/null 2>/dev/null ) &
pid=$!

max_threads_seen=0
# Sample ~2s, every 10ms (200 samples) to catch short-lived threads
for ((i=0; i<200; i++)); do
  kill -0 "$pid" 2>/dev/null || break
  nlwp=$(thread_count "$pid"); [[ "$nlwp" =~ ^[0-9]+$ ]] || nlwp=0
  (( nlwp > max_threads_seen )) && max_threads_seen="$nlwp"
  sleep 0.01
done
# stop sampler if still running
kill -TERM "$pid" 2>/dev/null || true
sleep 0.1
kill -KILL "$pid" 2>/dev/null || true
wait "$pid" 2>/dev/null || true

rt_ok=0
[ "$max_threads_seen" -ge 2 ] && rt_ok=1   # main + at least 1 created

if [ "$sym_ok" -eq 1 ] && [ "$rt_ok" -eq 1 ]; then
  echo -e "${GREEN}Test 1 OK: pthread_create found${NC}"
  SCORE=$((SCORE+1))
else
  if [ "$sym_ok" -ne 1 ]; then
    echo -e "${RED}Test 1 FAIL: pthread_create not found${NC}"
  fi
  if [ "$rt_ok" -ne 1 ]; then
    echo -e "${RED}Test 1 FAIL: could not observe ≥2 threads at runtime (max=${max_threads_seen})${NC}"
  fi
fi
echo "-------------------"

# -------------- Test 2: pthread_join used --------------
if bin_has_symbol "pthread_join"; then
  echo -e "${GREEN}Test 2 OK: pthread_join found${NC}"
  SCORE=$((SCORE+1))
else
  echo -e "${RED}Test 2 FAIL: pthread_join symbol not found${NC}"
fi
echo "-------------------"

# -------------- Test 3: No forbidden synchronization primitives --------------
# Disallow classic sync primitives for this exercise
FORBIDDEN_RE='pthread_mutex_|pthread_cond_|pthread_barrier_|\bsem_(init|wait|post|timedwait|trywait)\b|futex'
if ( command -v nm >/dev/null 2>&1 && { nm -u "$BIN"; nm -D "$BIN"; } 2>/dev/null; command -v readelf >/dev/null 2>&1 && readelf -Ws "$BIN" 2>/dev/null ) \
   | grep -Eiq "$FORBIDDEN_RE"; then
  echo -e "${RED}Test 3 FAIL: forbidden synchronization symbol(s) detected${NC}"
else
  echo -e "${GREEN}Test 3 OK: no forbidden synchronization symbols detected${NC}"
  SCORE=$((SCORE+1))
fi

# -------------- Final score --------------
echo "-------------------"
echo "Tests Passed: $SCORE / $TOTAL_TESTS"
FINAL=$(( (SCORE * 100) / TOTAL_TESTS ))
if [ "$FINAL" -ge 70 ]; then COLOR=$GREEN; else COLOR=$RED; fi
echo -e "Final Grade: ${COLOR}${FINAL}%${NC}"
