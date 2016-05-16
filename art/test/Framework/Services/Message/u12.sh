#!/bin/bash

pushd $LOCAL_TMP_DIR

status=0

rm -f  u12_warnings.log u12_placeholder.log

fw -p $LOCAL_TEST_DIR/u12_cfg.py

for file in  u12_warnings.log
do
  sed -i -r -f $LOCAL_TEST_DIR/filter-timestamps.sed $file
  diff $LOCAL_TEST_DIR/unit_test_outputs/$file $LOCAL_TMP_DIR/$file
  if [ $? -ne 0 ]
  then
    echo The above discrepancies concern $file
    status=1
  fi
done

for file in u12_placeholder.log
do
  if [ -f $file ]
  then
    echo A placeholder file was created when it should not be: $file
    status=1
  fi
done

popd

exit $status
