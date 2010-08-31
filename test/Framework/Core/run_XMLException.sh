#!/bin/bash

function die { echo Failure $1: status $2 ; exit $2 ; }

pushd ${LOCAL_TMP_DIR}
  echo ${LOCAL_TMP_DIR}
  fw -j testXMLSafeException.xml -p ${FW_HOME}/FWCore/Framework/test/testXMLSafeException_cfg.py
  xmllint testXMLSafeException.xml || die "fw testXMLSafeException_cfg.py produced invalid XML job report" $?

popd

exit 0
