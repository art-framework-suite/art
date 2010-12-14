#!/bin/bash

test=testRunMerge

function die { echo Failure $1: status $2 ; exit $2 ; }

pushd ${LOCAL_TMP_DIR}
  echo ${test}PROD1 ------------------------------------------------------------
  fw -p ${FW_HOME}/FWCore/Integration/test/${test}PROD1_cfg.py || die "fw ${test}PROD1_cfg.py" $?

  echo ${test}PROD2------------------------------------------------------------
  fw -p ${FW_HOME}/FWCore/Integration/test/${test}PROD2_cfg.py || die "fw ${test}PROD2_cfg.py" $?

  echo ${test}PROD3------------------------------------------------------------
  fw -p ${FW_HOME}/FWCore/Integration/test/${test}PROD3_cfg.py || die "fw ${test}PROD3_cfg.py" $?

  echo ${test}PROD4------------------------------------------------------------
  fw -p ${FW_HOME}/FWCore/Integration/test/${test}PROD4_cfg.py || die "fw ${test}PROD4_cfg.py" $?

  echo ${test}PROD5------------------------------------------------------------
  fw -p ${FW_HOME}/FWCore/Integration/test/${test}PROD5_cfg.py || die "fw ${test}PROD5_cfg.py" $?

  echo ${test}MERGE------------------------------------------------------------
  fw -p ${FW_HOME}/FWCore/Integration/test/${test}MERGE_cfg.py || die "fw ${test}MERGE_cfg.py" $?

  echo ${test}MERGE1------------------------------------------------------------
  fw -p ${FW_HOME}/FWCore/Integration/test/${test}MERGE1_cfg.py || die "fw ${test}MERGE1_cfg.py" $?

  echo ${test}TEST------------------------------------------------------------
  fw -p ${FW_HOME}/FWCore/Integration/test/${test}TEST_cfg.py || die "fw ${test}TEST_cfg.py" $?

  echo ${test}TEST1------------------------------------------------------------
  fw -p ${FW_HOME}/FWCore/Integration/test/${test}TEST1_cfg.py || die "fw ${test}TEST1_cfg.py" $?

  echo ${test}COPY------------------------------------------------------------
  fw -p ${FW_HOME}/FWCore/Integration/test/${test}COPY_cfg.py || die "fw ${test}COPY_cfg.py" $?

  echo ${test}COPY1------------------------------------------------------------
  fw -p ${FW_HOME}/FWCore/Integration/test/${test}COPY1_cfg.py || die "fw ${test}COPY1_cfg.py" $?

  echo ${test}PickEvents------------------------------------------------------------
  fw -p ${FW_HOME}/FWCore/Integration/test/${test}PickEvents_cfg.py || die "fw ${test}PickEvents_cfg.py" $?

popd

exit 0
