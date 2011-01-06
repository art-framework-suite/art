#!/bin/sh
# Pass in name and status
function die { echo $1: status $2 ;  exit $2; }


#---------------------------
# Create first input file
#---------------------------

fw --parameter-set ${FW_HOME}/IORoot/Common/test/PreFastMergeTest_1_cfg.py || die 'Failure using PreFastMergeTest_1_cfg.py' $?

#---------------------------
# Create first input file with extra branch
#---------------------------

fw --parameter-set ${FW_HOME}/IORoot/Common/test/PreFastMergeTest_1x_cfg.py || die 'Failure using PreFastMergeTest_1x_cfg.py' $?

#---------------------------
# Create second input file
#---------------------------

fw --parameter-set ${FW_HOME}/IORoot/Common/test/PreFastMergeTest_2_cfg.py || die 'Failure using PreFastMergeTest_2_cfg.py' $?


#---------------------------
# Merge files
#---------------------------

fw -j ${LOCAL_TMP_DIR}/TestFastMergeFJRx.xml --parameter-set ${FW_HOME}/IORoot/Common/test/FastMergeTest_x_cfg.py || die 'Failure using FastMergeTest_x_cfg.py' $?
#need to filter items in job report which always change
egrep -v "<GUID>|<PFN>" $FW_HOME/IORoot/Common/test/proper_fjrx_output > $LOCAL_TMP_DIR/proper_fjrx_output_filtered
egrep -v "<GUID>|<PFN>" $LOCAL_TMP_DIR/TestFastMergeFJRx.xml  > $LOCAL_TMP_DIR/TestFastMergeFJRx_filtered.xml
diff $LOCAL_TMP_DIR/proper_fjrx_output_filtered $LOCAL_TMP_DIR/TestFastMergeFJRx_filtered.xml || die 'output framework job report is wrong' $?

fw -j ${LOCAL_TMP_DIR}/TestFastMergeFJR.xml --parameter-set ${FW_HOME}/IORoot/Common/test/FastMergeTest_cfg.py || die 'Failure using FastMergeTest_cfg.py' $?
#need to filter items in job report which always change
egrep -v "<GUID>|<PFN>" $FW_HOME/IORoot/Common/test/proper_fjr_output > $LOCAL_TMP_DIR/proper_fjr_output_filtered
egrep -v "<GUID>|<PFN>" $LOCAL_TMP_DIR/TestFastMergeFJR.xml  > $LOCAL_TMP_DIR/TestFastMergeFJR_filtered.xml
diff $LOCAL_TMP_DIR/proper_fjr_output_filtered $LOCAL_TMP_DIR/TestFastMergeFJR_filtered.xml || die 'output framework job report is wrong' $?

fw -j ${LOCAL_TMP_DIR}/TestFastMergeRLFJR.xml --parameter-set ${FW_HOME}/IORoot/Common/test/FastMergeTestRL_cfg.py || die 'Failure using FastMergeTestRL_cfg.py' $?
#need to filter items in job report which always change
egrep -v "<GUID>|<PFN>" $FW_HOME/IORoot/Common/test/proper_RLfjr_output > $LOCAL_TMP_DIR/proper_RLfjr_output_filtered
egrep -v "<GUID>|<PFN>" $LOCAL_TMP_DIR/TestFastMergeRLFJR.xml  > $LOCAL_TMP_DIR/TestFastMergeRLFJR_filtered.xml
diff $LOCAL_TMP_DIR/proper_RLfjr_output_filtered $LOCAL_TMP_DIR/TestFastMergeRLFJR_filtered.xml || die 'output run subRun framework job report is wrong' $?

fw -j ${LOCAL_TMP_DIR}/TestFastMergeRFJR.xml --parameter-set ${FW_HOME}/IORoot/Common/test/FastMergeTestR_cfg.py || die 'Failure using FastMergeTestR_cfg.py' $?
#need to filter items in job report which always change
egrep -v "<GUID>|<PFN>" $FW_HOME/IORoot/Common/test/proper_Rfjr_output > $LOCAL_TMP_DIR/proper_Rfjr_output_filtered
egrep -v "<GUID>|<PFN>" $LOCAL_TMP_DIR/TestFastMergeRFJR.xml  > $LOCAL_TMP_DIR/TestFastMergeRFJR_filtered.xml
diff $LOCAL_TMP_DIR/proper_Rfjr_output_filtered $LOCAL_TMP_DIR/TestFastMergeRFJR_filtered.xml || die 'output run framework job report is wrong' $?
