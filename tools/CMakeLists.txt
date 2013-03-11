cet_script(checkClassVersion)

# artmod is now in cetpkgsupport, but code generation is tested here.
foreach(type Analyzer Producer Filter)
  ADD_CUSTOM_COMMAND(OUTPUT ToolsTest${type}_module.cc
    COMMAND artmod
    ARGS -A ${type} arttest::ToolsTest${type}
    DEPENDS artmod
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
  simple_plugin(ToolsTest${type} "module" NO_INSTALL)
endforeach()
