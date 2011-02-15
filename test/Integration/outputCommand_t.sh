#!/bin/bash

. cet_test_functions.sh

rm -f out.root cerr.log warnings.log

run_art outputCommand_w.fcl

check_files "out.root"

run_art outputCommand_r.fcl
