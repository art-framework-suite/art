#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Modules/ProvenanceDumper.h"
#include "cetlib/container_algorithms.h"

#include "cetlib/quiet_unit_test.hpp"

#include <iostream>
#include <set>

namespace arttest {
  class PDDetail;

  using TestProvenanceDumper = art::ProvenanceDumper<PDDetail>;
}

class arttest::PDDetail {
public:
  // The configuration here MUST be called 'Config'
  struct Config {
    fhicl::Atom<bool> expectSubRunProducts{fhicl::Name("expectSubRunProducts"),
                                           false};
    fhicl::Atom<bool> expectRunProducts{fhicl::Name("expectRunProducts"),
                                        false};
  };

  explicit PDDetail(fhicl::TableFragment<Config> const& ps)
    : nExpected_(9u + ps().expectSubRunProducts() + ps().expectRunProducts())
    , functionsCalled_()
  {}

  ~PDDetail()
  {
    // Required to be sure that we actually did the main check!
    BOOST_CHECK(functionsCalled_.find("endJob") != functionsCalled_.end());
  }

  void
  beginJob()
  {
    functionsCalled_.insert("beginJob");
  }

  void
  endJob()
  {
    functionsCalled_.insert("endJob");
    cet::copy_all(functionsCalled_,
                  std::ostream_iterator<std::string>(std::cout, "\n"));
    BOOST_REQUIRE_EQUAL(functionsCalled_.size(), nExpected_);
  }

  void
  preProcessEvent()
  {
    functionsCalled_.insert("preProcessEvent");
  }

  void
  processEventProvenance(art::Provenance const& prov)
  {
    functionsCalled_.insert("processEventProvenance");
    std::cout << prov << std::endl;
  }

  void
  postProcessEvent()
  {
    functionsCalled_.insert("postProcessEvent");
  }

  void
  preProcessSubRun()
  {
    functionsCalled_.insert("preProcessSubRun");
  }

  void
  processSubRunProvenance(art::Provenance const& prov)
  {
    functionsCalled_.insert("processSubRunProvenance");
    std::cout << prov << std::endl;
  }

  void
  postProcessSubRun()
  {
    functionsCalled_.insert("postProcessSubRun");
  }

  void
  preProcessRun()
  {
    functionsCalled_.insert("preProcessRun");
  }

  void
  processRunProvenance(art::Provenance const& prov)
  {
    functionsCalled_.insert("processRunProvenance");
    std::cout << prov << std::endl;
  }

  void
  postProcessRun()
  {
    functionsCalled_.insert("postProcessRun");
  }

private:
  size_t nExpected_;
  std::set<std::string> functionsCalled_;
};

DEFINE_ART_MODULE(arttest::TestProvenanceDumper)
