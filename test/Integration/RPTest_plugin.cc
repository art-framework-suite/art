////////////////////////////////////////////////////////////////////////
// Class:       RPTest
// Plugin Type: resultsproducer (art v1_15_02)
// File:        RPTest_plugin.cc
//
// Generated at Wed Sep 16 16:59:53 2015 by Christopher Green using cetskelgen
// from cetlib version v1_14_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/ResultsProducer.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Results.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Utilities/Exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "test/TestObjects/ToyProducts.h"

#include <algorithm>
#include <iterator>
#include <set>
#include <sstream>

namespace arttest {
  class RPTest;
}

class arttest::RPTest : public art::ResultsProducer {
public:
  explicit RPTest(fhicl::ParameterSet const & p);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  RPTest(RPTest const &) = delete;
  RPTest(RPTest &&) = delete;
  RPTest & operator = (RPTest const &) = delete;
  RPTest & operator = (RPTest &&) = delete;

  // Required functions.
  void clear() override;
  void writeResults(art::Results & res) override;

  // Selected optional functions.
  void beginJob() override;
  void beginRun(art::Run const & r) override;
  void beginSubRun(art::SubRun const & sr) override;
  void endJob() override;
  void endRun(art::Run const & r) override;
  void endSubRun(art::SubRun const &) override;
  void event(art::Event const & e) override;
  void readResults(art::Results const & res) override;

private:

  std::set<std::string> seenEntryPoints_;

};

arttest::RPTest::RPTest(fhicl::ParameterSet const &)
:
  seenEntryPoints_( {
      "Constructor", "clear", "writeResults", "beginJob", "beginRun",
        "beginSubRun", "event", "endJob", "endRun", "endSubRun",
        "readResults" } )
{
  seenEntryPoints_.erase("Constructor");
  produces<IntProduct>();
}

void arttest::RPTest::clear()
{
  seenEntryPoints_.erase("clear");
}

void arttest::RPTest::writeResults(art::Results & res)
{
  seenEntryPoints_.erase("writeResults");
  res.put(std::make_unique<IntProduct>(5.0));
}

void arttest::RPTest::beginJob()
{
  seenEntryPoints_.erase("beginJob");
}

void arttest::RPTest::beginRun(art::Run const &)
{
  seenEntryPoints_.erase("beginRun");
}

void arttest::RPTest::beginSubRun(art::SubRun const &)
{
  seenEntryPoints_.erase("beginSubRun");
}

void arttest::RPTest::endJob()
{
  seenEntryPoints_.erase("endJob");
  if (!seenEntryPoints_.empty()) {
    std::ostringstream failed;
    std::copy(seenEntryPoints_.cbegin(),
              seenEntryPoints_.cend(),
              std::ostream_iterator<std::string>(failed, " "));
    throw art::Exception(art::errors::LogicError)
      << "Failed to hit the following entry points for ResultsProducer: "
      << failed.str()
      << ".\n";
  }
}

void arttest::RPTest::endRun(art::Run const &)
{
  seenEntryPoints_.erase("endRun");
}

void arttest::RPTest::endSubRun(art::SubRun const &)
{
  seenEntryPoints_.erase("endSubRun");
}

void arttest::RPTest::event(art::Event const &)
{
  seenEntryPoints_.erase("event");
}

void arttest::RPTest::readResults(art::Results const &)
{
  seenEntryPoints_.erase("readResults");
}

DEFINE_ART_RESULTS_PLUGIN(arttest::RPTest)
