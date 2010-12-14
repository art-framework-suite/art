#!/bin/sh

function die { echo $1: status $2 ;  exit $2; }

fw --parameter-set ${FW_HOME}/FWCore/Integration/test/detsetvector_t_cfg.py || die 'Failed in detsetvector_t_cfg.py' $?

