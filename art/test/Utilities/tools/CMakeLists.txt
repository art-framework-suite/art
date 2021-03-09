include(toolPlugin)

foreach (tool
    ClassTool
    FunctionTool
    NestedClassTool
    NestedFunctionInClassTool
    NestedFunctionTool)
  build_plugin(${tool} "tool" NO_INSTALL BASENAME_ONLY)
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

# Define the tools needing the OperationBase interface.
foreach (op Add Multiply Subtract)
  build_plugin(${op}Number "tool" NO_INSTALL BASENAME_ONLY
    LIBRARIES CONDITIONAL art_test::OperationBase)
endforeach()

# Add an extra dependency to a tool's implementation.
target_link_libraries(AddNumber_tool PRIVATE fhiclcpp::types)


