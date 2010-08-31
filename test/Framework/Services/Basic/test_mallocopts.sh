#!/bin/bash

ANSWER="Malloc options: mmap_max=100001 trim_threshold=100002 top_padding=100003 mmap_threshold=100004"

CFG_PY="${FW_HOME}/FWCore/Services/test/mallocopts_cfg.py"

RESULT=`fw ${CFG_PY} 2>&1 | grep "Malloc opt"`

if [ "$ANSWER" != "$RESULT" ]
then
	echo "Failed"
	exit 1
fi

exit 0
