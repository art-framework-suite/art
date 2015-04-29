#!/bin/bash

. cet_test_functions.sh
rm -f out1.root out2.root out3.root cerr.log warnings.log
run_art test_ptrvector_01c.fcl --rethrow-all
check_files "out1.root"
run_art test_ptrvector_01d.fcl --rethrow-all
check_files "out2.root"
run_art test_ptrvector_01e.fcl --rethrow-all
check_files "out3.root"

