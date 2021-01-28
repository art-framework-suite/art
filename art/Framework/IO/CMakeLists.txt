art_make_library(
  LIBRARY_NAME
    art_Framework_IO
  SOURCE
    ClosingCriteria.cc
    FileStatsCollector.cc
    PostCloseFileRenamer.cc
  LIBRARIES
    Boost::date_time
    Boost::regex
    canvas::canvas
  PRIVATE
    art::art_Framework_IO_detail
    Boost::filesystem
)

install_headers()
install_source()

add_subdirectory(Catalog)
add_subdirectory(ProductMix)
add_subdirectory(Sources)
add_subdirectory(detail)
