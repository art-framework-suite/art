#!/bin/bash

test=testSeriesOfProcesses

function die { echo Failure $1: status $2 ; exit $2 ; }

pushd ${LOCAL_TMP_DIR}

  fw -p ${FW_HOME}/FWCore/Integration/test/${test}HLT_cfg.py || die "fw ${test}HLT_cfg.py" $?

  fw -p ${FW_HOME}/FWCore/Integration/test/${test}PROD_cfg.py || die "fw ${test}PROD_cfg.py" $?

  fw -p ${FW_HOME}/FWCore/Integration/test/${test}TEST_cfg.py || die "fw ${test}TEST_cfg.py" $?

  fw -p ${FW_HOME}/FWCore/Integration/test/${test}TEST1_cfg.py || die "fw ${test}TEST1_cfg.py" $?

  fw -p ${FW_HOME}/FWCore/Integration/test/${test}TEST2_cfg.py 2> ${test}TEST2.txt
  grep "Duplicate Process" ${test}TEST2.txt || die "fw ${test}TEST2_cfg.py" $?

  fw -p ${FW_HOME}/FWCore/Integration/test/${test}TEST3_cfg.py || die "Failure in history testing in ${test}" $?

  fw -p ${FW_HOME}/FWCore/Integration/test/${test}PROD2TEST_cfg.py || die "fw ${test}PROD2TEST_cfg.py" $?

  fw -p ${FW_HOME}/FWCore/Integration/test/${test}PROD2TEST_unscheduled_cfg.py || die "fw ${test}PROD2TEST_cfg.py" $?

popd

exit 0
