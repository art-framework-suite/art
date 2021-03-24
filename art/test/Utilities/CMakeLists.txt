add_subdirectory(tools)

# Note we are linking to each tool's interface target, not the tool
# itself (which is a plugin).

cet_test(make_tool_t USE_BOOST_UNIT
  LIBRARIES PRIVATE
    art_test::ClassTool_tool
    art_test::FunctionTool_tool
    art_test::NestedClassTool_tool
    art_test::NestedFunctionInClassTool_tool
    art_test::NestedFunctionTool_tool
    art_test::OperationBase # Could also (or instead) specify
                            # art_test::AddNumber_tool, etc. if we knew
                            # we were going to need them.
    art_plugin_support::toolMaker
    fhiclcpp::fhiclcpp
)

cet_test(ParameterSet_get_CLHEP_t
  LIBRARIES PRIVATE
    art::ParameterSetHelpers_CLHEP
    fhiclcpp::fhiclcpp
  )

cet_test(MallocOpts_t SOURCE MallocOpts_t.cpp
  LIBRARIES PRIVATE art::Utilities)

cet_test(pointersEqual_t USE_BOOST_UNIT LIBRARIES PRIVATE art::Utilities)
cet_test(ScheduleID_t USE_BOOST_UNIT LIBRARIES PRIVATE art::Utilities)
cet_test(parent_path_t USE_BOOST_UNIT LIBRARIES PRIVATE art::Utilities)
cet_test(remove_whitespace_t USE_BOOST_UNIT LIBRARIES PRIVATE art::Utilities)
