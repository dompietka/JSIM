#!/bin/bash
g++ @opcjeCpp main.cpp prev.cpp -o main.e

while getopts "vmg" flag; do
 case $flag in
   h) # Handle the -h flag
   echo "./manager [-v] [-m]"
   echo "-m : check for memory leaks"
   echo "-u : read from unhashed tests"
   ;;
   m) # Handle memory leak flag
    VALGRIND=1
   ;;
   u) # Handle unhashing flag
    UNHASHED=1
   ;;
   \?)
   # Handle invalid options
   ;;
 esac
done


# Directory to iterate over
TARGET_DIR="in"

# Check if the directory exists
if [ ! -d "$TARGET_DIR" ]; then
  echo "Directory $TARGET_DIR does not exist."
  exit 1
fi


# Iterate over all files in the directory
for FILE in "$TARGET_DIR"/*; do
  if [ -f "$FILE" ]; then
    BASENAME="${FILE%.in}"
	BASENAME="${BASENAME#in/}"
	OUT_FILE="out/$BASENAME.out"
	UNHASHED_FILE="unhashed/$BASENAME.unhashed"
	echo "Testing : $BASENAME"

	if [ "$VALGRIND" == 1 ]; then
		if [ "$UNHASHED" == 1 ];then
			valgrind --log-file=valgrind.rpt --leak-check=full --error-exitcode=1 ./main.e < "$UNHASHED_FILE" > solution_out
		else
			valgrind --log-file=valgrind.rpt --leak-check=full --error-exitcode=1 ./main.e < "$FILE" > solution_out
		fi
        if [ $? != 0 ]; then
            echo "$i Memory leak! "
            cat valgrind.rpt
            break
        fi
    else
		if [ "$UNHASHED" == 1 ]; then
			./main.e < "$UNHASHED_FILE" > solution_out
		else
			./main.e < "$FILE" > solution_out
		fi
    fi

	if diff -b solution_out "$OUT_FILE"
	then
		echo -ne "OK\n"
	else
		echo -ne "ERR\n"
		break
	fi
  fi
done
echo "Testing zmax_max_tests0.in with time:"
if [ "$UNHASHED" == 1 ]; then
	time ./main.e < unhashed/zmax_max_tests0.unhashed > solution_out
else
	time ./main.e < in/zmax_max_tests0.in > solution_out
fi
