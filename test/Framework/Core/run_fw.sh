#!/bin/bash

# Pass in name and status
function die { echo $1: status $2 ;  exit $2; }

(fw --help ) || die 'Failure running fw --help' $?


