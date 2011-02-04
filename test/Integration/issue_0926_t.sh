#!/bin/bash

function check_files() {
  (( result = 0 ))
  for file in "$@"; do
    [[ -r "$file" ]] || \
        { echo "Failed to find expected file \"$file\"" 1>&2;
          (( ++result )); }
  done
  if (( $result == 0 )); then
    return
  else
    echo "Failed to find $result files." 1>&2
    exit 1
  fi
}

function run_art() {
  local config="$1"
  echo "Invoking art --config \"$config\""
  art --config "$config"
  local status=$?
  (( ${status:-1} == 0 )) || \
      { echo "art failed on config \"${config}\" (status code $status)." 1>&2; \
      exit 1; }
}

rm -f issue_0926*.root cerr.log warnings.log

run_art issue_0926a.fcl

check_files issue_0926a.root

run_art issue_0926b.fcl

check_files issue_0926b.root

run_art issue_0926c.fcl
