Change files `zbior_ary.h` and `zbior_ary.c` to yours solution and then just run `./test.sh`.

To run only edge cases run `./test.sh -e`.

To run a specific amount of integration tests run `./test.sh -i` (keep in mind that the test are deterministic, so if you run it again it will test on the exact same input).

To run stress tests (some large tests) to test the speed of your program run `./test.sh -s`.

If a test fails it is saved to the file `test.txt` (the format is pretty intuitive - first two numbers represent the number of operations and q, then every operation adds a new zbior_ary to a vector and operations SUM, DIFFERENCE and INTERSECTION are followed by indices of the vector on which the operation is applied).

The correct output followed by your output is saved to `output.txt`.

There is also a script to run a custom test (the format is the same as in `test.txt`), example usage:
`./run_test.sh < test.txt`
