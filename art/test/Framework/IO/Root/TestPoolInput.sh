#!/bin/sh
# Pass in name and status
function die { echo $1: status $2 ;  exit $2; }

fw --parameter-set ${FW_HOME}/IORoot/Input/test/PreRootInputTest_cfg.py || die 'Failure using PreRootInputTest_cfg.py' $?

cp RootInputTest.root RootInputOther.root

fw --parameter-set ${FW_HOME}/IORoot/Input/test/RootInputTest_cfg.py || die 'Failure using RootInputTest_cfg.py' $?

fw --parameter-set ${FW_HOME}/IORoot/Input/test/PreRootInputTest2_cfg.py || die 'Failure using PreRootInputTest2_cfg.py' $?

cp RootInputTest.root RootInputOther.root

fw --parameter-set ${FW_HOME}/IORoot/Input/test/RootInputTest2_cfg.py || die 'Failure using RootInputTest2_cfg.py' $?

fw --parameter-set ${FW_HOME}/IORoot/Input/test/RootInputTest3_cfg.py || die 'Failure using RootInputTest3_cfg.py' $?

fw --parameter-set ${FW_HOME}/IORoot/Input/test/RootEmptyTest_cfg.py || die 'Failure using RootEmptyTest_cfg.py' $?

fw --parameter-set ${FW_HOME}/IORoot/Input/test/RootEmptyTest2_cfg.py || die 'Failure using RootEmptyTest2_cfg.py' $?
