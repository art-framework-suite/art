#!/bin/bash

. cet_test_functions.sh

rm -f out.root cerr.log warnings.log

run_art test_ptrvector_01a.fcl --rethrow-all

check_files "out.root"

run_art test_ptrvector_01b.fcl --rethrow-all
