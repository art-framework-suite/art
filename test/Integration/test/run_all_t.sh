#!/bin/sh


# Pass in name and status
function die { echo $1: status $2 ;  exit $2; }

${FW_HOME}/FWCore/Integration/test/refTest.sh || die 'Failed in refTest.sh' $?

${FW_HOME}/FWCore/Integration/test/transRefTest.sh || die 'Failed in transRefTest.sh' $?

${FW_HOME}/FWCore/Integration/test/inputSourceTest.sh || die 'Failed in inputSourceTest.sh' $?

${FW_HOME}/FWCore/Integration/test/inputExtSourceTest.sh || die 'Failed in inputExtSourceTest.sh' $?

${FW_HOME}/FWCore/Integration/test/eventSetupTest.sh || die 'Failed in eventSetupTest.sh' $?

${FW_HOME}/FWCore/Integration/test/hierarchy_example.sh || die 'Failed in hierarchy_example.sh' $?

${FW_HOME}/FWCore/Integration/test/service_example.sh || die 'Failed in service_example.sh' $?

