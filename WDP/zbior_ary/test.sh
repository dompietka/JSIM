#!/bin/bash
make
if [[ $? != "0" ]]; then
    exit 1
fi

BOLD=$(tput bold)
NORMAL=$(tput sgr0)
RED="\e[31m"
GREEN="\e[32m"
BLUE="\e[36m"

CURRENT_TEST=0

function run_tests {
    TEST_NUM=$1
    TEST_TYPE=$2
    TEST_FUNCTION=$3
    MAX_Q=$4
    INPUT_SIZE=$5
    POSITIVE=$6

    for i in $(seq 1 $TEST_NUM); do
        if ./tester.e $TEST_TYPE $TEST_FUNCTION SIZE $MAX_Q $INPUT_SIZE $POSITIVE $((i + CURRENT_TEST)) 2>test.txt 1>output.txt; then
            echo -ne "$BLUE - SIZE - $NORMAL $i$GREEN OK! $NORMAL\r"
        else
            echo -e "$BLUE - SIZE - $NORMAL $i$RED NIE OK! $NORMAL"
            exit 1
        fi
    done
    CURRENT_TEST=$((CURRENT_TEST + TEST_NUM))
    echo ""
    for i in $(seq 1 $TEST_NUM); do
        if ./tester.e $TEST_TYPE $TEST_FUNCTION ARY $MAX_Q $INPUT_SIZE $POSITIVE $((i + CURRENT_TEST)) 2>test.txt 1>output.txt; then
            echo -ne "$BLUE - ARY - $NORMAL $i$GREEN OK! $NORMAL\r"
        else
            echo -e "$BLUE - ARY - $NORMAL $i$RED NIE OK! $NORMAL"
            exit 1
        fi
    done
    CURRENT_TEST=$((CURRENT_TEST + TEST_NUM))
    echo ""
    for i in $(seq 1 $TEST_NUM); do
        if ./tester.e $TEST_TYPE $TEST_FUNCTION CONTAINS $MAX_Q $INPUT_SIZE $POSITIVE $((i + CURRENT_TEST)) 2>test.txt 1>output.txt; then
            echo -ne "$BLUE - CONTAINS - $NORMAL $i$GREEN OK! $NORMAL\r"
        else
            echo -e "$BLUE - CONTAINS - $NORMAL$RED NIE OK! $NORMAL"
            exit 1
        fi
    done
    CURRENT_TEST=$((CURRENT_TEST + TEST_NUM))
    echo ""
}

function run_set_of_tests {
    INFO=$1
    TEST_TYPE=$2
    TEST_FUNCTION=$3

    echo "$BOLD -- $INFO - SMALL POSITIVE -- $NORMAL"
    run_tests 100 "$TEST_TYPE" "$TEST_FUNCTION" 5 10 1
    echo "$BOLD -- $INFO - SMALL -- $NORMAL"
    run_tests 1000 "$TEST_TYPE" "$TEST_FUNCTION" 5 10 0
    echo "$BOLD -- $INFO - MEDIUM -- $NORMAL"
    run_tests 100 "$TEST_TYPE" "$TEST_FUNCTION" 20 100 0
    echo "$BOLD -- $INFO - LARGE -- $NORMAL"
    run_tests 100 "$TEST_TYPE" "$TEST_FUNCTION" 100 1000 0
}

function test_edgecases {
    echo "$BOLD -- EDGE CASES -- $NORMAL"
    for infile in "./edge_cases/"*.in; do
        base="${infile%.in}"
        ./tester.e CUSTOM NOCHECK <$base.in >output.txt
        echo -n "TEST $base RUN"
        if [[ -e "$base.out" ]]; then
            if diff -b $base.out output.txt >/dev/null; then
                echo -ne " :$GREEN OK! $NORMAL"
            else
                echo -ne " :$RED NIE OK! $NORMAL"
            fi
        fi
        echo ""
    done
}

if [[ $1 == "-i" ]]; then
    echo -n "Enter number of small integration tests to run: "
    read INTEGRATION_TESTS_NUM
    run_tests $INTEGRATION_TESTS_NUM "INTEGRATION" "ALL" 5 10 0
elif [[ $1 == "-s" ]]; then
    echo "$BOLD -- STRESS TESTS -- $NORMAL"
    rtime="$(
        TIMEFORMAT='%lR'
        time (run_tests 10 "STRESS" "ALL" 100 20000000 0) 2>&1 1>/dev/null
    )"

    echo "YOUR TIME: $rtime (expected: ~3s)"
elif [[ $1 == "-e" ]]; then
    test_edgecases
else
    run_set_of_tests "CONSTRUCTOR TESTS" "CONSTRUCTOR" "X"

    run_set_of_tests "UNIT TESTS - SUM" "UNIT" "SUM"
    run_set_of_tests "UNIT TESTS - DIFFERENCE" "UNIT" "DIFFERENCE"
    run_set_of_tests "UNIT TESTS - INTERSECTION" "UNIT" "INTERSECTION"

    run_set_of_tests "INTEGRATION TESTS - SUM" "INTEGRATION" "SUM"
    run_set_of_tests "INTEGRATION TESTS - DIFFERENCE" "INTEGRATION" "DIFFERENCE"
    run_set_of_tests "INTEGRATION TESTS - INTERSECTION" "INTEGRATION" "INTERSECTION"
    run_set_of_tests "INTEGRATION TESTS" "INTEGRATION" "ALL"

    echo "$BOLD -- INTEGRATION TESTS - VERY LARGE -- $NORMAL"
    run_tests 10 "INTEGRATION" "ALL" 1000 50000 0

    test_edgecases
fi

echo -e "$BOLD SUCCESSFULLY FINISHED!!! $NORMAL"
