INCLUDE(CetParseArgs)
INCLUDE(CheckUpsVersion)

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

FUNCTION(_check_prereqs VAR)
  check_ups_version(cetbuildtools ${CETBUILDTOOLS_VERSION} v4_17_00 PRODUCT_MATCHES_VAR HAVE_BD_ALLDICTS)
  SET(${VAR} ${HAVE_BD_ALLDICTS} PARENT_SCOPE)
  IF (NOT HAVE_BD_ALLDICTS)
    IF (HAVE_ROOT6)
      message(FATAL_ERROR "check_class_version: proper operation with ROOT6 requires cetbuildtools >= 4.17.00")
    ELSE (HAVE_ROOT6) # ROOT5
      message(WARNING "check_class_version: use of cetbuildtools >= 4.17.00 is recommended with ROOT6")
    ENDIF (HAVE_ROOT6)
  ENDIF (NOT HAVE_BD_ALLDICTS)
ENDFUNCTION()

MACRO(check_class_version)
  CET_PARSE_ARGS(ART_CCV
    "LIBRARIES;REQUIRED_DICTIONARIES"
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
    _check_prereqs(HAVE_BD_ALLDICTS)
    # Add the check to the end of the dictionary building step.
    add_custom_command(OUTPUT ${dictname}_checked
      COMMAND checkClassVersion ${ART_CCV_EXTRA_ARGS}
      -l ${LIBRARY_OUTPUT_PATH}/lib${dictname}_dict
      -x ${CMAKE_CURRENT_SOURCE_DIR}/classes_def.xml
      -t ${dictname}_checked
      COMMENT "Checking class versions for ROOT dictionary ${dictname}"
      DEPENDS ${LIBRARY_OUTPUT_PATH}/lib${dictname}_dict.so
      )
    add_custom_target(checkClassVersion_${dictname} ALL
      DEPENDS ${dictname}_checked)
    IF (HAVE_BD_ALLDICTS)
      # All checkClassVersion invocations must wait until after *all*
      # dictionaries have been built.
      add_dependencies(checkClassVersion_${dictname} BuildDictionary_AllDicts)
    ENDIF()
    if (ART_CCV_REQUIRED_DICTIONARIES)
      add_dependencies(${dictname}_dict ${ART_CCV_REQUIRED_DICTIONARIES})
    endif()
  ENDIF()
ENDMACRO()
