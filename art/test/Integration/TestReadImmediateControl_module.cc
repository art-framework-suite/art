#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Modules/ProvenanceDumper.h"

#include "cetlib/quiet_unit_test.hpp"

namespace arttest {
  class RICDetail;

  using TestReadImmediateControl = art::ProvenanceDumper<RICDetail>;
} // namespace arttest

class arttest::RICDetail {
public:
  struct Config {
    fhicl::Atom<bool> expectRunProducts{fhicl::Name("expectRunProducts"), true};
    fhicl::Atom<bool> expectSubRunProducts{fhicl::Name("expectSubRunProducts"),
                                           true};
    fhicl::Atom<bool> expectEventProducts{fhicl::Name("expectEventProducts"),
                                          false};
  };

  explicit RICDetail(fhicl::TableFragment<Config> const& ps);

  void preProcessRun();
  void processRunProvenance(art::Provenance const&);
  void postProcessRun();

  void preProcessSubRun();
  void processSubRunProvenance(art::Provenance const&);
  void postProcessSubRun();

  void preProcessEvent();
  void processEventProvenance(art::Provenance const&);
  void postProcessEvent();

private:
  bool const expectRunProducts_;
  bool const expectSubRunProducts_;
  bool const expectEventProducts_;

  std::size_t presentRunProducts_{0ull};
  std::size_t presentSubRunProducts_{0ull};
  std::size_t presentEventProducts_{0ull};
};

arttest::RICDetail::RICDetail(fhicl::TableFragment<Config> const& ps)
  : expectRunProducts_(ps().expectRunProducts())
  , expectSubRunProducts_(ps().expectSubRunProducts())
  , expectEventProducts_(ps().expectEventProducts())
{}

void
arttest::RICDetail::preProcessRun()
{
  presentRunProducts_ = 0ull;
}

void
arttest::RICDetail::processRunProvenance(art::Provenance const&)
{
  ++presentRunProducts_;
}

void
arttest::RICDetail::postProcessRun()
{
  if (expectRunProducts_) {
    BOOST_CHECK_GT(presentRunProducts_, 0ull);
  } else {
    BOOST_CHECK_EQUAL(presentRunProducts_, 0ull);
  }
}

void
arttest::RICDetail::preProcessSubRun()
{
  presentSubRunProducts_ = 0ull;
}

void
arttest::RICDetail::processSubRunProvenance(art::Provenance const&)
{
  ++presentSubRunProducts_;
}

void
arttest::RICDetail::postProcessSubRun()
{
  if (expectSubRunProducts_) {
    BOOST_CHECK_GT(presentSubRunProducts_, 0ull);
  } else {
    BOOST_CHECK_EQUAL(presentSubRunProducts_, 0ull);
  }
}

void
arttest::RICDetail::preProcessEvent()
{
  presentEventProducts_ = 0ull;
}

void
arttest::RICDetail::processEventProvenance(art::Provenance const&)
{
  ++presentEventProducts_;
}

void
arttest::RICDetail::postProcessEvent()
{
  if (expectEventProducts_) {
    BOOST_CHECK_GT(presentEventProducts_, 0ull);
  } else {
    BOOST_CHECK_EQUAL(presentEventProducts_, 0ull);
  }
}

DEFINE_ART_MODULE(arttest::TestReadImmediateControl)
