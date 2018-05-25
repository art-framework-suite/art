# The following configurations should succeed

cet_make_exec(check_data_dependencies SOURCE check_data_dependencies.cc LIBRARIES canvas cetlib_except art_Framework_Core)

cet_test(DataDeps_EmptyConfig HANDBUILT
  TEST_EXEC check_data_dependencies
  TEST_ARGS empty.fcl
  DATAFILES
  empty.fcl
  OPTIONAL_GROUPS data-dependencies)

cet_test(DataDeps_OnePathWithProducersAndFilters HANDBUILT
  TEST_EXEC check_data_dependencies
  TEST_ARGS one_path_with_producers_and_filters.fcl
  DATAFILES
  one_path_with_producers_and_filters.fcl
  OPTIONAL_GROUPS data-dependencies)

cet_test(DataDeps_OnePathWithProducersAndFiltersWithDeps HANDBUILT
  TEST_EXEC check_data_dependencies
  TEST_ARGS one_path_with_producers_and_filters_with_deps.fcl
  DATAFILES
  one_path_with_producers_and_filters_with_deps.fcl
  OPTIONAL_GROUPS data-dependencies)

cet_test(DataDeps_TwoPathsWithProducersAndFiltersWithDeps HANDBUILT
  TEST_EXEC check_data_dependencies
  TEST_ARGS two_paths_with_producers_and_filters_with_deps.fcl
  DATAFILES
  two_paths_with_producers_and_filters_with_deps.fcl
  OPTIONAL_GROUPS data-dependencies)

cet_test(DataDeps_ModifiersAndObservers HANDBUILT
  TEST_EXEC check_data_dependencies
  TEST_ARGS modifiers_and_observers.fcl
  DATAFILES
  modifiers_and_observers.fcl
  OPTIONAL_GROUPS data-dependencies)

cet_test(DataDeps_ModifiersAndObserversWithDeps HANDBUILT
  TEST_EXEC check_data_dependencies
  TEST_ARGS modifiers_and_observers_with_deps.fcl
  DATAFILES
  modifiers_and_observers_with_deps.fcl
  OPTIONAL_GROUPS data-dependencies)

cet_test(DataDeps_TwoPathsWithProducersWithDeps HANDBUILT
  TEST_EXEC check_data_dependencies
  TEST_ARGS two_paths_with_producers_with_deps.fcl
  DATAFILES
  two_paths_with_producers_with_deps.fcl
  OPTIONAL_GROUPS data-dependencies)

cet_test(DataDeps_ThreePathsWithProducersWithDeps HANDBUILT
  TEST_EXEC check_data_dependencies
  TEST_ARGS three_paths_with_producers_with_deps.fcl
  DATAFILES
  three_paths_with_producers_with_deps.fcl
  OPTIONAL_GROUPS data-dependencies)

cet_test(DataDeps_OnePathWithProducersAndACircularity HANDBUILT
  TEST_EXEC check_data_dependencies
  TEST_ARGS one_path_with_producers_and_a_circularity.fcl
  DATAFILES
  one_path_with_producers_and_a_circularity.fcl)

cet_test(DataDeps_OnePathWithProducersAndSelfCircularity HANDBUILT
  TEST_EXEC check_data_dependencies
  TEST_ARGS one_path_with_producers_and_self_circularity.fcl
  DATAFILES
  one_path_with_producers_and_self_circularity.fcl
  OPTIONAL_GROUPS data-dependencies)
