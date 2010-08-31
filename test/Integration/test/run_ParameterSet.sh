#!/bin/bash

test=testParameterSet

function die { echo Failure $1: status $2 ; exit $2 ; }

pushd ${LOCAL_TMP_DIR}

  echo ${test} ------------------------------------------------------------
  fw -p ${FW_HOME}/FWCore/Integration/test/${test}_cfg.py 2> ${test}.txt
  grep "Illegal parameter" ${test}.txt || die "fw ${test}_cfg.py" $?

popd

exit 0
