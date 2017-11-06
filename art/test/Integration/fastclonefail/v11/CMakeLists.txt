art_make_library(
  LIBRARIES
  art_Framework_Core cetlib ${ROOT_CORE}
  SOURCE ClonedProd.cc
  NO_INSTALL
)

art_dictionary(NO_INSTALL
  DICTIONARY_LIBRARIES art_test_Integration_fastclonefail_v11
  )

simple_plugin(ClonedProdAnalyzer
  "module"
  art_test_Integration_fastclonefail_v11
  NO_INSTALL
  )

set_target_properties(
  art_test_Integration_fastclonefail_v11
  art_test_Integration_fastclonefail_v11_dict
  art_test_Integration_fastclonefail_v11_ClonedProdAnalyzer_module
  PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY
  ${CMAKE_CURRENT_BINARY_DIR}
  )

#  Write data to an output file using a private dictionary from this
#  directory. The intention is that the data members of the data product
#  class will be changed in such a way that the later test that reads
#  this file will need to use schema evolution which will cause fast
#  cloning to fail.
cet_test(test_fastclonefail_r.sh PREBUILT
  TEST_ARGS test_fastclone_fail_v11.fcl ../../v10/test_fastclonefail_w.sh.d/out.root
  TEST_PROPERTIES DEPENDS test_fastclonefail_w.sh
  PASS_REGULAR_EXPRESSION "Unable to fast clone tree"
  DATAFILES test_fastclone_fail_v11.fcl
  REQUIRED_FILES ${CMAKE_CURRENT_BINARY_DIR}/../v10/test_fastclonefail_w.sh.d/out.root
)
