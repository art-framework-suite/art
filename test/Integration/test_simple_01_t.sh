#!/bin/bash

. cet_test_functions.sh

rm -f out.root cerr.log warnings.log

run_art test_simple_01.fcl

check_files "out.root"

run_art test_simple_01r.fcl
