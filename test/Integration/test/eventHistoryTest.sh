#!/bin/sh

# Pass in name and status
function die { echo $1: status $2 ;  exit $2; }

pushd ${LOCAL_TMP_DIR}

# Write a file for the FIRST process
fw --parameter-set ${FW_HOME}/FWCore/Integration/test/EventHistory_1_cfg.py || die 'Failed in EventHistory_1' $?
echo "*************************************************"
echo "**************** Finished pass 1 ****************"
echo "*************************************************"

# Read the first file, write the second.
fw --parameter-set ${FW_HOME}/FWCore/Integration/test/EventHistory_2_cfg.py || die 'Failed in EventHistory_2' $?
echo "*************************************************"
echo "**************** Finished pass 2 ****************"
echo "*************************************************"

# Read the second file, write the third.
fw --parameter-set ${FW_HOME}/FWCore/Integration/test/EventHistory_3_cfg.py || die 'Failed in EventHistory_3' $?
echo "*************************************************"
echo "**************** Finished pass 3 ****************"
echo "*************************************************"

# Read the third file, make sure the event data have the right history
fw --parameter-set ${FW_HOME}/FWCore/Integration/test/EventHistory_4_cfg.py || die 'Failed in EventHistory_4' $?
echo "*************************************************"
echo "**************** Finished pass 4 ****************"
echo "*************************************************"

popd
