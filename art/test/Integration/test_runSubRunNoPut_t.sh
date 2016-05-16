#!/bin/bash

# Process 1
art --rethrow-all -c runSubRunNoPut_t.fcl -o out1.root -n 1

# Process 2
touch empty.fcl
art --rethrow-all -c empty.fcl -o out.root -s out1.root -s out1.root -n 2 || exit 1

rm out1.root
