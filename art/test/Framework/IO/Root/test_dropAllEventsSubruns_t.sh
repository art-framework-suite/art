#!/bin/bash

. cet_test_functions.sh

rm -f out*.root 

run_art dropAllEvents_t.fcl --rethrow-all
run_art dropAllEventsSubruns_t1.fcl --rethrow-all
run_art dropAllEventsSubruns_t2.fcl --rethrow-all

check_files "out_dropAllEvents.root"
check_files "out_dropAllEventsSubruns1.root"
check_files "out_dropAllEventsSubruns2.root"

#root -l -b -q test_dropAllEventsSubruns_verify.cxx || exit 1
echo "Running test_dropAllEventsSubruns_verify ..."
test_dropAllEventsSubruns_verify || exit 1
echo "Finished test_dropAllEventsSubruns_verify ..."

run_art dropAllEvents_r_t.fcl --rethrow-all
run_art dropAllEventsSubruns_r_t1.fcl --rethrow-all
run_art dropAllEventsSubruns_r_t2.fcl --rethrow-all

check_files "out_dropAllEvents_r.root"
check_files "out_dropAllEventsSubruns1_r.root"
check_files "out_dropAllEventsSubruns2_r.root"

#root -l -b -q "test_dropAllEventsSubruns_verify.cxx(\"r\")" || exit 1
echo "Running test_dropAllEventsSubruns_verify r ..."
test_dropAllEventsSubruns_verify r || exit 1
echo "Finished test_dropAllEventsSubruns_verify r ..."
