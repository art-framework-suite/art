#
art_make(
  EXCLUDE
  dump_range_sets.cc
  LIB_LIBRARIES
  art_Framework_IO_Root
  )

cet_make_exec(dump_range_sets
  SOURCE dump_range_sets.cc
  NO_INSTALL
  LIBRARIES
  canvas_Persistency_Provenance
  canvas_Utilities
  test_Integration_dump-range-sets
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
#  ${ROOT_RIO}
  )