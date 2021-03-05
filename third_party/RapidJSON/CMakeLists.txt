########################################################################
# RapidJSON
#
# Configured as a submodule.
########################################################################

########################################################################
# RapidJSON build options.
########################################################################
set(CMAKE_BUILD_TYPE Release)
set(RAPIDJSON_BUILD_DOC OFF CACHE BOOL "")
set(RAPIDJSON_BUILD_EXAMPLES OFF CACHE BOOL "")
set(ENABLE_INSTRUMENTATION_OPT OFF CACHE BOOL "")

########################################################################
# Manual installation and export of RapidJSON because their installation
# system uses absolute paths.
########################################################################

# Subdirectory. No automatic build.
add_subdirectory(rapidjson EXCLUDE_FROM_ALL)

########################################################################
# Post-processing for relocatability.
########################################################################

##################
# pkg-config file.
cet_script(fix-pc NO_INSTALL)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/RapidJSON-installed.pc
  COMMAND fix-pc "${CMAKE_INSTALL_PREFIX}" "/../.." "${${PROJECT_NAME}_EXEC_PREFIX}"
  rapidjson/RapidJSON.pc RapidJSON-installed.pc
  COMMENT "processing RapidJSON.pc for relocatable installation"
  MAIN_DEPENDENCY rapidjson/RapidJSON.pc
  DEPENDS fix-pc
)
add_custom_target(RapidJSON-installed-pc ALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/RapidJSON-installed.pc)

##################
# CMake Config file.
cet_script(fix-config NO_INSTALL)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/RapidJSONConfig-installed.cmake
  COMMAND fix-config "../../../../" "${${PROJECT_NAME}_INCLUDE_DIR}"
  rapidjson/RapidJSONConfig.cmake RapidJSONConfig-installed.cmake
  COMMENT "processing RapidJSONConfig.cmake for relocatable installation"
  MAIN_DEPENDENCY rapidjson/RapidJSONConfig.cmake
  DEPENDS fix-config
)
add_custom_target(RapidJSON-installed-config ALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/RapidJSONConfig-installed.cmake)

########################################################################
# Installation.
########################################################################

##################
# pkg-config file.
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/RapidJSON-installed.pc
  RENAME RapidJSON.pc
  DESTINATION ${${PROJECT_NAME}_LIBRARY_DIR}/pkgconfig)

##################
# CMake Config file.
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/RapidJSONConfig-installed.cmake
  RENAME RapidJSONConfig.cmake
  DESTINATION ${${PROJECT_NAME}_LIBRARY_DIR}/RapidJSON/cmake)

##################
# CMake Config version file.
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/rapidjson/RapidJSONConfigVersion.cmake
  DESTINATION ${${PROJECT_NAME}_LIBRARY_DIR}/RapidJSON/cmake)

##################
# Headers.
install(DIRECTORY rapidjson/include/
  DESTINATION ${${PROJECT_NAME}_INCLUDE_DIR}
  FILES_MATCHING PATTERN "*.h" PATTERN "[.#]*.h" EXCLUDE)
