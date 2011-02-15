#!/bin/bash

. cet_test_functions.sh

rm -f issue_0926*.root cerr.log warnings.log

run_art issue_0926a.fcl

check_files issue_0926a.root

run_art issue_0926b.fcl

check_files issue_0926b.root

run_art issue_0926c.fcl
