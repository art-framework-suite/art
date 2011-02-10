#!/bin/bash

function run_art_and_fail() {
  local config="$1"
  echo "Invoking art --config \"$config\""
  art --config "$config" > stdout.log 2> stderr.log
  local status=$?
  (( ${status:-1} != 0 )) || \
      { echo "expected art failure on config \"${config}\" did not happen (status code $status)." 1>&2;
      exit 1; }
}

rm -f cerr.log warnings.log

run_art_and_fail issue_0940.fcl
fgrep Assertion stderr.log
status=$?
(( ${status} != 0 )) ||
    { echo "art appears to have failed on an assertion." 1>&2;
    exit 1; }
