#!/bin/bash

. cet_test_functions.sh -r

rm -f out1.root out2.root cerr.log warnings.log

run_art EventSelectorFromFile_w.fcl

check_files "out.root" "out2.root"

run_art EventSelectorFromFile_r1.fcl

run_art EventSelectorFromFile_r2.fcl
