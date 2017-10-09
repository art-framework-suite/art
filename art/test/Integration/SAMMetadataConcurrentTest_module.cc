#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/System/FileCatalogMetadata.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "cetlib/SimultaneousFunctionSpawner.h"
#include "cetlib/canonical_string.h"
#include "cetlib/quiet_unit_test.hpp"

#include <string>
#include <vector>

using namespace std;
using namespace std::string_literals;

namespace {

  auto
  mockMetadata()
  {
    art::FileCatalogMetadata::collection_type result;
    for (unsigned i{}; i != 10; ++i) {
      auto const str_index = std::to_string(i);
      result.emplace_back("k" + str_index, "v" + str_index);
    }
    return result;
  }

  class SAMMetadataConcurrentTest : public art::EDAnalyzer {
  public:
    struct Config {
    };
    using Parameters = art::EDAnalyzer::Table<Config>;
    explicit SAMMetadataConcurrentTest(Parameters const& p) : art::EDAnalyzer{p}
    {}

  private:
    void analyze(art::Event const&) override{};

    void
    beginJob() override
    {
      vector<function<void()>> tasks;
      auto mockMD = mockMetadata();
      // Cannot yet create a ServiceHandle on a different thread.
      // Will create the handle and share it across all threads.
      art::ServiceHandle<art::FileCatalogMetadata> md;
      cet::transform_all(mockMD, back_inserter(tasks), [&md](auto const& pr) {
        return [&md, &pr] { md->addMetadataString(pr.first, pr.second); };
      });
      cet::SimultaneousFunctionSpawner sfs{tasks};
    }

    void
    endJob() override
    {
      art::FileCatalogMetadata::collection_type coll;
      art::ServiceHandle<art::FileCatalogMetadata const> {}
      ->getMetadata(coll);
      cet::sort_all(coll);

      // Check that 'file_type' and 'process_name' were automatically
      // inserted.  Then remove so that we can check the pairs that
      // were inserted by us.
      using value_type = decltype(coll)::value_type;
      auto file_type_it = cet::find_in_all(
        coll, value_type{"file_type", cet::canonical_string("unknown")});

      auto const& process_name =
        art::ServiceHandle<art::TriggerNamesService const>
      {}
      ->getProcessName();
      auto process_name_it = cet::find_in_all(
        coll, value_type{"process_name", cet::canonical_string(process_name)});

      BOOST_REQUIRE(file_type_it != coll.cend());
      BOOST_REQUIRE(process_name_it != coll.cend());
      coll.erase(file_type_it);
      coll.erase(process_name_it);

      auto const& mockMD = mockMetadata();
      BOOST_CHECK_EQUAL(coll.size(), mockMD.size());
      for (auto c = coll.cbegin(), m = mockMD.cbegin(); c != coll.cend();
           ++c, ++m) {
        BOOST_CHECK_EQUAL(c->first, m->first);
        BOOST_CHECK_EQUAL(c->second, cet::canonical_string(m->second));
      }
    }
  };
}

DEFINE_ART_MODULE(SAMMetadataConcurrentTest)
