# project name
project(art_Framework_IO)

art_make(LIB_LIBRARIES
  art_Persistency_Provenance canvas_Persistency_Provenance
  ${Boost_DATE_TIME_LIBRARY}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_REGEXLIBRARY}
)

install_headers()
install_source()

# build art_Framework_IO libraries
add_subdirectory (Catalog)
add_subdirectory (ProductMix)
add_subdirectory (Root)
add_subdirectory (Sources)
add_subdirectory (detail)