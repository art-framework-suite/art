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

run_art ptr_list_01.fcl

