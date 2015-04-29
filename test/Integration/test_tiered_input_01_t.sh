#!/bin/bash

. cet_test_functions.sh
rm -f m1.root m2.root m3.root m4.root m5.root cerr.log warnings.log
run_art test_tiered_input_01_make_m1.fcl --rethrow-all
check_files "m1.root"
run_art test_tiered_input_01_read_m1.fcl --rethrow-all
run_art test_tiered_input_01_make_m2.fcl --rethrow-all
check_files "m2.root"
run_art test_tiered_input_01_read_m2.fcl --rethrow-all
run_art test_tiered_input_01_make_m3.fcl --rethrow-all
check_files "m3.root"
run_art test_tiered_input_01_read_both.fcl --rethrow-all
run_art test_tiered_input_01_copy_both.fcl --rethrow-all
check_files "m4.root"
run_art test_tiered_input_01_copy_both_no_fastclone.fcl --rethrow-all
check_files "m5.root"

exit 0
