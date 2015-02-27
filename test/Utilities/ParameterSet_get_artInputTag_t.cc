#include "art/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/exception.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

  void try_and_print( fhicl::ParameterSet const & pset, std::string const & parm )
    try {
      pset.get<art::InputTag>( parm );
    }
    catch(fhicl::exception const& e ){
      std::cout << e.what() << std::endl;
    }

}

int main() {

  //===========================================================
  // Test 1-argument InputTag c'tor
  //===========================================================

  fhicl::ParameterSet pset;
  pset.put<std::string>("first_key","label:instance:processName");

  std::vector<std::string> vs;
  for ( unsigned i(0); i != 10 ; ++i ) {
    std::ostringstream value;
    value << "label"       << i << ":"
          << "instance"    << i << ":"
          << "processName" << i;
    vs.emplace_back(value.str());
  }
  pset.put("second_key", vs );

  auto const tag      = pset.get<art::InputTag>("first_key");
  auto const vec_tags = pset.get<std::vector<art::InputTag>>("second_key");

  std::cout << tag << std::endl << std::endl;

  for( const auto& tag : vec_tags ) {
    std::cout << tag << std::endl;
  }

  auto const vec_tags_empty = pset.get("third_key", std::vector<art::InputTag>() );

  try
    {
      vec_tags_empty.empty() && ( std::cout << "\nThis is empty." << std::endl );
    }
  catch(...)
    {
      std::cout << " Uh oh, this is bad." << std::endl;
    }

  std::vector<art::InputTag> vtag;
  if ( pset.get_if_present("second_key", vtag) ) {
    for( const auto& tag : vtag ) {
      std::cout << "One more time: " << tag << std::endl;
    }
  }

  //===========================================================
  // Test multi-argument constructors
  //===========================================================

  pset.put<std::vector<std::string>>("multi1",{"label","instance"});
  pset.put<std::vector<std::string>>("multi2",{"label","instance","process"});
  pset.put<std::vector<std::string>>("multierr1",{"label:something:else"});
  pset.put<std::vector<std::string>>("multierr2",{"label","instance","process","something","else"});

  try_and_print( pset, "multi1" );
  try_and_print( pset, "multi2" );
  try_and_print( pset, "multierr1" );
  try_and_print( pset, "multierr2" );

}
