cet_test(PostCloseFileRenamer_t USE_BOOST_UNIT
  LIBRARIES PRIVATE
    art::Framework_IO
    canvas::canvas
    Boost::filesystem
)

cet_test(Source_t LIBRARIES PRIVATE art::Framework_IO_Sources)
