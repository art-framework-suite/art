########################################################################
# art_dictionary
#
# Wrapper around cetbuildtools' build_dictionary featuring the addition
# of commonly required libraries to the dictionary library link list,
# and the use of the check_class_version to update checksums and
# class versions for dictionary items.
#
####################################
# Options and Arguments
#
# UPDATE_IN_PLACE
#   Passed through to check_class_version.
#
# DICT_FUNCTIONS
#   Passed through to build_dictionary.
#
# DICT_NAME_VAR
#   Passed through to build_dictionary.
#
# DICTIONARY_LIBRARIES
#   Passed through to build_dictionary with additions.
#
# COMPILE_FLAGS
#   Passed through to build_dictionary.
#
#########################################################################
include(BuildDictionary)
include(CMakeParseArguments)
include(CheckClassVersion)

function(art_dictionary)
  cmake_parse_arguments(AD
    "UPDATE_IN_PLACE;DICT_FUNCTIONS"
    "DICT_NAME_VAR"
    "DICTIONARY_LIBRARIES;COMPILE_FLAGS"
    ${ARGN}
    )
  if(ART_PERSISTENCY_COMMON)
    # Using art as a product rather than building art itself.
    set(AD_DICTIONARY_LIBRARIES
      ${ART_PERSISTENCY_COMMON} ${ART_UTILITIES} ${CETLIB} ${AD_DICTIONARY_LIBRARIES}
      )
  else()
    set(AD_DICTIONARY_LIBRARIES
      art_Persistency_Common art_Utilities ${CETLIB} ${AD_DICTIONARY_LIBRARIES}
      )
  endif()
  if (AD_DICT_FUNCTIONS)
    set(want_build_dictionary_version v3_13_00)
    if (COMMAND check_ups_version)
      check_ups_version(cetbuildtools $ENV{CETBUILDTOOLS_VERSION} ${want_build_dictionary_version}
        PRODUCT_MATCHES_VAR understands_DICT_FUNCTIONS)
    endif()
    if (understands_DICT_FUNCTIONS)
      set(extra_args DICT_FUNCTIONS)
    else()
      message(WARNING "art_dictionary: DICT_FUNCTIONS not forwarded to build_dictionary command too old to understand it (require ${want_build_dictionary_version}, found $ENV{CETBUILDTOOLS_VERSION}).")
    endif()
  endif()
  if (AD_COMPILE_FLAGS)
    # available since at least cetbuildtools v3_00_00
    set(dict_flags COMPILE_FLAGS ${AD_COMPILE_FLAGS})
    #message(STATUS "art_dictionary: Passing ${dict_flags} to build_dictionary")
  endif()
  build_dictionary(DICT_NAME_VAR dictname
    DICTIONARY_LIBRARIES ${AD_DICTIONARY_LIBRARIES}
    ${AD_UNPARSED_ARGUMENTS}
    ${extra_args} ${dict_flags})
  if (cet_generated_code) # Bubble up to top scope.
    set(cet_generated_code ${cet_generated_code} PARENT_SCOPE)
  endif()
  if (AD_DICT_NAME_VAR)
    set (${AD_DICT_NAME_VAR} ${dictname} PARENT_SCOPE)
  endif()
  if(AD_UPDATE_IN_PLACE)
    set(AD_CCV_ARGS ${AD_CCV_ARGS} "UPDATE_IN_PLACE" ${AD_UPDATE_IN_PLACE})
  endif()
  #message(STATUS "Calling check_class_version with args ${AD_ARGS}")
  check_class_version(${AD_LIBRARIES} UPDATE_IN_PLACE ${AD_CCV_ARGS})
endfunction()
