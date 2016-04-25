#
art_make(
  EXCLUDE
  dump_file_info.cc
  LIB_LIBRARIES
  art_Framework_IO_Root
  )

cet_make_exec(dump_file_info
  SOURCE dump_file_info.cc
  NO_INSTALL
  LIBRARIES
  canvas_Persistency_Provenance
  canvas_Utilities
  art_test_Integration_dump-file-info
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  )