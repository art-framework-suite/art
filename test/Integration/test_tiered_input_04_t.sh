#!/bin/bash

. cet_test_functions.sh
rm -f f11.root f12.root f13.root
rm -f f21.root f22.root f23.root
rm -f f31.root f32.root f33.root
rm -f f41.root f42.root f43.root
rm -f out1.root out1_nofastcloning.root
rm -f cerr.log warnings.log
run_art test_tiered_input_04_make_f11.fcl --rethrow-all
check_files "f11.root"
run_art test_tiered_input_04_make_f12.fcl --rethrow-all
check_files "f12.root"
run_art test_tiered_input_04_make_f13.fcl --rethrow-all
check_files "f13.root"
run_art test_tiered_input_04_make_f21.fcl --rethrow-all
check_files "f21.root"
run_art test_tiered_input_04_make_f22.fcl --rethrow-all
check_files "f22.root"
run_art test_tiered_input_04_make_f23.fcl --rethrow-all
check_files "f23.root"
run_art test_tiered_input_04_make_f31.fcl --rethrow-all
check_files "f31.root"
run_art test_tiered_input_04_make_f32.fcl --rethrow-all
check_files "f32.root"
run_art test_tiered_input_04_make_f33.fcl --rethrow-all
check_files "f33.root"
run_art test_tiered_input_04_make_f41.fcl --rethrow-all
check_files "f41.root"
run_art test_tiered_input_04_make_f42.fcl --rethrow-all
check_files "f42.root"
run_art test_tiered_input_04_make_f43.fcl --rethrow-all
check_files "f43.root"
run_art test_tiered_input_04_read.fcl --rethrow-all
run_art test_tiered_input_04_copy.fcl --rethrow-all
check_files "out1.root"
run_art test_tiered_input_04_copy_nofastcloning.fcl --rethrow-all
check_files "out1_nofastcloning.root"
run_art test_tiered_input_04_read_out1_nofastcloning.fcl --rethrow-all

exit 0
