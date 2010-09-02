#!/bin/bash

# Pass in name and status
function die { echo $1: status $2 ;  exit $2; }

export STD_OUT=${LOCAL_TMP_DIR}/cout.txt
export STD_ERR=${LOCAL_TMP_DIR}/cerr.txt

#(totalview fw -a --parameter-set ${LOCAL_TEST_DIR}/messageLogger_cfg.py > ${STD_OUT}  2> ${STD_ERR} ) || die 'Failure using messageLogger_cfg.py' $?
(fw --parameter-set ${LOCAL_TEST_DIR}/messageLogger_cfg.py > ${STD_OUT}  2> ${STD_ERR} ) || die 'Failure using messageLogger_cfg.py' $?
