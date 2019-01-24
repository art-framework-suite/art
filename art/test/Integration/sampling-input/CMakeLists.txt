simple_plugin(SamplingAnalyzer "module" NO_INSTALL)

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

cet_test(SamplingInput_config_t_04 HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c sampling_input_config_t_04.fcl
  DATAFILES fcl/sampling_input_config_t_04.fcl
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION "It is illegal to specify a negative weight \\(\\-")

cet_test(SamplingInput_config_t_05 HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c sampling_input_config_t_05.fcl
  DATAFILES fcl/sampling_input_config_t_05.fcl
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION "The sum of weights over all datasets must be greater than 0")

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
cet_test(SamplingInput_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c sampling_input_t.fcl -n 20 -o out.root
  DATAFILES fcl/sampling_input_t.fcl)

cet_test(SamplingInput_r HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c sampling_input_r.fcl
  DATAFILES
  fcl/sampling_input_t.fcl
  fcl/sampling_input_r.fcl
  REQUIRED_FILES ../SamplingInput_t.d/out.root
  TEST_PROPERTIES DEPENDS SamplingInput_t)

set_tests_properties(SamplingInput_t
  PROPERTIES
  DEPENDS "SamplingInput_w_signal;SamplingInput_w_bkgd;SamplingInput_w_noise")
