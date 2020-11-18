include(CetTest)

cet_test(GlobalSignal_t USE_BOOST_UNIT LIBRARIES art::art_Framework_Services_Registry)
cet_test(LocalSignal_t USE_BOOST_UNIT LIBRARIES art::art_Utilities)
