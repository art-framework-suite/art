#!/bin/sh

function die { echo $1: status $2 ;  exit $2; }

fw --parameter-set ${FW_HOME}/FWCore/Integration/test/ViewTest_cfg.py || die 'Failed in ViewTest_cfg.py' $?
