#!/bin/bash

. cet_test_functions.sh
rm -f f1.root f2.root f3.root f4.root cerr.log warnings.log
run_art test_tiered_input_02_make_f1.fcl --rethrow-all
check_files "f1.root"
run_art test_tiered_input_02_make_f2.fcl --rethrow-all
check_files "f2.root"
run_art test_tiered_input_02_make_f3.fcl --rethrow-all
check_files "f3.root"
run_art test_tiered_input_02_make_f4.fcl --rethrow-all
check_files "f4.root"
run_art test_tiered_input_02_read.fcl --rethrow-all

exit 0
