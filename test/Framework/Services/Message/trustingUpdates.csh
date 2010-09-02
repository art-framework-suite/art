#!/bin/csh -v

# This script updates all the unit test outputs.
# It should NOT be used until a suitable set of fws and comparisions
# have been done, to see that the change is working correctly.
# If in any doubt about some case, check the outputs by hand for that case.

# This script needs to be maintained when new unit tests are added.

fw -e u1.cfg
mv     u1_* unit_test_outputs/
fw u2.cfg >& u2_cerr.mout
mv     u2_* unit_test_outputs/
fw u3.cfg
mv     u3_* unit_test_outputs/
fw u4.cfg
mv     u4_* unit_test_outputs/
fw u5.cfg
mv     u5_* unit_test_outputs/
fw u6.cfg
mv     u6_* unit_test_outputs/
fw -e u7.cfg
mv     u7_* unit_test_outputs/
fw u8.cfg
mv     u8_* unit_test_outputs/
fw -e u9.cfg
mv     warnings.log unit_test_outputs/
mv     infos.log unit_test_outputs/
fw -e u10.cfg
mv     u10_* unit_test_outputs/
fw u11.cfg
mv     u11_* unit_test_outputs/
fw u12.cfg
mv     u12_* unit_test_outputs/
fw u13.cfg
mv     u13_* unit_test_outputs/
fw -e u14.cfg
mv     u14_* unit_test_outputs/
fw u15.cfg
mv     u15_* unit_test_outputs/
fw -e u16.cfg
mv     u16_* unit_test_outputs/
fw u17.cfg
mv     u17_* unit_test_outputs/
fw u18.cfg
mv     u18_* unit_test_outputs/
fw u19.cfg
mv     u19_* unit_test_outputs/
fw -e u20.cfg
mv     u20_* unit_test_outputs/
mv     FrameworkJobReport.xml unit_test_outputs/
fw u21.cfg
mv     u21_* unit_test_outputs/
