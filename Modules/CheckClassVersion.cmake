INCLUDE(CetParseArgs)

MACRO(check_class_version)
  CET_PARSE_ARGS(ART_CCV
    "LIBRARIES"
    "UPDATE_IN_PLACE"
    ${ARGN}
    )
  IF(ART_CCV_LIBRARIES)
    MESSAGE(FATAL_ERROR "LIBRARIES option not supported at this time: ensure your library is linked to any necessary libraries not already pulled in by ART.")
  ENDIF()
  IF(ART_CCV_UPDATE_IN_PLACE)
    SET(EXTRA_ARGS ${EXTRA_ARGS} "-G")
  ENDIF()
  IF(NOT dictname)
    MESSAGE(FATAL_ERROR "CHECK_CLASS_VERSION must be called after BUILD_DICTIONARY.")
  ENDIF()
  ADD_CUSTOM_TARGET(${dictname}_check ALL
    COMMAND ${EXECUTABLE_OUTPUT_PATH}/checkClassVersion ${EXTRA_ARGS} -l ${LIBRARY_OUTPUT_PATH}/lib${dictname}_dict.so -x ${CMAKE_CURRENT_SOURCE_DIR}/classes_def.xml
    DEPENDS checkClassVersion classes_def.xml
    VERBATIM
    )
  ADD_DEPENDENCIES(${dictname}_check ${dictname}_dict ${dictname} art_Framework_Core)
ENDMACRO()
