#!/bin/bash

EXEC_DIR=${1:-.}

export TMP_DIR=`mktemp -d /tmp/${0##*/}.XXXXXXXXXX`

trap "[[ -d \"$TMP_DIR\" ]] && rm -rf \"$TMP_DIR\"" EXIT

LD_LIBRARY_PATH="$LD_LIBRARY_PATH:lib2:lib1"

printenv | grep BOOST | sort

export BOOST_TEST_SHOW_PROGRESS=yes
export BOOST_TEST_LOG_LEVEL=message

${EXEC_DIR}/$(basename "${0}" .sh)

exit $?
