#define BOOST_TEST_MODULE ( artapp test )
#include "boost/test/auto_unit_test.hpp"

#include "art/Framework/Core/IntermediateTablePostProcessor.h"
#include "cetlib/filepath_maker.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/intermediate_table.h"
#include "fhiclcpp/make_ParameterSet.h"
#include "fhiclcpp/parse.h"

#include <iostream>
#include <sstream>
#include <fstream>

using namespace fhicl;

BOOST_AUTO_TEST_SUITE ( IntermediateTablePostProcessorTests )

BOOST_AUTO_TEST_CASE ( emptyConfig ) {
   ParameterSet main_pset;
   intermediate_table raw_config;
   BOOST_CHECK_NO_THROW ( fhicl::parse_document("", raw_config) );
   art::IntermediateTablePostProcessor itpp;
   BOOST_CHECK_NO_THROW ( itpp.apply(raw_config) );
   BOOST_REQUIRE_NO_THROW ( make_ParameterSet(raw_config, main_pset) );
   std::cerr << main_pset.to_string() << "\n";
}

BOOST_AUTO_TEST_CASE ( test_simple_01 ) {
   ParameterSet main_pset;
   intermediate_table raw_config;
   cet::filepath_maker lookupPolicy;
   BOOST_CHECK_NO_THROW ( fhicl::parse_document("test_simple_01.fcl", lookupPolicy, raw_config) );
   art::IntermediateTablePostProcessor itpp;
   ParameterSet check_pset;
   BOOST_CHECK_NO_THROW ( make_ParameterSet(raw_config, check_pset) );
   std::cerr << check_pset.to_string() << "\n";
   BOOST_CHECK_NO_THROW ( itpp.apply(raw_config) );
   BOOST_REQUIRE_NO_THROW ( make_ParameterSet(raw_config, main_pset) );
   std::cerr << main_pset.to_string() << "\n";
}

BOOST_AUTO_TEST_SUITE_END()
