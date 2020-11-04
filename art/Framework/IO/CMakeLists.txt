art_make(LIB_LIBRARIES ### MIGRATE-ACTION-RECOMMENDED: use art_make_library(), art_dictionary(), simple_plugin() with explicit source lists
  art_Framework_IO_detail
  canvas
  cetlib_except
  Boost::date_time
  Boost::filesystem
  Boost::regex)

install_headers()
install_source()

# build art_Framework_IO libraries
add_subdirectory(Catalog)
add_subdirectory(ProductMix)
add_subdirectory(Sources)
add_subdirectory(detail)
