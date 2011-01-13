#!/bin/bash

function run_art() {
    local config="$1"
    art --config "$config"
    local status=$?
    (( ${status:-1} == 0 )) || \
        { echo "art failed on config \"${config}\" (status code $status)." 1>&2; \
        exit 1; }
}

rm -f out.root cerr.log warnings.log

run_art test_simple_01.fcl

[[ -r "out.root" ]] || \
  { echo "Failed to find expected root files out.root" 1>&2; \
    exit 1; }

run_art test_simple_01r.fcl
