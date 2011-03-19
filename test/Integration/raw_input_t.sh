#!/bin/bash

. cet_test_functions.sh

rm -f out.root cerr.log warnings.log

run_art raw_input_01.fcl

