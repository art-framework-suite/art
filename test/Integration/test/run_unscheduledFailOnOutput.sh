#!/bin/bash

test=unscheduled_fail_on_output_
CFG_DIR=${FW_HOME}/FWCore/Integration/test/../python/test

function die { echo Failure $1: status $2 ; exit $2 ; }

pushd ${LOCAL_TMP_DIR}

  fw ${CFG_DIR}/${test}Rethrow_cfg.py && die "fw ${test}Rethrow_cfg.py did not fail" $?

  fw ${CFG_DIR}/${test}IgnoreCompletely_cfg.py || die "fw ${test}IgnoreCompletely_cfg.py" $?
  fw ${CFG_DIR}/${test}read_found_events.py || die "fw ${test}read_found_events.py" $?

  fw -p ${CFG_DIR}/${test}FailModule_cfg.py || die "fw ${test}FailModule_cfg.py" $?
  fw ${CFG_DIR}/${test}read_found_events.py || die "fw ${test}read_found_events.py" $?

  fw -p ${CFG_DIR}/${test}FailPath_cfg.py || die "fw ${test}FailPath_cfg.py" $?
  #NOTE: Following fails because of an assert, plus we don't know what behavior we actually want in this case
  fw ${CFG_DIR}/${test}read_no_events.py || die "fw ${test}read_no_events.py" $?

  fw -p ${CFG_DIR}/${test}SkipEvent_cfg.py || die "fw ${test}SkipEvent_cfg.py" $?
  #NOTE: Following fails because of an assert, plus we don't know what behavior we actually want in this case
  fw ${CFG_DIR}/${test}read_no_events.py || die "fw ${test}read_no_events.py" $?

popd

exit 0
