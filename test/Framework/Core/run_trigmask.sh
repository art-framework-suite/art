#!/bin/bash

# Pass in name and status
function die { echo $1: status $2 ;  exit $2; }

F1=${FW_HOME}/FWCore/Framework/test/testTrigBits0_cfg.py
F2=${FW_HOME}/FWCore/Framework/test/testTrigBits1_cfg.py
F3=${FW_HOME}/FWCore/Framework/test/testTrigBits2_cfg.py
F4=${FW_HOME}/FWCore/Framework/test/testTrigBits3_cfg.py
F5=${FW_HOME}/FWCore/Framework/test/testTrigBits4_cfg.py

(fw $F1 ) || die "Failure using $F1" $?
(fw $F2 ) || die "Failure using $F2" $?
(fw $F3 ) || die "Failure using $F3" $?
(fw $F4 ) || die "Failure using $F4" $?
(fw $F5 ) || die "Failure using $F5" $?


