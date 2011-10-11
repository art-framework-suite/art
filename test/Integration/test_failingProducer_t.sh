#!/bin/bash

# Import test functions.
. cet_test_functions.sh

# Use nova instead of art.
export ART_EXEC=nova

rm -f out.root cerr.log warnings.log

run_art test_failingProducer_w.fcl --rethrow-all

check_files "out.root"

rm -f OutCloned.root cerr.log warnings.log

run_art test_failingProducer_r.fcl

check_files "outCloned.root"

rm -f OutCloned.root cerr.log warnings.log

run_art test_failingProducer_r.fcl -n 250

check_files "outCloned.root"
