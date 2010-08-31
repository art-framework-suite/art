#!/bin/sh

function die { echo $1: status $2 ;  exit $2; }

fw --parameter-set ${FW_HOME}/FWCore/Integration/test/EventSetupTest_cfg.py || die 'Failed in EventSetupTest_cfg.py' $?
fw --parameter-set ${FW_HOME}/FWCore/Integration/test/EventSetupAppendLabelTest_cfg.py || die 'Failed in EventSetupAppendLabelTest_cfg.py' $?
fw --parameter-set ${FW_HOME}/FWCore/Integration/test/EventSetupTest2_cfg.py || die 'Failed in EventSetupTest2_cfg.py' $?
fw --parameter-set ${FW_HOME}/FWCore/Integration/test/EventSetupTest2_cfg.py || die 'Failed in EventSetupAppendLabelTest2_cfg.py' $?
