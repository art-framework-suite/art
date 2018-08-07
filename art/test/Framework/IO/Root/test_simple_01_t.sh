#!/bin/bash

. cet_test_functions.sh

rm -f out.root cerr.log warnings.log

run_art test_simple_01.fcl --rethrow-all

check_files "out.root"
check_files "tfile_output.root"

#root -l -b -q test_simple_01_verify.cxx || exit 1
echo "Running test_simple_01_verify ..."
test_simple_01_verify || exit 1
echo "Finished test_simple_01_verify"


run_art test_simple_01r.fcl --rethrow-all
