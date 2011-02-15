#!/bin/bash

. cet_test_functions.sh

rm -f cerr.log warnings.log

fail_art issue_0940.fcl

# Re-source tests but change behavior on fail
. cet_test_functions.sh -r

check_fail fgrep Assertion stderr.log || \
  { echo "art appears to have failed on an assertion." 1>&2;
    exit 1; }
