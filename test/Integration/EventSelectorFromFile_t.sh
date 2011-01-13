#!/bin/bash

function run_art() {
    local config="$1"
    art --config "$config"
    local status=$?
    (( ${status:-1} == 0 )) || \
        { echo "art failed on config \"${config}\" (status code $status)." 1>&2; \
        exit 1; }
}

rm -f out1.root out2.root cerr.log warnings.log

run_art EventSelectorFromFile_w.fcl

{ [[ -r "out1.root" ]] && [[ -r "out2.root" ]]; } || \
  { echo "Failed to find expected root files out1.root and out2.root." 1>&2; \
    exit 1; }

run_art EventSelectorFromFile_r1.fcl

run_art EventSelectorFromFile_r2.fcl
