////////////////////////////////////////////////////////////////////////
// Class:       EmptyPSetRegistryChecker
// Plugin Type: analyzer (art v1_16_02)
// File:        EmptyPSetRegistryChecker_module.cc
//
// Generated at Tue Oct 13 14:57:00 2015 by Christopher Green using cetskelgen
// from cetlib version v1_15_01.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Utilities/InputTag.h"
#include "cetlib/quiet_unit_test.hpp"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetRegistry.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

namespace arttest {
  class EmptyPSetRegistryChecker;
}


class arttest::EmptyPSetRegistryChecker : public art::EDAnalyzer {
public:
  explicit EmptyPSetRegistryChecker(fhicl::ParameterSet const & p);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  EmptyPSetRegistryChecker(EmptyPSetRegistryChecker const &) = delete;
  EmptyPSetRegistryChecker(EmptyPSetRegistryChecker &&) = delete;
  EmptyPSetRegistryChecker & operator = (EmptyPSetRegistryChecker const &) = delete;
  EmptyPSetRegistryChecker & operator = (EmptyPSetRegistryChecker &&) = delete;

  // Required functions.
  void analyze(art::Event const & e) override;

  // Selected optional functions.
  void beginJob() override;
  void respondToOpenInputFile(art::FileBlock const & fb) override;

private:

  std::size_t registrySize_;

};


arttest::EmptyPSetRegistryChecker::EmptyPSetRegistryChecker(fhicl::ParameterSet const & p)
  :
  EDAnalyzer(p),
  registrySize_(0ull)
{}

void arttest::EmptyPSetRegistryChecker::analyze(art::Event const &)
{
}

void arttest::EmptyPSetRegistryChecker::beginJob()
{
  registrySize_ = fhicl::ParameterSetRegistry::size();
}

void arttest::EmptyPSetRegistryChecker::respondToOpenInputFile(art::FileBlock const &)
{
  fhicl::ParameterSetRegistry::stageIn(); // Load everything in from DB.
  BOOST_REQUIRE_EQUAL(fhicl::ParameterSetRegistry::size(), registrySize_);
}

DEFINE_ART_MODULE(arttest::EmptyPSetRegistryChecker)
