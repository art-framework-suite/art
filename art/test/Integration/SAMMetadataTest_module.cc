////////////////////////////////////////////////////////////////////////
// Class:       SAMMetadataTest
// Module Type: analyzer
// File:        SAMMetadataTest_module.cc
//
// Generated at Mon Feb 25 09:04:27 2013 by Christopher Green using artmod
// from art v1_03_05.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Services/System/FileCatalogMetadata.h"
#include "fhiclcpp/ParameterSet.h"

#include <memory>

namespace arttest {
  class SAMMetadataTest;
}

class arttest::SAMMetadataTest : public art::EDAnalyzer {
public:
  explicit SAMMetadataTest(fhicl::ParameterSet const& p);

  void analyze(art::Event const& e) override;

  void beginJob() override;

private:
};

arttest::SAMMetadataTest::SAMMetadataTest(fhicl::ParameterSet const& p)
  : art::EDAnalyzer(p)
{}

void
arttest::SAMMetadataTest::analyze(art::Event const&)
{}

void
arttest::SAMMetadataTest::beginJob()
{
  art::ServiceHandle<art::FileCatalogMetadata> metadata;
  metadata->addMetadataString("testMetadata", "success!");
}

DEFINE_ART_MODULE(arttest::SAMMetadataTest)
