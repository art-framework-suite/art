add_subdirectory(detail)

cet_make_library(SOURCE
    ClosingCriteria.cc
    FileStatsCollector.cc
    PostCloseFileRenamer.cc
  LIBRARIES
  PUBLIC
    art::Framework_Core
    canvas::canvas
    fhiclcpp::types
    Boost::date_time
    Boost::regex
  PRIVATE
    art::Framework_IO_detail
    Boost::filesystem
)

install_headers()
install_source()

add_subdirectory(Catalog)
add_subdirectory(ProductMix)
add_subdirectory(Sources)
