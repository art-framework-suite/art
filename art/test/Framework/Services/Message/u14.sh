#!/bin/bash

pushd $LOCAL_TMP_DIR

status=0

rm -f u14_errors.log u14_warnings.log u14_infos.log u14_debugs.log u14_default.log u14_job_report.mxml

fw -j u14_job_report.mxml -p $LOCAL_TEST_DIR/u14_cfg.py

for file in u14_errors.log u14_warnings.log u14_infos.log u14_debugs.log u14_default.log u14_job_report.mxml
do
  sed -i -r -f $LOCAL_TEST_DIR/filter-timestamps.sed $file
  diff $LOCAL_TEST_DIR/unit_test_outputs/$file $LOCAL_TMP_DIR/$file
# fancy_diffs $LOCAL_TEST_DIR/unit_test_outputs/$file $LOCAL_TMP_DIR/$file $LOCAL_TEST_DIR/$file.diffs
  if [ $? -ne 0 ]
  then
    echo The above discrepancies concern $file
    status=1
  fi
done

popd

exit $status
