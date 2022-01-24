# Configure file to handle differences for Mac.
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/check_libs.cc.in
  ${CMAKE_CURRENT_BINARY_DIR}/check_libs.cc @ONLY)

####################################

cet_make_library(SOURCE
    BasicOptionsHandler.cc
    BasicPostProcessor.cc
    BasicSourceOptionsHandler.cc
    BasicOutputOptionsHandler.cc
    DebugOptionsHandler.cc
    ProcessingOptionsHandler.cc
    FileCatalogOptionsHandler.cc
    OptionsHandler.cc
    artapp.cc
    find_config.cc
    run_art.cc
    detail/MetadataRegexHelpers.cc
    detail/AllowedConfiguration.cc
    detail/PluginSymbolResolvers.cc
    detail/event_start.cc
    detail/fillSourceList.cc
    detail/get_LibraryInfoCollection.cc
    detail/get_MetadataCollector.cc
    detail/get_MetadataSummary.cc
    detail/md-collector/describe.cc
    detail/md-collector/print_description_blocks.cc
    detail/output_to.cc
    detail/print_config_summary.cc
    detail/prune_configuration.cc
  LIBRARIES
  PUBLIC
    art::Utilities
    Boost::program_options
    fhiclcpp::fhiclcpp
    cetlib::cetlib
  PRIVATE
    art::Framework_EventProcessor
    art::Framework_Core
    art::Persistency_Provenance
    art::Version
    canvas::canvas
    messagefacility::MF_MessageLogger
    fhiclcpp::types
    cetlib::container_algorithms
    cetlib_except::cetlib_except
    Boost::filesystem
    TBB::tbb
    range-v3::range-v3
)

# Build an art exec.
function(art_exec TARGET_STEM IN_STEM)
  cmake_parse_arguments(PARSE_ARGV 1 AE "" "" "LIBRARIES")
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${IN_STEM}.cc.in
    ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_STEM}.cc @ONLY)
  cet_make_exec(NAME ${TARGET_STEM}
    SOURCE ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_STEM}.cc
    LIBRARIES PRIVATE
      art::Framework_Art
      messagefacility::MF_MessageLogger
      cetlib::cetlib
      Boost::program_options
      Boost::filesystem
    LIBRARIES ${AE_LIBRARIES}
  )
  # Enable plugins to access symbols exported by the exec (CMake policy CMP0065).
  set_property(TARGET ${TARGET_STEM} PROPERTY ENABLE_EXPORTS TRUE)
endfunction()

cet_make_exec(NAME get-loadable-libraries
  SOURCE get-loadable-libraries.cc
  LIBRARIES PRIVATE cetlib::cetlib
)

# Standard execs
art_exec(art art)
art_exec(gm2 art)
art_exec(lar art)
art_exec(nova art)

# Execs with Boost unit testing enabled for modules.
art_exec(art_ut art_ut LIBRARIES PRIVATE Boost::unit_test_framework)
art_exec(gm2_ut art_ut LIBRARIES PRIVATE Boost::unit_test_framework)
art_exec(lar_ut art_ut LIBRARIES PRIVATE Boost::unit_test_framework)
art_exec(nova_ut art_ut LIBRARIES PRIVATE Boost::unit_test_framework)

cet_make_completions(art ${CMAKE_CURRENT_SOURCE_DIR}/art_completion_customizations)
cet_make_completions(gm2 ${CMAKE_CURRENT_SOURCE_DIR}/art_completion_customizations)
cet_make_completions(lar ${CMAKE_CURRENT_SOURCE_DIR}/art_completion_customizations)
cet_make_completions(nova ${CMAKE_CURRENT_SOURCE_DIR}/art_completion_customizations)

cet_make_exec(NAME check_libs
  SOURCE ${CMAKE_CURRENT_BINARY_DIR}/check_libs.cc
  LIBRARIES PRIVATE art::Utilities
)

install_headers(SUBDIRS detail detail/md-collector detail/md-summary)
install_source(SUBDIRS detail)
