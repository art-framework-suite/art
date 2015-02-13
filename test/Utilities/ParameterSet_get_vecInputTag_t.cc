#include "art/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

int main() {

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

}
