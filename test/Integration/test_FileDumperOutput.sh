#!/bin/bash

. cet_test_functions.sh

rm -f out.root cerr.log warnings.log

run_art FileDumperOutputTest_w.fcl

run_art FileDumperOutputTest_r.fcl
