set(pm_plugins
  ${CMAKE_CURRENT_SOURCE_DIR}/Analyzer.pm
  ${CMAKE_CURRENT_SOURCE_DIR}/EmptyEventTimestampPlugin.pm
  ${CMAKE_CURRENT_SOURCE_DIR}/FileCatalogMetadataPlugin.pm
  ${CMAKE_CURRENT_SOURCE_DIR}/Filter.pm
  ${CMAKE_CURRENT_SOURCE_DIR}/Output.pm
  ${CMAKE_CURRENT_SOURCE_DIR}/Producer.pm
  ${CMAKE_CURRENT_SOURCE_DIR}/Service.pm
  ${CMAKE_CURRENT_SOURCE_DIR}/ServiceImpl.pm
  ${CMAKE_CURRENT_SOURCE_DIR}/ServiceInterface.pm
)
cet_copy(${pm_plugins} DESTINATION  ${CMAKE_CURRENT_BINARY_DIR})
install(FILES ${pm_plugins}
  DESTINATION ${product}/${version}/perllib/CetSkelPlugins/)
