cet_make_exec(check_data_dependencies
  SOURCE check_data_dependencies.cc
  LIBRARIES canvas cetlib_except art_Framework_Core)

set(config_files
  empty.fcl
  modifiers_and_observers.fcl
  modifiers_and_observers_with_deps.fcl
  one_path_with_producers_and_a_current_process_circularity.fcl
  one_path_with_producers_and_an_apparent_circularity.fcl
  one_path_with_producers_and_filters.fcl
  one_path_with_producers_and_filters_with_deps.fcl
  one_path_with_wrong_signature_lookup_1.fcl
  one_path_with_wrong_signature_lookup_2.fcl
  one_path_with_wrong_signature_lookup_3.fcl
  one_path_with_producers_and_self_circularity.fcl
  three_paths_with_producers_with_current_process_deps.fcl
  three_paths_with_producers_with_deps.fcl
  three_paths_with_producers_with_input_source_deps.fcl
  two_paths_with_producers_and_filters_with_deps.fcl
  two_paths_with_producers_with_current_process_deps.fcl
  two_paths_with_producers_with_deps.fcl)

function(capitalize str output)
  string(SUBSTRING ${str} 0 1 FIRST_LETTER)
  string(SUBSTRING ${str} 1 -1 TAIL)
  string(TOUPPER ${FIRST_LETTER} FIRST_LETTER)
  set(${output} "${FIRST_LETTER}${TAIL}" PARENT_SCOPE)
endfunction()

# The get_testname function converts from snake case to camel case.
function(get_testname testname config)
  get_filename_component(basename ${config} NAME_WE)
  string(REGEX MATCH "([a-zA-Z0-9]+)_?" head ${basename})
  capitalize(${head} capitalized)
  string(REPLACE "_" "" no_underscore_cap ${capitalized})
  set(capped_components ${no_underscore_cap})
  string(REGEX MATCHALL "_([a-zA-Z0-9]+)" matches ${basename})
  foreach(component ${matches})
    string(REPLACE "_" "" no_underscore_comp ${component})
    capitalize(${no_underscore_comp} component_to_add)
    set(capped_components ${capped_components} ${component_to_add})
  endforeach()
  string(REPLACE ";" "" tmp_name ${capped_components})
  set(testname ${tmp_name} PARENT_SCOPE)
endfunction()


foreach(config ${config_files})
  get_testname(testname ${config})
  cet_test(DataDeps_${testname} HANDBUILT
  TEST_EXEC check_data_dependencies
  TEST_ARGS ${config}
  DATAFILES
  ${config}
  OPTIONAL_GROUPS data-dependencies)
endforeach()
