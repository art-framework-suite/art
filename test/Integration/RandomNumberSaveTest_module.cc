////////////////////////////////////////////////////////////////////////
// Class:       RandomNumberSaveTest
// Module Type: filter
// File:        RandomNumberSaveTest_module.cc
//
// Module for testing (with appropriate configuration) both aspects of
// random number state storage / retrieval: file and data product.
//
// Generated at Wed Oct 12 11:44:35 2011 by Chris Green using artmod
// from art v1_00_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Optional/RandomNumberGenerator.h"
#include "art/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"

#include "CLHEP/Random/RandFlat.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <ostream>
#include <string>
#include <vector>

namespace arttest {
  class RandomNumberSaveTest;
}

class arttest::RandomNumberSaveTest : public art::EDFilter {
public:
  typedef std::vector<size_t> prod_t;

  explicit RandomNumberSaveTest(fhicl::ParameterSet const & p);
  virtual ~RandomNumberSaveTest();

  virtual bool filter(art::Event & e);

private:

  std::string myLabel_;
  CLHEP::RandFlat dist_;
  size_t dieOnNthEvent_;
  size_t eventN_;
  bool genUnsaved_;
};

std::ostream & operator << (std::ostream & os,
                            arttest::RandomNumberSaveTest::prod_t const & v) {
  cet::copy_all(v,
                std::ostream_iterator<arttest::RandomNumberSaveTest::prod_t::value_type>
                (os, ", "));
  return os;
}

#include "art/Utilities/quiet_unit_test.hpp"

arttest::RandomNumberSaveTest::RandomNumberSaveTest(fhicl::ParameterSet const & p)
  :
  myLabel_(p.get<std::string>("module_label")),
  dist_((createEngine(get_seed_value(p)),
         art::ServiceHandle<art::RandomNumberGenerator>()->getEngine())),
  dieOnNthEvent_(p.get<size_t>("dieOnNthEvent", 0)),
  eventN_(0),
  genUnsaved_(p.get<bool>("genUnsaved", true))
{
  produces<prod_t>();
}

arttest::RandomNumberSaveTest::~RandomNumberSaveTest()
{
}

bool arttest::RandomNumberSaveTest::filter(art::Event & e)
{
  if (++eventN_ == dieOnNthEvent_) {
    throw art::Exception(art::errors::Configuration)
      << "Throwing while processing ordinal event "
      << eventN_
      << " as requested.\n";
  }
  art::Handle<prod_t> hp;
  prod_t nums;
  static size_t const nums_size = 5;
  static size_t const random_range = 1000;
  nums.reserve(nums_size);
  generate_n(std::back_inserter(nums),
       nums_size,
       [this](){ return dist_.fireInt(random_range);});
  std::cerr << "nums: " << nums << "\n";
  if (e.getByLabel(myLabel_, hp)) {
    std::cerr << "(*hp): " << *hp << "\n";
    // Reading.
    BOOST_REQUIRE((*hp) == nums);
    if (genUnsaved_) {
      // Intentionally attempt to throw the sequence off for the next event.
      dist_.fireInt(random_range);
    }
  } else {
    // Writing.
    e.put(std::unique_ptr<prod_t>(new prod_t(nums)));
  }
  return true;
}

DEFINE_ART_MODULE(arttest::RandomNumberSaveTest)
