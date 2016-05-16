#include "art/Framework/Core/FileCatalogMetadataPlugin.h"

#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"

#include <iostream>
#include <string>

namespace arttest {
  class TestMetadata;
}

class arttest::TestMetadata : public art::FileCatalogMetadataPlugin {
public:
  TestMetadata(fhicl::ParameterSet const & pset);

private:
  void beginJob() override;
  void endJob() override;
  void collectMetadata(art::Event const & e) override;
  void beginRun(art::Run const & r) override;
  void endRun(art::Run const & r) override;
  void beginSubRun(art::SubRun const & sr) override;
  void endSubRun(art::SubRun const & sr) override;
  collection_type produceMetadata() override;
};

arttest::TestMetadata::
TestMetadata(fhicl::ParameterSet const & pset)
  :
  FileCatalogMetadataPlugin(pset)
{
}

void
arttest::TestMetadata::
beginJob()
{
  std::cout << "TestMetadataPlugin::beginJob()" << std::endl;
}

void
arttest::TestMetadata::
endJob()
{
  std::cout << "TestMetadataPlugin::endJob()" << std::endl;
}

void
arttest::TestMetadata::
collectMetadata(art::Event const &)
{
  std::cout << "TestMetadataPlugin::collectMetadata()" << std::endl;
}

void
arttest::TestMetadata::
beginRun(art::Run const &)
{
  std::cout << "TestMetadataPlugin::beginRun()" << std::endl;
}

void
arttest::TestMetadata::
endRun(art::Run const &)
{
  std::cout << "TestMetadataPlugin::endRun()" << std::endl;
}

void
arttest::TestMetadata::
beginSubRun(art::SubRun const &)
{
  std::cout << "TestMetadataPlugin::beginSubRun()" << std::endl;
}

void
arttest::TestMetadata::
endSubRun(art::SubRun const &)
{
  std::cout << "TestMetadataPlugin::endSubRun()" << std::endl;
}

auto
arttest::TestMetadata::
produceMetadata()
-> collection_type
{
  collection_type result { { "key1", "\"value1\"" }, { "key2", "\"value2\"" } };
  std::cout << "TestMetadataPlugin::produceMetadata()" << std::endl;
  return result;
}

DEFINE_ART_FILECATALOGMETADATA_PLUGIN(arttest::TestMetadata)
