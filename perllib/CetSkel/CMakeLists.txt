configure_file($ENV{CETLIB_DIR}/perllib/PluginVersionInfo.pm.in
  ${CMAKE_CURRENT_BINARY_DIR}/${product}/PluginVersionInfo.pm
  @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${product}/PluginVersionInfo.pm
  DESTINATION ${product}/${version}/perllib/CetSkel/${product}/)

set(pm_files
  ${CMAKE_CURRENT_SOURCE_DIR}/AnalyzerCommon.pm
  ${CMAKE_CURRENT_SOURCE_DIR}/FilterProducerCommon.pm
  ${CMAKE_CURRENT_SOURCE_DIR}/ServiceCommon.pm
)

cet_copy(${pm_files} DESTINATION  ${CMAKE_CURRENT_BINARY_DIR})
install(FILES ${pm_files}
  DESTINATION ${product}/${version}/perllib/CetSkel/)
