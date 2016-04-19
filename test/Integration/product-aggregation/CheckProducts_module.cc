//--------------------------------------------------------------------
//
// Main motivation for RunProductAnalyzer is to test product
// aggregation.
//
//--------------------------------------------------------------------

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "cetlib/quiet_unit_test.hpp"
#include "fhiclcpp/types/TupleAs.h"
#include "test/Integration/product-aggregation/CalibConstants.h"
#include "test/Integration/product-aggregation/Geometry.h"
#include "test/Integration/product-aggregation/TrackEfficiency.h"

#include <random>

using art::EventRange;
using art::InputTag;
using art::RangeSet;
using fhicl::Name;
using fhicl::Sequence;
using fhicl::TupleAs;
using std::string;
using std::vector;

namespace {

  class Fraction {
  public:
    Fraction(vector<unsigned> const& nums,
             vector<unsigned> const& denoms)
      : nums_{nums}
      , denoms_{denoms}
    {}

    double value() const
    {
      if (denoms_.size() == 0) return 0.;
      double const num = std::accumulate(nums_.begin(), nums_.end(), 0);
      double const denom = std::accumulate(denoms_.begin(), denoms_.end(), 0);
      return denom == 0. ? 0. : num/denom;
    }

  private:
    vector<unsigned> nums_;
    vector<unsigned> denoms_;
  };

  double constexpr tolerance  = std::numeric_limits<double>::epsilon();

  struct Config {
    TupleAs<InputTag(string)> npotsTag { Name("npotsTag") };
    TupleAs<InputTag(string)> geomTag { Name("geomTag") };
    TupleAs<InputTag(string)> calibTag { Name("calibTag") };
    TupleAs<InputTag(string)> trkEffTag { Name("trkEffTag") };
    Sequence<TupleAs<Fraction(Sequence<unsigned>,Sequence<unsigned>)>> fracs { Name("trkEffs") };
  };

  class CheckProducts : public art::EDAnalyzer {
  public:

    using Parameters = EDAnalyzer::Table<Config>;
    explicit CheckProducts(Parameters const& config)
      : EDAnalyzer{config}
      , potsTag_{config().npotsTag()}
      , geomTag_{config().geomTag()}
      , calibTag_{config().calibTag()}
      , trkEffTag_{config().trkEffTag()}
      , trkFracs_{config().fracs()}
    {}

    void beginRun(art::Run const& r) override
    {
      // Geometry check
      auto const& geomH  = r.getValidHandle<arttest::Geometry>(geomTag_);
      auto const& geomRef = RangeSet::forRun(r.id());
      BOOST_CHECK_EQUAL(geomH.provenance()->rangeSet(), geomRef);
      BOOST_CHECK_EQUAL(geomH->run(), r.run());

      // nPOTs check
      RangeSet nPotsRef {r.run()};
      nPotsRef.emplace_range(0,1,101);
      nPotsRef.emplace_range(1,1,101);
      nPotsRef.collapse();
      auto const& npotsH = r.getValidHandle<unsigned>(potsTag_);
      BOOST_CHECK_EQUAL(npotsH.provenance()->rangeSet(), nPotsRef);
      BOOST_CHECK_EQUAL(*npotsH, 200u);
    }

    void beginSubRun(art::SubRun const& sr) override
    {
      // CalibConstants check
      auto const& calRef = RangeSet::forSubRun(sr.id());
      auto const& calH = sr.getValidHandle<arttest::CalibConstants>(calibTag_);
      BOOST_CHECK_EQUAL(calH.provenance()->rangeSet(), calRef);

      // TrackEfficiency check
      RangeSet trkEffRef {sr.run()};
      trkEffRef.emplace_range(sr.subRun(),1,101);
      auto const& trkEffH = sr.getValidHandle<arttest::TrackEfficiency>(trkEffTag_);
      BOOST_CHECK_EQUAL(trkEffH.provenance()->rangeSet(), trkEffRef);

      double const expected {trkFracs_.at(sr.subRun()).value()};
      BOOST_CHECK_CLOSE_FRACTION(trkEffH->efficiency(), expected, tolerance);
    }

    void analyze(art::Event const&) override {}

  private:

    art::InputTag potsTag_;
    art::InputTag geomTag_;
    art::InputTag calibTag_;
    art::InputTag trkEffTag_;
    vector<Fraction> trkFracs_;
  };  // CheckProducts

}

DEFINE_ART_MODULE(CheckProducts)
