foreach (tool
    ClassTool
    FunctionTool
    NestedClassTool
    NestedFunctionInClassTool
    NestedFunctionTool)
  cet_build_plugin(${tool} art::tool NO_INSTALL BASENAME_ONLY)
endforeach()

# Add transitive dependencies to a couple of our tools:
target_sources(NestedFunctionTool_tool PUBLIC NestedFunctionTool.h)
target_link_libraries(NestedFunctionTool_tool art_plugin_support::toolMaker)

target_sources(NestedClassTool_tool INTERFACE NestedClassTool.h)
target_link_libraries(NestedClassTool_tool INTERFACE fhiclcpp::fhiclcpp)

target_sources(NestedFunctionInClassTool_tool PUBLIC NestedFunctionInClassTool.h)
target_link_libraries(NestedFunctionInClassTool_tool PUBLIC
  art_plugin_types::tool
  fhiclcpp::fhiclcpp
)

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


