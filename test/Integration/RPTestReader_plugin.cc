////////////////////////////////////////////////////////////////////////
// Class:       RPTestReader
// Plugin Type: resultsproducer (art v1_15_02)
// File:        RPTestReader_plugin.cc
//
// Generated at Wed Sep 16 16:58:28 2015 by Christopher Green using cetskelgen
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

namespace arttest {
  class RPTestReader;
}

class arttest::RPTestReader : public art::ResultsProducer {
public:
  explicit RPTestReader(fhicl::ParameterSet const & p);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  RPTestReader(RPTestReader const &) = delete;
  RPTestReader(RPTestReader &&) = delete;
  RPTestReader & operator = (RPTestReader const &) = delete;
  RPTestReader & operator = (RPTestReader &&) = delete;

  // Required functions.
  void clear() override;
  void writeResults(art::Results & res) override;

  // Selected optional functions.
  void event(art::Event const & e) override;
  void readResults(art::Results const & res) override;

private:
  std::string intResultsLabel_;
};


arttest::RPTestReader::RPTestReader(fhicl::ParameterSet const & p)
:
  intResultsLabel_(p.get<std::string>("intResultsLabel"))
{
}

void arttest::RPTestReader::clear()
{
  // NOP.
}

void arttest::RPTestReader::writeResults(art::Results & res [[gnu::unused]] )
{
  // NOP.
}

void arttest::RPTestReader::event(art::Event const & e [[gnu::unused]])
{
  // NOP.
}

void arttest::RPTestReader::readResults(art::Results const & res)
{
  art::Handle<IntProduct> hi;
  res.getByLabel(intResultsLabel_, hi);
  if (hi->value != 5.0) {
    throw art::Exception(art::errors::LogicError)
      << "Unexcepted value in results int product: "
      << hi->value
      << ".\n";
  }
}

DEFINE_ART_RESULTS_PLUGIN(arttest::RPTestReader)
