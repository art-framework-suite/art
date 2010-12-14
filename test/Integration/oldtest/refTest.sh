#!/bin/sh

function die { echo $1: status $2 ;  exit $2; }

fw --parameter-set ${FW_HOME}/FWCore/Integration/test/RefTest_cfg.py || die 'Failed in RefTest_cfg.py' $?

# Pass in name and status



