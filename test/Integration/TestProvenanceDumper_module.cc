#include "art/Framework/Modules/ProvenanceDumper.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "cetlib/container_algorithms.h"

#define BOOST_TEST_DYN_LINK
#include <boost/test/included/unit_test.hpp>

#include <iostream>
#include <set>

namespace arttest {
  class PDDetail;

  typedef art::ProvenanceDumper<PDDetail> TestProvenanceDumper;
}

class arttest::PDDetail {
public:
  explicit PDDetail(fhicl::ParameterSet const & pset)
    :
    nExpected_(9u +
               pset.get<bool>("expectSubRunProducts", false) +
               pset.get<bool>("expectRunProducts", false)),
    functionsCalled_()
  { }

  ~PDDetail() {
    // Required to be sure that we actually did the main check!
    BOOST_CHECK(functionsCalled_.find("endJob") != functionsCalled_.end());
  }

  void beginJob() {
    functionsCalled_.insert("beginJob");
  }

  void endJob() {
    functionsCalled_.insert("endJob");
    cet::copy_all(functionsCalled_, std::ostream_iterator<std::string>(std::cout, "\n"));
    BOOST_REQUIRE_EQUAL(functionsCalled_.size(), nExpected_);
  }

  void preProcessEvent() {
    functionsCalled_.insert("preProcessEvent");
  }

  void processEventProvenance(art::Provenance const & prov) {
    functionsCalled_.insert("processEventProvenance");
    std::cout << prov << std::endl;
  }

  void postProcessEvent() {
    functionsCalled_.insert("postProcessEvent");
  }

  void preProcessSubRun() {
    functionsCalled_.insert("preProcessSubRun");
  }

  void processSubRunProvenance(art::Provenance const & prov) {
    functionsCalled_.insert("processSubRunProvenance");
    std::cout << prov << std::endl;
  }

  void postProcessSubRun() {
    functionsCalled_.insert("postProcessSubRun");
  }

  void preProcessRun() {
    functionsCalled_.insert("preProcessRun");
  }

  void processRunProvenance(art::Provenance const & prov) {
    functionsCalled_.insert("processRunProvenance");
    std::cout << prov << std::endl;
  }

  void postProcessRun() {
    functionsCalled_.insert("postProcessRun");
  }

private:
  size_t nExpected_;
  std::set<std::string> functionsCalled_;
};

DEFINE_ART_MODULE(arttest::TestProvenanceDumper);
