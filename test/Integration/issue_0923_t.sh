#!/bin/bash

ART_DEBUG_CONFIG= nova -c issue_0923b.fcl >issue_0923_out.txt 2>&1
status=$?

(( $status == 1 )) || { echo "Unexpected exit code $status from exec invocation." 1>&2; exit 1; }

diff -u issue_0923_ref.txt issue_0923_out.txt

exit $?
