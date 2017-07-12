#
# Make a shareable library containing the data class
# implementation and the dictionary.
#
# Note: We will make this private, see the
#       comments below.
#
add_library(
art_test_Integration_fastclonefail_v11_ClonedProd
SHARED
${CMAKE_CURRENT_SOURCE_DIR}/ClonedProd.cc
${CMAKE_CURRENT_BINARY_DIR}/ClonedProd_dict.cc
)

#
#  Support building using either mrb or
#  standalone buildtools, we must check
#  if we are using cetlib as a UPS product
#  or not.
#
if(TARGET cetlib)
set(cetlib_lib cetlib)
else()
set(cetlib_lib ${CETLIB})
endif()

#
# Add the link libraries for the dictionary.
#
# Note: We are using target names here, and the target
#       names are from different packages, so this only
#       works right in an mrb build environment!
#
target_link_libraries(
art_test_Integration_fastclonefail_v11_ClonedProd
PRIVATE
art_Framework_Core
${cetlib_lib}
${ROOT_CORE}
)

#
# Make our data class library private, do not
# put it into the library directory of the build.
#
# Note: This is absolutely necessary, because
#       in ROOT 6 libraries are a globally shared
#       resource, and we are trying here to have
#       two different libraries for the same data
#       class in order to intentionally create a
#       situation where the data product cannot be
#       fast cloned.
#
set_target_properties(
art_test_Integration_fastclonefail_v11_ClonedProd
PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
)

#
# Make the -I directives we will need for
# the dictionary generator.
#
get_property(incdirlist DIRECTORY PROPERTY INCLUDE_DIRECTORIES)
#message("v11 incdirlist=${incdirlist}")
set(incdirs "")
foreach(dir IN LISTS incdirlist)
list(APPEND incdirs "-I${dir}")
endforeach(dir)
#message("v11 incdirs=${incdirs}")

#
# Rule to create the dictionary source file, and the associated
# rootmap and root pcm files, from the class header, guided
# by the linkdef file.
#
add_custom_command(
OUTPUT
  ${CMAKE_CURRENT_BINARY_DIR}/ClonedProd_dict.cc
  #${CMAKE_CURRENT_BINARY_DIR}/libart_test_Integration_fastclonefail_v11_ClonedProd.rootmap
  #${CMAKE_CURRENT_BINARY_DIR}/libart_test_Integration_fastclonefail_v11_ClonedProd_rdict.pcm
COMMAND
  # Get rid of any previous output.
  ${CMAKE_COMMAND} -E remove -f
    ${CMAKE_CURRENT_BINARY_DIR}/ClonedProd_dict.cc
    ${CMAKE_CURRENT_BINARY_DIR}/libart_test_Integration_fastclonefail_v11_ClonedProd.rootmap
    ${CMAKE_CURRENT_BINARY_DIR}/libart_test_Integration_fastclonefail_v11_ClonedProd_rdict.pcm
COMMAND
  # Call the dictionary generator to do the work.
  rootcling
    -f ${CMAKE_CURRENT_BINARY_DIR}/ClonedProd_dict.cc
    ${incdirs}
    -rml ${CMAKE_CURRENT_BINARY_DIR}/libart_test_Integration_fastclonefail_v11_ClonedProd.so
    -rmf ${CMAKE_CURRENT_BINARY_DIR}/libart_test_Integration_fastclonefail_v11_ClonedProd.rootmap
    -s ${CMAKE_CURRENT_BINARY_DIR}/libart_test_Integration_fastclonefail_v11_ClonedProd.so
    ${CMAKE_CURRENT_SOURCE_DIR}/ClonedProd.h
    ${CMAKE_CURRENT_SOURCE_DIR}/LinkDef.h
DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/ClonedProd.h
  ${CMAKE_CURRENT_SOURCE_DIR}/LinkDef.h
COMMENT
  "Generating dictionary for ClonedProd v11 ..."
VERBATIM
)

#
# Make our analyzer module, and link it with our
# data product library and dictionary.
#
# Note: We will keep this private, see the
#       comments below.
#
simple_plugin(
art_test_Integration_fastclonefail_v11_ClonedProdAnalyzer
"module"
art_test_Integration_fastclonefail_v11_ClonedProd
NO_INSTALL
BASENAME_ONLY
SOURCE
  ${CMAKE_CURRENT_SOURCE_DIR}/ClonedProdAnalyzer_module.cc
)

#
# Make our analyzer module private, do not
# put it into the library directory of the build.
#
# Note: This is not strictly necessary, but does
#       keep things a little bit tidier.
#
set_target_properties(
art_test_Integration_fastclonefail_v11_ClonedProdAnalyzer_module
PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
)

#
#  Read the data from the input file
#  using a private dictionary from
#  this directory where the data members
#  of the data product class need
#  schema evolution and so the Events
#  tree must be read entry by entry
#  instead of being fast cloned.
#
#  Note: We are using a script here
#  instead of running art directly
#  because we need to add the current
#  directory to the LD_LIBRARY_PATH
#  to use our private dictionary
#  for the data product class.
#

add_test(
NAME
  test_fastclonefail_r
COMMAND
  ${CMAKE_CURRENT_SOURCE_DIR}/test_fastclonefail_r.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/test_fastclone_fail_v11.fcl
    ${CMAKE_CURRENT_BINARY_DIR}/../v10/out.root
)

#
#  Our test must run after the test
#  that creates the data file for
#  us to read (see directory v10).
#
#  Also the result of running our
#  test must also provoke the unable
#  to fast clone message.
#
set_tests_properties(
test_fastclonefail_r
PROPERTIES
  DEPENDS
    test_fastclonefail_w
  PASS_REGULAR_EXPRESSION
    "Unable to fast clone tree"
)

