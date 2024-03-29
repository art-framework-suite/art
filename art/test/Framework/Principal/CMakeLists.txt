# include(cetTest)
find_package(Catch2 REQUIRED)

foreach(test Closed Open)
  cet_test(${test}RangeSetHandler_t USE_BOOST_UNIT
    LIBRARIES PRIVATE
      art::Framework_Principal
      canvas::canvas
   )
endforeach()

set(event_test_libraries
    art::Framework_Principal
    art::Persistency_Common
    art::Persistency_Provenance
    art::Version
    art_test::TestObjects
    canvas::canvas
    fhiclcpp::fhiclcpp
)

cet_test(Event_t USE_BOOST_UNIT
  LIBRARIES PRIVATE
    ${event_test_libraries}
    cetlib::container_algorithms
)

cet_test(EventPrincipal_t USE_BOOST_UNIT
  LIBRARIES PRIVATE ${event_test_libraries})

cet_test(Selector_t USE_BOOST_UNIT
  LIBRARIES PRIVATE art::Framework_Principal)

cet_test(Handle_concepts_t USE_CATCH2_MAIN LIBRARIES PRIVATE art::Framework_Principal)

