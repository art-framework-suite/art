#!/bin/bash

. cet_test_functions.sh

rm -f out.root

export LD_LIBRARY_PATH=..${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}
export DYLD_LIBRARY_PATH=..${DYLD_LIBRARY_PATH:+:${DYLD_LIBRARY_PATH}}
export CET_PLUGIN_PATH=..${CET_PLUGIN_PATH:+:${CET_PLUGIN_PATH}}

printenv LD_LIBRARY_PATH

art -c "${1}" --rethrow-all

check_files "out.root"

