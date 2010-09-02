#!/bin/bash

# Pass in name and status
function die { echo $1: status $2 ;  exit $2; }

(fw ${FW_HOME}/FWCore/Framework/test/test_esproducerlooper_cfg.py ) || die 'Failure using test_esproducerlooper_cfg.py' $?
(fw ${FW_HOME}/FWCore/Framework/test/test_esproducerlooper_stop_cfg.py ) || die 'Failure using test_esproducerlooper_stop_cfg.py' $?
(fw ${FW_HOME}/FWCore/Framework/test/test_esproducerlooper_override_cfg.py ) || die 'Failure using test_esproducerlooper_override_cfg.py' $?
#(fw ${FW_HOME}/FWCore/Framework/test/test_esproducerlooper_prefer_cfg.py ) || die 'Failure using test_esproducerlooper_prefer_cfg.py' $?
#(fw ${FW_HOME}/FWCore/Framework/test/test_esproducerlooper_prefer_not_source_cfg.py ) || die 'Failure using test_esproducerlooper_prefer_not_source_cfg.py' $?
(fw ${FW_HOME}/FWCore/Framework/test/test_esproducerlooper_prefer_producer_cfg.py ) || die 'Failure using test_esproducerlooper_prefer_producer_cfg.py' $?
