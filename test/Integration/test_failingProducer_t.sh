#!/bin/bash

function run_nova() {
    local config="$1"
    shift
    nova --config "$config" "$@"
    local status=$?
    (( ${status:-1} == 0 )) || \
        { echo "nova failed on config \"${config}\" (status code $status)." 1>&2; \
        exit 1; }
}

rm -f out.root cerr.log warnings.log

run_nova test_failingProducer_w.fcl

[[ -r "out.root" ]] || \
  { echo "Failed to find expected root files out.root" 1>&2; \
    exit 1; }

rm -f OutCloned.root cerr.lg warnings.log

run_nova test_failingProducer_r.fcl

[[ -r "outCloned.root" ]] || \
  { echo "Failed to find expected root files outCloned.root (test 1)" 1>&2; \
    exit 1; }

rm -f OutCloned.root cerr.lg warnings.log

run_nova test_failingProducer_r.fcl -n 250

[[ -r "outCloned.root" ]] || \
  { echo "Failed to find expected root files outCloned.root (test 2)" 1>&2; \
    exit 1; }
