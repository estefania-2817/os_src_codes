l#!/bin/bash

echo "🧪 TICKETS SALES TEST"
echo "============================="

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color
OS_NAME=$(uname -s)

TOTAL_TESTS=0
PASSED_TESTS=0

# Compile
compile_ok=true
gcc -I../ticket_office/include ../ticket_office/src/main.c ../ticket_office/src/functions.c -o ticket_office || compile_ok=false
gcc -I../buyer/include ../buyer/src/main.c ../buyer/src/functions.c -o buyer || compile_ok=false

if [ "$compile_ok" = false ]; then
    echo "❌ Compilation failed. Skipping tests."
    TOTAL_TESTS=0
    PASSED_TESTS=0
    PERCENTAGE=0
    echo ""
    echo "⚙️ FINAL RESULT:"
    echo "==================="
    echo -e "${RED} Tests passed: $PASSED_TESTS/$TOTAL_TESTS"
    echo -e "${RED} Percentage: $PERCENTAGE%"
    echo "❌ DEFICIENT - Build failed"
    exit 2
fi

ipcs -m | awk '/16/ {print $2}' | xargs -I {} ipcrm -m {} 2>/dev/null


mkdir -p logs

echo "Running ticket_office..."
./ticket_office > logs/ticket_office.log 2>&1 &
TO_PID=$!
echo "Running buyer..."
./buyer > logs/buyer.log 2>&1 &
BUYER_PID=$!
sleep 1

echo "🚀 Running tests..."

# Test 1: 3Threads was created
echo -e "${YELLOW}1. VERIFY THREAD CREATION${NC}"
THREAD_TEST_PASSED=false

case "$OS_NAME" in
    Darwin)
        echo  "   OS: $OS_NAME..."
        if nm buyer | grep -q "pthread_create"; then
            echo "   ✅ PASSED - buyer links against pthread_create $OS_NAME"
            PASSED_TESTS=$((PASSED_TESTS + 1))
            THREAD_TEST_PASSED=true
        else
            echo "   ❌ FAILED - buyer binary missing pthread_create symbol $OS_NAME"
        fi
        ;;
    Linux)
        echo  "   OS: $OS_NAME..."
        THREADS_CREATED=""
        WORKER_THREADS=0
        if [ -n "$BUYER_PID" ] && ps -p "$BUYER_PID" > /dev/null 2>&1; then
            if [ -d "/proc/$BUYER_PID/task" ]; then
                THREADS_CREATED=$(ls -1 "/proc/$BUYER_PID/task" 2>/dev/null | wc -l | awk '{print $1}')
            elif ps -o nlwp= -p "$BUYER_PID" > /dev/null 2>&1; then
                THREADS_CREATED=$(ps -o nlwp= -p "$BUYER_PID" 2>/dev/null | tr -d ' ')
            fi
        fi

        if [[ "$THREADS_CREATED" =~ ^[0-9]+$ ]]; then
            WORKER_THREADS=$((THREADS_CREATED - 1))
        fi

        if [ "$WORKER_THREADS" -eq 3 ]; then
            echo "   ✅ PASSED - buyer spawned 3 buyer threads $OS_NAME"
            PASSED_TESTS=$((PASSED_TESTS + 1))
            THREAD_TEST_PASSED=true
        else
            echo "   ❌ FAILED - expected 3 worker threads, found $WORKER_THREADS"
        fi
        ;;
    *)
        if nm buyer | grep -q "pthread_create"; then
            echo "   ✅ PASSED2222 - pthread_create symbol present $OS_NAME"
            PASSED_TESTS=$((PASSED_TESTS + 1))
            THREAD_TEST_PASSED=true
        else
            echo "   ❌ FAILED - could not verify thread creation on $OS_NAME"
        fi
        ;;
esac
TOTAL_TESTS=$((TOTAL_TESTS + 1))


# Test 2: If requests 3, should buy 3
echo -e "${YELLOW}2. Mutex validation${NC}"
if nm buyer | grep -q "pthread_mutex_lock"; then
    echo "   ✅ PASSED - buyer create mutex"
    PASSED_TESTS=$((PASSED_TESTS + 1))
else
    echo "   ❌ FAILED - buyer missing pthread create/mutex"
fi
TOTAL_TESTS=$((TOTAL_TESTS + 1))


wait $BUYER_PID 2>/dev/null
wait $TO_PID 2>/dev/null

# Test 3: Validate sold out...
LAST_LINES=$(tail -6 ./logs/ticket_office.log)
echo -e "${YELLOW}3. Validate report result${NC}"
# printf '   <<%q>>\n' "$LAST_LINES"
if echo "$LAST_LINES" | grep -A4 "TICKET REPORT:" | grep -q "SOLD OUT..."; then
    echo "   ✅ PASSED - Correct report results"
    PASSED_TESTS=$((PASSED_TESTS + 1))
else
    echo "   ❌ FAILED - No match in last output"
    echo "   SOLD OUT..."
fi
TOTAL_TESTS=$((TOTAL_TESTS + 1))

# Test 4: Verify shared memory was released
echo -e "${YELLOW}4. Shared memory released${NC}"
if ipcs -m | grep -q "0x00000010"; then
    echo "   ❌ FAILED - Shared memory segment still present"
else
    echo "   ✅ PASSED - Shared memory segment released"
    PASSED_TESTS=$((PASSED_TESTS + 1))
fi
TOTAL_TESTS=$((TOTAL_TESTS + 1))



# Calculate percentage
if [ $TOTAL_TESTS -gt 0 ]; then
    PERCENTAGE=$((PASSED_TESTS * 100 / TOTAL_TESTS))
else
    PERCENTAGE=0
fi

echo ""
echo "⚙️ FINAL RESULT:"
echo "==================="
# Clean up
# rm -f buyer logs/output.log logs/ticket_office.log ticket_office

# Final evaluation
if [ $PERCENTAGE -ge 98 ]; then
    echo -e "${GREEN} Tests passed: $PASSED_TESTS/$TOTAL_TESTS"
    echo -e "${GREEN} Percentage: $PERCENTAGE%"
    echo "🎉 EXCELLENT! Business rules work correctly"
    exit 0
elif [ $PERCENTAGE -ge 80 ]; then
    echo -e "${YELLOW} Tests passed: $PASSED_TESTS/$TOTAL_TESTS"
    echo -e "${YELLOW} Percentage: $PERCENTAGE%"
    echo "⚠️  REGULAR - Some rules need adjustments"
    exit 1
else
    echo -e "${RED} Tests passed: $PASSED_TESTS/$TOTAL_TESTS"
    echo -e "${RED} Percentage: $PERCENTAGE%"
    echo "❌ DEFICIENT - Most rules failed"
    exit 2
fi
