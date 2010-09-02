#!/bin/bash

# Pass in name and status
function die { echo $1: status $2 ;  exit $2; }

F1=${FW_HOME}/FWCore/Framework/test/test_deepCall_allowUnscheduled_true_cfg.py
F2=${FW_HOME}/FWCore/Framework/test/test_deepCall_allowUnscheduled_true_fail_cfg.py
F3=${FW_HOME}/FWCore/Framework/test/test_offPath_allowUnscheduled_false_fail_cfg.py
F4=${FW_HOME}/FWCore/Framework/test/test_offPath_allowUnscheduled_true_cfg.py
F5=${FW_HOME}/FWCore/Framework/test/test_onPath_allowUnscheduled_false_cfg.py
F6=${FW_HOME}/FWCore/Framework/test/test_onPath_allowUnscheduled_true_cfg.py
F7=${FW_HOME}/FWCore/Framework/test/test_onPath_wrongOrder_allowUnscheduled_false_fail_cfg.py
F8=${FW_HOME}/FWCore/Framework/test/test_onPath_wrongOrder_allowUnscheduled_true_fail_cfg.py

(fw $F1 ) || die "Failure using $F1" $?
!(fw $F2 ) || die "Failure using $F2" $?
!(fw $F3 ) || die "Failure using $F3" $?
(fw $F4 ) || die "Failure using $F4" $?
(fw $F5 ) || die "Failure using $F5" $?
(fw $F6 ) || die "Failure using $F6" $?
!(fw $F7 ) || die "Failure using $F7" $?
!(fw $F8 ) || die "Failure using $F8" $?


