#!/bin/bash
# If successful this will dump core so disable that... 
ulimit -c 0

F1=${FW_HOME}/FWCore/Services/test/fpe_test_cfg.py

echo "***"
echo "If the test is successful, fw will fail (abort) with error status 139 or 11."
echo "The purpose is to test that floating point exceptions cause failures."
echo "If the floating point exception does not cause the fw job to"
echo "abort, an explicit exception will thrown from CMS code that also causes"
echo "an abort, but this time with error status 65."
echo "The values 139, 11, and 65 depend on things underneath that CMS does not"
echo "control.  These values have changed before and may change again. If"
echo "they do, someone will need to investigate and change the criteria in"
echo "this shell script (fpe_test.sh)."
echo "***"


fw $F1 >& /dev/null
status=$?

echo "Completed fw $F1"
echo "fw status: " $status
if [ $status -ne 139 -a $status -ne 11 ] ; then
 echo "Test FAILED, status not expected value"
 exit 1
fi
echo "Test SUCCEEDED"
