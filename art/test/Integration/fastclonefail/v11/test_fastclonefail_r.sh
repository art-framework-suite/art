#!/bin/bash

. cet_test_functions.sh

rm -f copy.root

export LD_LIBRARY_PATH=..:${LD_LIBRARY_PATH}
export DYLD_LIBRARY_PATH=..:${DYLD_LIBRARY_PATH}

art -c "${1}" -s "${2}" --rethrow-all

check_files "copy.root"

