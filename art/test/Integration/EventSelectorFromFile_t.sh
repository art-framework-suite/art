#!/bin/bash

. cet_test_functions.sh

rm -f out1.root out2.root cerr.log warnings.log

run_art EventSelectorFromFile_w.fcl --rethrow-all

check_files "out1.root" "out2.root"

run_art EventSelectorFromFile_r1.fcl --rethrow-all

run_art EventSelectorFromFile_r2.fcl --rethrow-all
