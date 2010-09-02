#!/bin/sh

function die { echo $1: status $2 ;  exit $2; }

fw --parameter-set ${FW_HOME}/FWCore/Integration/test/hierarchy_example_cfg.py || die 'Failed in hierarchy_example_cfg.py' $?

