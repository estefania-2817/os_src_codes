#!/bin/bash

# --- Configuration ---
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
RED='\033[0;31m'
NC='\033[0m'
EXECUTABLE="LAB4_L1"
OUTPUT_FILE="test_output.log"
TEST_LOG="scheduling_test_results.csv"



# --- Compilation (original program) ---
echo "--- Compiling the program ---"
gcc -Wall -Werror -I../include ../src/main.c ../src/functions.c -o $EXECUTABLE
if [ $? -ne 0 ]; then
    echo -e "${RED}Compilation failed. Exiting.${NC}"
    exit 1
fi
echo -e "${GREEN}Compilation successful!${NC}"
echo "-------------------"

# Define Test Matrix: [NICE_VALUE] [CPU_CORE]
TEST_CONFIGS=(
    "1 0"      # High priority, core 0
    "1 0"      # High priority, core 0
    "10 0"     # Medium priority, core 0 (Should be slower than '0 0' under load)
    "15 0"     # Low priority, core 0 (Should be slower than '10 0' under load)
    "19 0"     # Low priority, core 0 (Should be slower than '10 0' under load)
    "1 2"      # High priority, core 2
    "1 2"      # High priority, core 2
    "10 2"     # Medium priority, core 2 (Should be slower than '0 2' under load)
    "15 2"     # Low priority, core 2 (Should be slower than '10 2' under load)
    "19 2"     # Low priority, core 2 (Should be slower than '10 2' under load)
)

# --- Helper Functions ---

# Function to check the process state (PSR/NI) using its PID
# Arguments: $1=PID, $2=Expected CPU, $3=Expected Nice
validate_os_state() {
    local pid=$1
    local expected_cpu=$2
    local expected_nice=$3
    local actual_cpu
    local actual_nice

    # Reset global flags (1 = PASS, 0 = FAIL)
    NICE_OK=0
    CPU_OK=0

    # Protect against empty/invalid PID
    if [[ -z "$pid" || ! "$pid" =~ ^[0-9]+$ ]]; then
        actual_nice="N/A"
        actual_cpu="N/A"
    else
        # Use ps with -p (PID) and -o fields; trim whitespace and take first non-empty line.
        actual_nice=$(ps -p "$pid" -o ni= 2>/dev/null | awk '{$1=$1;print}' | head -n1)
        actual_cpu=$(ps -p "$pid" -o psr= 2>/dev/null | awk '{$1=$1;print}' | head -n1)

        # If ps failed or returned nothing, mark as N/A
        if [[ -z "$actual_nice" ]]; then actual_nice="N/A"; fi
        if [[ -z "$actual_cpu" ]]; then actual_cpu="N/A"; fi
    fi

    # --- Validation & set flags ---
    if [ "$actual_nice" == "$expected_nice" ]; then
        nice_check="${GREEN}NI OK${NC}"
        NICE_OK=1
    else
        nice_check="${RED}NI FAIL (Exp:$expected_nice, Got:$actual_nice)${NC}"
        NICE_OK=0
    fi

    if [ "$actual_cpu" == "$expected_cpu" ]; then
        cpu_check="${GREEN}CPU OK${NC}"
        CPU_OK=1
    else
        cpu_check="${YELLOW}CPU OK (Affinity set, but running on $actual_cpu)${NC}"
        CPU_OK=0
    fi

    echo -e "  [OS Check] PID $pid: $nice_check | $cpu_check"
}
# ...existing code...

# --- Main Test Execution ---

echo -e "--- Starting Thread Scheduling Tests ---\n"

# Initialize log file
echo "Nice_Input,CPU_Input,Time_Sec,Checksum,OS_Nice_Check,OS_CPU_Check,Points" > $TEST_LOG

# Initialize scoring
SCORE=0
TOTAL_TESTS=${#TEST_CONFIGS[@]}
TOTAL_POINTS=$(( TOTAL_TESTS * 10 ))  # 10 points per experiment (3 CPU + 3 NICE + 4 LOG)

for config in "${TEST_CONFIGS[@]}"; do
    #kills previos execuitions
    pkill -f $EXECUTABLE

    IFS=' ' read -r NICE_INPUT CPU_INPUT <<< "$config"

    echo -e "\n[EXECUTING] Config: NICE=$NICE_INPUT, CPU=$CPU_INPUT..."

    # --- Run the program and capture its output ---
    "./$EXECUTABLE" $NICE_INPUT $CPU_INPUT > $OUTPUT_FILE 2>&1 &
    # Give the OS a moment to spawn the process before querying for its PID
    sleep 0.4
    # Derive process name from the EXECUTABLE and find its PID using pgrep.
    PROC_NAME=$(basename "$EXECUTABLE")
    PID=$(pgrep -x "$PROC_NAME" | head -n 1)

    echo "  [Info] Running PID: $PID"

    # --- Validation 1: OS State Check ---
    # Validate the nice and affinity settings in the OS while the process is running
    NICE_OK=0
    CPU_OK=0
    validate_os_state $PID $CPU_INPUT $NICE_INPUT

    wait $PID

    # --- Validation 2: Log Output Check ---
    LOG_LINE=$(grep "^LOG|" "$OUTPUT_FILE")

    echo "  [Info] Captured LOG line: $LOG_LINE"
    
    LOG_OK=0
    if [ -n "$LOG_LINE" ]; then
        # Parse the structured log output
        TIME=$(echo "$LOG_LINE" | awk -F'|' '{print $4}' | awk -F'=' '{print $2}')
        FINAL_CPU=$(echo "$LOG_LINE" | awk -F'|' '{print $3}' | awk -F'=' '{print $2}')
        CHECKSUM=$(echo "$LOG_LINE" | awk -F'|' '{print $5}' | awk -F'=' '{print $2}')
        
        echo -e "  [Result] Time: ${YELLOW}${TIME}s${NC}, Final CPU: $FINAL_CPU"
        LOG_OK=1
    else
        echo -e "${RED}ERROR: Failed to find final LOG output line.${NC}"
        TIME="0.000000"
        CHECKSUM="N/A"
        FINAL_CPU="N/A"
        LOG_OK=0
    fi

    # --- Scoring: 3 points CPU, 3 points NICE, 4 points LOG ---
    per_test_score=$(( CPU_OK * 3 + NICE_OK * 3 + LOG_OK * 4 ))
    SCORE=$(( SCORE + per_test_score ))

    OS_NICE_STR=$( [ "$NICE_OK" -eq 1 ] && echo "PASS" || echo "FAIL" )
    OS_CPU_STR=$( [ "$CPU_OK" -eq 1 ] && echo "PASS" || echo "FAIL" )

    # Log the result to CSV (include earned points)
    echo "$NICE_INPUT,$CPU_INPUT,$TIME,$CHECKSUM,$OS_NICE_STR,$OS_CPU_STR,$per_test_score" >> $TEST_LOG

done

echo -e "\n--- Test Summary ---\n"
echo "Results logged to $TEST_LOG"
cat $TEST_LOG
echo -e "\n----------------------------------------------------------------------"
echo -e "Total Score: ${GREEN}$SCORE${NC} out of ${TOTAL_POINTS}"
echo -e "----------------------------------------------------------------------"