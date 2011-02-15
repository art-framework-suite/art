#!/bin/bash

. cet_test_functions.sh

rm -f out.root cerr.log warnings.log

run_art test_view_01a.fcl

check_files "out.root"

run_art test_view_01b.fcl
