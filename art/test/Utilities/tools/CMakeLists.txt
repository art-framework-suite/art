foreach (tool
    ClassTool
    FunctionTool
    NestedFunctionTool)
  cet_build_plugin(${tool} art::tool NO_INSTALL BASENAME_ONLY)
endforeach()

# Add transitive dependencies to a couple of our tools:
target_sources(NestedFunctionTool_tool PUBLIC NestedFunctionTool.h)
target_link_libraries(NestedFunctionTool_tool PRIVATE art_plugin_support::toolMaker)

cet_build_plugin(NestedClassTool art::tool
  NO_INSTALL BASENAME_ONLY IMPL_SOURCE
  LIBRARIES INTERFACE
  art_plugin_support::toolMaker
  fhiclcpp::fhiclcpp
)
target_sources(NestedClassTool_tool INTERFACE NestedClassTool.h)

cet_build_plugin(NestedFunctionInClassTool art::tool
  NO_INSTALL BASENAME_ONLY IMPL_SOURCE
  LIBRARIES INTERFACE
  art_plugin_support::toolMaker
  fhiclcpp::fhiclcpp
)
target_sources(NestedFunctionInClassTool_tool INTERFACE NestedFunctionInClassTool.h)

# Make a standalone interface target for an abstract tool base.
cet_make_library(LIBRARY_NAME OperationBase
  NO_INSTALL BASENAME_ONLY LIB_TYPE INTERFACE
  SOURCE OperationBase.h
)
# "write," not "make" - won't be installed or collected; NO_INSTALL
# below applies to generated tools, not the builder.
cet_write_plugin_builder(OperationTestTool art::tool Modules NO_INSTALL
  LIBRARIES CONDITIONAL art_test::OperationBase)
include(OperationTestTool)

# Define the tools needing the OperationBase interface.
foreach (op Add Multiply Subtract)
  cet_build_plugin(${op}Number OperationTestTool BASENAME_ONLY
    IMPL_TARGET_VAR ${op}Number_impl_tgt)
endforeach()

# Add an extra dependency to a tool's implementation.
target_link_libraries(${AddNumber_impl_tgt} PRIVATE fhiclcpp::types)


