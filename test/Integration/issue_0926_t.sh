#!/bin/bash

. cet_test_functions.sh

rm -f issue_0926*.root cerr.log warnings.log

run_art issue_0926a.fcl --rethrow-all

check_files issue_0926a.root

run_art issue_0926b.fcl --rethrow-all

check_files issue_0926b.root

cp issue_0926b.root issue_0926b1.root
cp issue_0926b.root issue_0926b2.root
cp issue_0926b.root issue_0926b3.root

run_art issue_0926c.fcl --rethrow-all
