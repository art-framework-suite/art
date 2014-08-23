INCLUDE(CetParseArgs)

EXECUTE_PROCESS(COMMAND root-config --has-python
  RESULT_VARIABLE ART_CCV_ROOT_CONFIG_OK
  OUTPUT_VARIABLE ART_CCV_ENABLED
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

IF(NOT ART_CCV_ROOT_CONFIG_OK EQUAL 0)
  MESSAGE(FATAL_ERROR "Could not execute root-config successfully to interrogate configuration: exit code ${ART_CCV_ROOT_CONFIG_OK}")
ENDIF()

IF(NOT ART_CCV_ENABLED)
  MESSAGE("WARNING: The version of root against which we are building currently has not been built "
    "with python support: ClassVersion checking is disabled."
    )
ENDIF()

string(REPLACE "/" "+" _ccv_target "${EXECUTABLE_OUTPUT_PATH}/checkClassVersion")

MACRO(check_class_version)
  CET_PARSE_ARGS(ART_CCV
    "LIBRARIES"
    "UPDATE_IN_PLACE"
    ${ARGN}
    )
  IF(ART_CCV_LIBRARIES)
    MESSAGE(FATAL_ERROR "LIBRARIES option not supported at this time: "
      "ensure your library is linked to any necessary libraries not already pulled in by ART.")
  ENDIF()
  IF(ART_CCV_UPDATE_IN_PLACE)
    SET(ART_CCV_EXTRA_ARGS ${ART_CCV_EXTRA_ARGS} "-G")
  ENDIF()
  IF(NOT dictname)
    MESSAGE(FATAL_ERROR "CHECK_CLASS_VERSION must be called after BUILD_DICTIONARY.")
  ENDIF()
  IF(ART_CCV_ENABLED)
    # Add the check to the end of the dictionary building step.
    add_custom_command(TARGET ${dictname}_dict POST_BUILD
      COMMAND checkClassVersion ${ART_CCV_EXTRA_ARGS}
      -l ${LIBRARY_OUTPUT_PATH}/lib${dictname}_dict
      -x ${CMAKE_CURRENT_SOURCE_DIR}/classes_def.xml
      VERBATIM
      )
    add_dependencies(${dictname}_dict art_Framework_Core ${_ccv_target})
  ENDIF()
ENDMACRO()
