# Should pass and provide config output file for comparison in
# BasicOutputOptions_01_r against reference.
cet_test(BasicOutputOptions_01_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS
    -o out.root
    --tmpdir /tmp
    -c /dev/null
    --debug-config=config.out
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION "\\*\\* Config output to file")
config_ref_test(BasicOutputOptions_01
 OUTPUT_FILTERS DEFAULT ${filterOutputModule})

# Specified stream name with empty outputs.
cet_test(BasicOutputOptions_12_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS
    -o myOut:out.root
    --tmpDir /tmp
    -c /dev/null
    --debug-config=config.out
  TEST_PROPERTIES
  PASS_REGULAR_EXPRESSION "Config output to file")
config_ref_test(BasicOutputOptions_12
 OUTPUT_FILTERS DEFAULT ${filterOutputModule})
########################################################################

# Must make sure that excluded paths that share modules with
# non-excluded paths do not cause problems.  It is sufficient for the
# job to finish successfully--i.e. no output need be checked for this
# test.
cet_test(ExcludedPaths_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c excluded_paths_t.fcl
  DATAFILES fcl/excluded_paths_t.fcl)

cet_test(PruneConfig_t_01 HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c prune_config_t_01.fcl --prune-config
  DATAFILES fcl/prune_config_t_01.fcl
  TEST_PROPERTIES WILL_FAIL TRUE)

# These tests check that the defaults for reporting unused modules are
# correct for the mu2e executable and the art executable (representing
# all other framework executables).
cet_test(ReportUnused_art_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c UnusedModule_t.fcl --debug-config=/dev/null
  DATAFILES fcl/UnusedModule_t.fcl
  TEST_PROPERTIES PASS_REGULAR_EXPRESSION "The following module label is either not assigned to any path")

cet_test(ReportUnused_mu2e_t HANDBUILT
  TEST_EXEC mu2e
  TEST_ARGS -c UnusedModule_t.fcl --debug-config=/dev/null
  DATAFILES fcl/UnusedModule_t.fcl
  TEST_PROPERTIES FAIL_REGULAR_EXPRESSION "The following module label is either not assigned to any path")

simple_plugin(ScheduleThreadChecker "module" NO_INSTALL)
cet_test(CheckSchedulesThreads_t HANDBUILT
  TEST_EXEC art
  TEST_ARGS -c check_schedules_threads_t.fcl
  DATAFILES fcl/check_schedules_threads_t.fcl)
