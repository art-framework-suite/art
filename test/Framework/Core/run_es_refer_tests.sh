#!/bin/bash

# Pass in name and status
function die { echo $1: status $2 ;  exit $2; }

(fw ${FW_HOME}/FWCore/Framework/test/test_es_prefer_prods_cfg.py ) || die 'Failure using test_es_prefer_prods_cfg.py' $?
(fw ${FW_HOME}/FWCore/Framework/test/test_es_prefer_sources_cfg.py ) || die 'Failure using test_es_prefer_sources_cfg.py' $?
(fw ${FW_HOME}/FWCore/Framework/test/test_es_prefer_source_beats_prod_cfg.py ) || die 'Failure using test_es_prefer_source_beats_prod_cfg.py' $?
(fw ${FW_HOME}/FWCore/Framework/test/test_prod_trumps_source_cfg.py ) || die 'Failure using test_prod_trumps_source_cfg.py' $?
(fw ${FW_HOME}/FWCore/Framework/test/test_es_prefer_2_es_sources_order1_cfg.py ) || die 'Failure using test_es_prefer_2_es_sources_order1_cfg.py' $?
(fw ${FW_HOME}/FWCore/Framework/test/test_es_prefer_2_es_sources_order2_cfg.py ) || die 'Failure using test_es_prefer_2_es_sources_order2_cfg.py' $?
(fw ${FW_HOME}/FWCore/Framework/test/test_2_es_sources_no_prefer_cfg.py ) || die 'Failure using test_2_es_sources_no_prefer_cfg.py' $?
