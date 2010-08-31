#!/bin/sh
# Pass in name and status
function die { echo $1: status $2 ;  exit $2; }

fw --parameter-set ${FW_HOME}/IOPool/Output/test/PoolOutputTest_cfg.py || die 'Failure using PoolOutputTest_cfg.py' $?

fw --parameter-set ${FW_HOME}/IOPool/Output/test/PoolDropTest_cfg.py || die 'Failure using PoolDropTest_cfg.py' $?

fw --parameter-set ${FW_HOME}/IOPool/Output/test/PoolMissingTest_cfg.py || die 'Failure using PoolMissingTest_cfg.py' $?

fw --parameter-set ${FW_HOME}/IOPool/Output/test/PoolOutputRead_cfg.py || die 'Failure using PoolOutputRead_cfg.py' $?

fw --parameter-set ${FW_HOME}/IOPool/Output/test/PoolDropRead_cfg.py || die 'Failure using PoolDropRead_cfg.py' $?

fw --parameter-set ${FW_HOME}/IOPool/Output/test/PoolMissingRead_cfg.py || die 'Failure using PoolMissingRead_cfg.py' $?


fw ${FW_HOME}/IOPool/Output/test/TestProvA_cfg.py || die 'Failure using TestProvA_cfg.py' $?
#reads file from above
fw ${FW_HOME}/IOPool/Output/test/TestProvB_cfg.py || die 'Failure using TestProvB_cfg.py' $?
#reads file from above
fw ${FW_HOME}/IOPool/Output/test/TestProvC_cfg.py || die 'Failure using TestProvC_cfg.py' $?

fw --parameter-set ${FW_HOME}/IOPool/Output/test/PoolOutputTestUnscheduled_cfg.py || die 'Failure using PoolOutputTestUnscheduled_cfg.py' $?
