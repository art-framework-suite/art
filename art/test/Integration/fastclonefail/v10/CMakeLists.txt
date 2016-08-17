#
# Make a shareable library containing the data class
# implementation and the dictionary.
# Note: We will make this private, see the comments
#       below.
#
add_library(
art_test_Integration_fastclonefail_v10_ClonedProd
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

# Add the link libraries needed for the dictionary.
#
# Note: We are using target names here, and the target
#       names are from different packages, so this only
#       works right in an mrb build environment!
#
target_link_libraries(
art_test_Integration_fastclonefail_v10_ClonedProd
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
art_test_Integration_fastclonefail_v10_ClonedProd
PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
)

#
# Make the -I directives we will need for
# the dictionary generator.
#
get_property(incdirlist DIRECTORY PROPERTY INCLUDE_DIRECTORIES)
#message("v10 incdirlist=${incdirlist}")
set(incdirs "")
foreach(dir IN LISTS incdirlist)
list(APPEND incdirs "-I${dir}")
endforeach(dir)
#message("v10 incdirs=${incdirs}")

#
# Rule to create the dictionary source file, and the associated
# rootmap and root pcm files, from the class header, guided
# by the linkdef file.
#
add_custom_command(
OUTPUT
  ${CMAKE_CURRENT_BINARY_DIR}/ClonedProd_dict.cc
  #${CMAKE_CURRENT_BINARY_DIR}/libart_test_Integration_fastclonefail_v10_ClonedProd.rootmap
  #${CMAKE_CURRENT_BINARY_DIR}/libart_test_Integration_fastclonefail_v10_ClonedProd_rdict.pcm
COMMAND
  # Get rid of any previous output.
  ${CMAKE_COMMAND} -E remove -f
    ${CMAKE_CURRENT_BINARY_DIR}/ClonedProd_dict.cc
    ${CMAKE_CURRENT_BINARY_DIR}/libart_test_Integration_fastclonefail_v10_ClonedProd.rootmap
    ${CMAKE_CURRENT_BINARY_DIR}/libart_test_Integration_fastclonefail_v10_ClonedProd_rdict.pcm
COMMAND
  # Call the dictionary generator to do the work.
  rootcling
    -f ${CMAKE_CURRENT_BINARY_DIR}/ClonedProd_dict.cc
    ${incdirs}
    -rml ${CMAKE_CURRENT_BINARY_DIR}/libart_test_Integration_fastclonefail_v10_ClonedProd.so
    -rmf ${CMAKE_CURRENT_BINARY_DIR}/libart_test_Integration_fastclonefail_v10_ClonedProd.rootmap
    -s ${CMAKE_CURRENT_BINARY_DIR}/libart_test_Integration_fastclonefail_v10_ClonedProd.so
    ${CMAKE_CURRENT_SOURCE_DIR}/ClonedProd.h
    ${CMAKE_CURRENT_SOURCE_DIR}/LinkDef.h
DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/ClonedProd.h
  ${CMAKE_CURRENT_SOURCE_DIR}/LinkDef.h
COMMENT
  "Generating dictionary for v10 ..."
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
art_test_Integration_fastclonefail_v10_ClonedProdProducer
"module"
art_test_Integration_fastclonefail_v10_ClonedProd
NO_INSTALL
BASENAME_ONLY
SOURCE
  ${CMAKE_CURRENT_SOURCE_DIR}/ClonedProdProducer_module.cc
)

#
# Make our analyzer module private, do not
# put it into the library directory of the build.
#
# Note: This is not strictly necessary, but does
#       keep things a little bit tidier.
#
set_target_properties(
art_test_Integration_fastclonefail_v10_ClonedProdProducer_module
PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
)

#
#  Write data to an output file
#  using a private dictionary from
#  this directory. The intention is
#  that the data members of the data
#  product class will be changed in
#  such a way that the later test that
#  reads this file will need to use
#  schema evolution which will cause
#  fast cloning to fail.
#
add_test(
NAME
  test_fastclonefail_w
COMMAND
  ${CMAKE_CURRENT_SOURCE_DIR}/test_fastclonefail_w.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/test_fastclone_fail_v10.fcl
)

