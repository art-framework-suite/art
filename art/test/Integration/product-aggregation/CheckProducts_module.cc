#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/test/Integration/product-aggregation/CalibConstants.h"
#include "art/test/Integration/product-aggregation/Fraction.h"
#include "art/test/Integration/product-aggregation/Geometry.h"
#include "art/test/Integration/product-aggregation/TrackEfficiency.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "cetlib/quiet_unit_test.hpp"
#include "fhiclcpp/types/TupleAs.h"

using art::EventRange;
using art::InputTag;
using art::RangeSet;
using arttest::Fraction;
using fhicl::Name;
using fhicl::Sequence;
using fhicl::TupleAs;
using std::string;
using std::vector;

namespace {

  double constexpr tolerance = std::numeric_limits<double>::epsilon();

  struct Config {
    TupleAs<InputTag(string)> npotsTag{Name("npotsTag")};
    TupleAs<InputTag(string)> nParticlesTag{Name("nParticlesTag")};
    TupleAs<InputTag(string)> geomTag{Name("geomTag")};
    TupleAs<InputTag(string)> calibTag{Name("calibTag")};
    TupleAs<InputTag(string)> trkEffTag{Name("trkEffTag")};
    Sequence<TupleAs<Fraction(Sequence<unsigned>, Sequence<unsigned>)>> fracs{
      Name("trkEffs")};
  };

  class CheckProducts : public art::EDAnalyzer {
  public:
    using Parameters = EDAnalyzer::Table<Config>;
    explicit CheckProducts(Parameters const& config);

    void beginRun(art::Run const& r) override;
    void beginSubRun(art::SubRun const& r) override;
    void
    analyze(art::Event const&) override
    {}

  private:
    art::InputTag potsTag_;
    art::InputTag nParticlesTag_;
    art::InputTag geomTag_;
    art::InputTag calibTag_;
    art::InputTag trkEffTag_;
    vector<Fraction> trkFracs_;

  }; // CheckProducts

  CheckProducts::CheckProducts(Parameters const& config)
    : EDAnalyzer{config}
    , potsTag_{config().npotsTag()}
    , nParticlesTag_{config().nParticlesTag()}
    , geomTag_{config().geomTag()}
    , calibTag_{config().calibTag()}
    , trkEffTag_{config().trkEffTag()}
    , trkFracs_{config().fracs()}
  {}

  void
  CheckProducts::beginRun(art::Run const& r)
  {
    // Geometry check
    auto const& geomH = r.getValidHandle<arttest::Geometry>(geomTag_);
    auto const& geomRef = RangeSet::forRun(r.id());
    BOOST_CHECK(
      art::same_ranges(geomH.provenance()->rangeOfValidity(), geomRef));
    BOOST_CHECK_EQUAL(geomH->run(), r.run());

    // nPOTs check
    RangeSet nPotsRef{r.run()};
    nPotsRef.emplace_range(0, 1, 101);
    nPotsRef.emplace_range(1, 1, 101);
    nPotsRef.collapse();
    auto const& npotsH = r.getValidHandle<unsigned>(potsTag_);
    BOOST_CHECK(
      art::same_ranges(npotsH.provenance()->rangeOfValidity(), nPotsRef));
    BOOST_CHECK_EQUAL(*npotsH, 200u);
  }

  void
  CheckProducts::beginSubRun(art::SubRun const& sr)
  {
    // CalibConstants check
    auto const& calRef = RangeSet::forSubRun(sr.id());
    auto const& calH = sr.getValidHandle<arttest::CalibConstants>(calibTag_);
    BOOST_CHECK(art::same_ranges(calH.provenance()->rangeOfValidity(), calRef));

    // TrackEfficiency check
    RangeSet trkEffRef{sr.run()};
    trkEffRef.emplace_range(sr.subRun(), 1, 101);
    auto const& trkEffH =
      sr.getValidHandle<arttest::TrackEfficiency>(trkEffTag_);
    BOOST_CHECK(
      art::same_ranges(trkEffH.provenance()->rangeOfValidity(), trkEffRef));

    double const expected{trkFracs_.at(sr.subRun()).value()};
    BOOST_CHECK_CLOSE_FRACTION(trkEffH->efficiency(), expected, tolerance);

    // nParticles and TrackEfficiency RangeSet check
    auto const& nParticlesH = sr.getValidHandle<unsigned>(nParticlesTag_);
    BOOST_CHECK(art::same_ranges(trkEffH, nParticlesH));
  }

} // namespace

DEFINE_ART_MODULE(CheckProducts)
