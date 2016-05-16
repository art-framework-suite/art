#!/bin/sh
# Pass in name and status
function die { echo $1: status $2 ;  exit $2; }

fw --parameter-set ${FW_HOME}/IORoot/Output/test/RootOutputTest_cfg.py || die 'Failure using RootOutputTest_cfg.py' $?

fw --parameter-set ${FW_HOME}/IORoot/Output/test/RootDropTest_cfg.py || die 'Failure using RootDropTest_cfg.py' $?

fw --parameter-set ${FW_HOME}/IORoot/Output/test/RootMissingTest_cfg.py || die 'Failure using RootMissingTest_cfg.py' $?

fw --parameter-set ${FW_HOME}/IORoot/Output/test/RootOutputRead_cfg.py || die 'Failure using RootOutputRead_cfg.py' $?

fw --parameter-set ${FW_HOME}/IORoot/Output/test/RootDropRead_cfg.py || die 'Failure using RootDropRead_cfg.py' $?

fw --parameter-set ${FW_HOME}/IORoot/Output/test/RootMissingRead_cfg.py || die 'Failure using RootMissingRead_cfg.py' $?


fw ${FW_HOME}/IORoot/Output/test/TestProvA_cfg.py || die 'Failure using TestProvA_cfg.py' $?
#reads file from above
fw ${FW_HOME}/IORoot/Output/test/TestProvB_cfg.py || die 'Failure using TestProvB_cfg.py' $?
#reads file from above
fw ${FW_HOME}/IORoot/Output/test/TestProvC_cfg.py || die 'Failure using TestProvC_cfg.py' $?

fw --parameter-set ${FW_HOME}/IORoot/Output/test/RootOutputTestUnscheduled_cfg.py || die 'Failure using RootOutputTestUnscheduled_cfg.py' $?
