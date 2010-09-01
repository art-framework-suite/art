#!/bin/sh

# Pass in name and status
function die { echo $1: status $2 ;  exit $2; }

pushd ${LOCAL_TMP_DIR}

fw ${FW_HOME}/FWCore/Integration/test/CatchStdExceptiontest_cfg.py &> CatchStdException.log && die 'Failed in using CatchStdException_cfg.py' $?

grep -q WhatsItESProducer CatchStdException.log || die 'Failed to find Producers name' $?
#grep -w ESProducer CatcheStdException.log

popd

