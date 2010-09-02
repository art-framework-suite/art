#!/bin/sh
# Pass in name and status
function die { echo $1: status $2 ;  exit $2; }

fw --parameter-set ${FW_HOME}/IOPool/Input/test/PrePoolInputTest_cfg.py || die 'Failure using PrePoolInputTest_cfg.py' $?

cp PoolInputTest.root PoolInputOther.root

fw --parameter-set ${FW_HOME}/IOPool/Input/test/PoolInputTest_cfg.py || die 'Failure using PoolInputTest_cfg.py' $?

fw --parameter-set ${FW_HOME}/IOPool/Input/test/PrePoolInputTest2_cfg.py || die 'Failure using PrePoolInputTest2_cfg.py' $?

cp PoolInputTest.root PoolInputOther.root

fw --parameter-set ${FW_HOME}/IOPool/Input/test/PoolInputTest2_cfg.py || die 'Failure using PoolInputTest2_cfg.py' $?

fw --parameter-set ${FW_HOME}/IOPool/Input/test/PoolInputTest3_cfg.py || die 'Failure using PoolInputTest3_cfg.py' $?

fw --parameter-set ${FW_HOME}/IOPool/Input/test/PoolEmptyTest_cfg.py || die 'Failure using PoolEmptyTest_cfg.py' $?

fw --parameter-set ${FW_HOME}/IOPool/Input/test/PoolEmptyTest2_cfg.py || die 'Failure using PoolEmptyTest2_cfg.py' $?
