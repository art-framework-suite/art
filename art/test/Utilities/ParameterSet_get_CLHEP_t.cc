#include "art/Utilities/ParameterSetHelpers/CLHEP_ps.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/exception.h"

#include <iostream>
#include <vector>

int
main()
{

  fhicl::ParameterSet pset;

  std::vector<double> const clhep_vec2 = {0.6, 123098};
  std::vector<double> const clhep_vec3 = {0.4, -0.5, 123.0};
  std::vector<double> const clhep_lorVec = {0.4, -0.5, 123.0, 58.};

  pset.put<std::vector<double>>("clhepTwoVector", clhep_vec2);
  pset.put<std::vector<double>>("clhepThreeVector", clhep_vec3);
  pset.put<std::vector<double>>("clhepLorVector", clhep_lorVec);

  auto const vec2 = pset.get<CLHEP::Hep2Vector>("clhepTwoVector");
  auto const vec3 = pset.get<CLHEP::Hep3Vector>("clhepThreeVector");
  auto const lorVec = pset.get<CLHEP::HepLorentzVector>("clhepLorVector");

  // Various errors
  try {
    pset.get<CLHEP::Hep3Vector>("clhepTwoVector");
  }
  catch (fhicl::exception const& e) {
    std::cout << e.what() << std::endl;
  }

  try {
    pset.get<CLHEP::Hep2Vector>("clhepLorVector");
  }
  catch (fhicl::exception const& e) {
    std::cout << e.what() << std::endl;
  }
}
