#!/bin/bash

# --- Configuration & Cleanup ---
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'
SCORE=0
LOG_FILE="test_output.log"
SHM_NAME="/mem_block_exam_1"
MQ_NAME="/mq_exam_1"

rm -f  data_processor $LOG_FILE

# --- Compilation ---
echo "--- Compiling Programs ---"
# gcc -Wall -Werror ../data_creator/src/main.c -o data_creator -lrt
# if [ $? -ne 0 ]; then echo -e "${RED}Creator failed to compile.${NC}"; exit 1; fi
gcc -Wall -Werror ../data_processor/src/main.c -o data_processor -lrt
if [ $? -ne 0 ]; then echo -e "${RED}Processor failed to compile.${NC}"; exit 1; fi
# gcc -Wall -Werror ../data_logger/src/main.c -o data_logger -lrt
# if [ $? -ne 0 ]; then echo -e "${RED}Logger failed to compile.${NC}"; exit 1; fi
echo -e "${GREEN}Program compiled successfully.${NC}\n"

# --- Run the Main Process and Capture All Output ---
echo "--- Running Data Processor and Capturing Output ---"
# We run the processor in the background and capture all of its output.
# The children's output will be part of this stream.
./data_processor > $LOG_FILE 2>&1 &
PROCESSOR_PID=$!

# Give the processes a moment to start and create IPC objects.
sleep 1

# --- Test 1: Process Creation 10 pts) ---
echo "--- Test 1: Process Creation (10 pts) ---"
creator_pid=$(pgrep -x "data_creator")
logger_pid=$(pgrep -x "data_logger")

if [ -n "$creator_pid" ] && [ -n "$logger_pid" ]; then
    echo -e "${GREEN}PASS: data_creator and data_logger were created.${NC}"
    SCORE=$(echo "$SCORE + 10" | bc)
else
    echo -e "${RED}FAIL: Not all child processes were created.${NC}"
fi
echo ""

# --- Test 2: IPC Object Creation (11 pts) ---
echo "--- Test 2: IPC Object Creation (11 pts) ---"
mq_created=false
shm_created=false

if [ -e "/dev/mqueue/$MQ_NAME" ]; then mq_created=true; fi
if [ -e "/dev/shm$SHM_NAME" ]; then shm_created=true; fi

if $mq_created && $shm_created; then
    echo -e "${GREEN}PASS: MQ and SHM were created in the system.${NC}"
    SCORE=$(echo "$SCORE + 11" | bc)
else
    echo -e "${RED}FAIL: Not all IPC objects were created.${NC}"
fi
echo ""

# --- Test 3: Signals Received (12 pts) ---
echo "--- Test 3: Signal Reception (12 pts) ---"
# We'll wait for the data processing to finish.
wait $PROCESSOR_PID
# Count the number of "new data available" messages from data_processor.
# This indicates the signal was received
signal_count=$(grep -c "Data Processor: new data available" $LOG_FILE)

if [ "$signal_count" -eq 5 ]; then
    echo -e "${GREEN}PASS: Data processor received 5 signals and processed data.${NC}"
    SCORE=$(echo "$SCORE + 12" | bc)
else
    echo -e "${RED}FAIL: Expected 5 signals, but found $signal_count.${NC}"
fi
echo ""

# --- Test 4: Correct Average Sent (18 pts) ---
echo "--- Test 4: Correct Average Sent (18 pts) ---"
# The logger's output should be in test_output.log.
# We'll check for the exact calculated averages.
expected_averages=("5.500000" "15.500000" "25.500000" "35.500000" "45.500000")
pass_count=0
for avg in "${expected_averages[@]}"; do
    if grep -q "Received average: $avg" $LOG_FILE; then
        pass_count=$((pass_count + 1))
    fi
done

if [ "$pass_count" -eq 5 ]; then
    echo -e "${GREEN}PASS: All 5 averages were sent and received correctly.${NC}"
    SCORE=$(echo "$SCORE + 18" | bc)
else
    echo -e "${RED}FAIL: Not all averages were found in the logger's output. Found $pass_count out of 5.${NC}"
fi
echo ""

# --- Test 5: IPC Unlinking (9 pts) ---
echo "--- Test 5: IPC Unlinking (9 pts) ---"
mq_unlinked=true
shm_unlinked=true

if [ -e "/dev/mqueue/$MQ_NAME" ]; then mq_unlinked=false; fi
if [ -e "/dev/shm$SHM_NAME" ]; then shm_unlinked=false; fi

if $mq_unlinked && $shm_unlinked && $mq_created && $shm_created; then
    echo -e "${GREEN}PASS: MQ and SHM were successfully unlinked.${NC}"
    SCORE=$(echo "$SCORE + 9" | bc)
else
    if ! $mq_created || ! $shm_created; then
        echo -e "${RED}FAIL: IPC objects were not created, so unlinking test is invalid.${NC}"
    else
        echo -e "${RED}FAIL: Not all IPC objects were unlinked.${NC}"
    fi
fi
echo ""

# --- Final Score ---
echo "-----------------------------------"
echo -e "Final Score: ${GREEN}$SCORE${NC} out of 60"
echo "-----------------------------------"

# --- Final Cleanup ---
# rm -f data_creator data_processor data_logger $LOG_FILE