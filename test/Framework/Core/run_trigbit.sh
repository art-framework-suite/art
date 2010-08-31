#!/bin/bash

# Pass in name and status
function die { echo $1: status $2 ;  exit $2; }

F1=${FW_HOME}/FWCore/Framework/test/testBitsPass_cfg.py
F2=${FW_HOME}/FWCore/Framework/test/testBitsFail_cfg.py
F3=${FW_HOME}/FWCore/Framework/test/testBitsMove_cfg.py
F4=${FW_HOME}/FWCore/Framework/test/testBitsCount_cfg.py
F5=${FW_HOME}/FWCore/Framework/test/testFilterIgnore_cfg.py

(fw $F1 ) || die "Failure using $F1" $?
(fw $F2 ) || die "Failure using $F2" $?
(fw $F3 ) || die "Failure using $F3" $?
(fw $F4 ) || die "Failure using $F4" $?
(fw $F5 ) || die "Failure using $F5" $?


