simple_plugin(SamplingAnalyzer "module")

art_dictionary(NO_INSTALL)

cet_test(SamplingInput_config_t_01 HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c sampling_input_config_t_01.fcl
  DATAFILES fcl/sampling_input_config_t_01.fcl
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION "No datasets were configured for the SamplingInput source")

cet_test(SamplingInput_config_t_02 HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c sampling_input_config_t_02.fcl
  DATAFILES fcl/sampling_input_config_t_02.fcl
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION "The 'fileNames' sequence must contain 1 and only 1 filename")

cet_test(SamplingInput_config_t_03 HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c sampling_input_config_t_03.fcl
  DATAFILES fcl/sampling_input_config_t_03.fcl
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION "The specification '3' is not a valid EventID")

# Create two output files that have different (Sub)Run numbers and can
# be used to test the event sampling.
cet_test(SamplingInput_w_signal HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c sampling_input_w_signal.fcl -n 10
  DATAFILES
  fcl/sampling_input_w.fcl
  fcl/sampling_input_w_signal.fcl)

cet_test(SamplingInput_w_bkgd HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c sampling_input_w_bkgd.fcl -n 10
  DATAFILES
  fcl/sampling_input_w.fcl
  fcl/sampling_input_w_bkgd.fcl)

cet_test(SamplingInput_w_noise HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c sampling_input_w_noise.fcl -n 10
  DATAFILES
  fcl/sampling_input_w.fcl
  fcl/sampling_input_w_noise.fcl)

# SamplingInput proper tests
cet_test(SamplingInput_t_01 HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c sampling_input_t_01.fcl -n 20 -o out.root
  DATAFILES fcl/sampling_input_t_01.fcl)

cet_test(SamplingInput_r_01 HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c sampling_input_r_01.fcl
  DATAFILES
  fcl/sampling_input_t_01.fcl
  fcl/sampling_input_r_01.fcl
  REQUIRED_FILES ../SamplingInput_t_01.d/out.root
  TEST_PROPERTIES DEPENDS SamplingInput_t_01)

set_tests_properties(SamplingInput_t_01
  PROPERTIES
  DEPENDS "SamplingInput_w_signal;SamplingInput_w_bkgd;SamplingInput_w_noise")
