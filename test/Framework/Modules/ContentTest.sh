#!/bin/sh

function die { echo $1: status $2; exit $2; }

fw ${FW_HOME}/FWCore/Modules/test/ContentTest_cfg.py || die 'failed running fw ContentTest_cfg.py' $?
fw ${FW_HOME}/FWCore/Modules/test/printeventsetupcontent_cfg.py || die 'failed running fw printeventsetupcontent_cfg.py' $?
