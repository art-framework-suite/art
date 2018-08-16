art_make(LIB_LIBRARIES
  art_Framework_IO_detail
  canvas
  cetlib_except
  ${Boost_DATE_TIME_LIBRARY}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_REGEX_LIBRARY}
  ${Boost_SYSTEM_LIBRARY})

install_headers()
install_source()

# build art_Framework_IO libraries
add_subdirectory(Catalog)
add_subdirectory(ProductMix)
add_subdirectory(Sources)
add_subdirectory(detail)
