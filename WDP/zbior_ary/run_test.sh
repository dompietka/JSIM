#!/bin/bash
make
if [[ $? != "0" ]]; then
    exit 1
fi

CHECK="CHECK"
if [[ $1 == "-c" ]]; then
    CHECK="NOCHECK"
fi
./tester.e CUSTOM $CHECK >output.txt

echo "OUTPUTS: "
cat output.txt
