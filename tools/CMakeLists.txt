CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/artmod.in
  "${CMAKE_CURRENT_BINARY_DIR}/artmod"
  @ONLY
  )

ADD_CUSTOM_TARGET(artmod ALL
  COMMAND ${CMAKE_COMMAND} -E
  copy  "${CMAKE_CURRENT_BINARY_DIR}/artmod"
  "${EXECUTABLE_OUTPUT_PATH}/"
  DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/artmod"
  )

ADD_CUSTOM_TARGET(checkClassVersion ALL
  COMMAND ${CMAKE_COMMAND} -E
  copy "${CMAKE_CURRENT_SOURCE_DIR}/checkClassVersion"
  "${EXECUTABLE_OUTPUT_PATH}/"
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/checkClassVersion"
  )

foreach(type Analyzer Producer Filter)
  ADD_CUSTOM_COMMAND(OUTPUT ToolsTest${type}_module.cc
    COMMAND artmod
    ARGS -A ${type} arttest::ToolsTest${type}
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/artmod
    COMMENT "Generating ToolsTest${type}_module.cc"
    VERBATIM
    )
  # We knowingly generate code with unused parameters and missing
  # returns: user is supposed to fill those in when xe implements the module
  # methods.
  SET_SOURCE_FILES_PROPERTIES(ToolsTest${type}_module.cc
    PROPERTIES
    COMPILE_FLAGS "-Wno-unused-parameter -Wno-return-type"
    )
  simple_plugin(ToolsTest${type} "module" NOINSTALL)
endforeach()

INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/artmod
  ${EXECUTABLE_OUTPUT_PATH}/checkClassVersion
  DESTINATION ${flavorqual_dir}/bin
  )
