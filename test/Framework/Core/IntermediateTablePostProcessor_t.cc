#define BOOST_TEST_MODULE ( artapp test )
#include "boost/test/auto_unit_test.hpp"

#include "art/Framework/Core/IntermediateTablePostProcessor.h"
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
   BOOST_CHECK ( fhicl::parse_document("", raw_config) );
   art::IntermediateTablePostProcessor itpp;
   BOOST_CHECK ( itpp(raw_config, main_pset) );
   std::cerr << main_pset.to_string() << "\n";
}

BOOST_AUTO_TEST_CASE ( test_simple_01 ) {
   ParameterSet main_pset;
   intermediate_table raw_config;
   std::ifstream config_stream("test_simple_01.fcl");
   BOOST_CHECK ( fhicl::parse_document(config_stream, raw_config) );
   art::IntermediateTablePostProcessor itpp;
   ParameterSet check_pset;
   BOOST_CHECK ( make_ParameterSet(raw_config, check_pset) );
   std::cerr << check_pset.to_string() << "\n";
   BOOST_CHECK ( itpp(raw_config, main_pset) );
   std::cerr << main_pset.to_string() << "\n";
}

BOOST_AUTO_TEST_SUITE_END()
