#!/bin/bash

. cet_test_functions.sh
rm -f f11.root f12.root f13.root f2.root f3.root f4.root cerr.log warnings.log
run_art test_tiered_input_03_make_f11.fcl --rethrow-all
check_files "f11.root"
run_art test_tiered_input_03_make_f12.fcl --rethrow-all
check_files "f12.root"
run_art test_tiered_input_03_make_f13.fcl --rethrow-all
check_files "f13.root"
run_art test_tiered_input_03_make_f2.fcl --rethrow-all
check_files "f2.root"
run_art test_tiered_input_03_make_f3.fcl --rethrow-all
check_files "f3.root"
run_art test_tiered_input_03_make_f4.fcl --rethrow-all
check_files "f4.root"
run_art test_tiered_input_03_read.fcl --rethrow-all

exit 0
