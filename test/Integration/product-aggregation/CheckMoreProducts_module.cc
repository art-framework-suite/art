#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "cetlib/quiet_unit_test.hpp"
#include "fhiclcpp/types/TupleAs.h"
#include "test/Integration/product-aggregation/CalibConstants.h"
#include "test/Integration/product-aggregation/Fraction.h"
#include "test/Integration/product-aggregation/Geometry.h"
#include "test/Integration/product-aggregation/TrackEfficiency.h"

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

  double constexpr tolerance  = std::numeric_limits<double>::epsilon();

  struct Config {
    TupleAs<InputTag(string)> trkEffTag { Name("trkEffTag") };
    TupleAs<InputTag(string)> trkEffValueTag { Name("trkEffValueTag") };
    TupleAs<InputTag(string)> particleRatioTag { Name("particleRatioTag") };
    Sequence<double> expParticleRatios { Name("expParticleRatios") };
  };

  class CheckMoreProducts : public art::EDAnalyzer {
  public:

    using Parameters = EDAnalyzer::Table<Config>;
    explicit CheckMoreProducts(Parameters const& config);

    void beginSubRun(art::SubRun const& r) override;
    void analyze(art::Event const&) override {}

  private:

    art::InputTag trkEffTag_;
    art::InputTag trkEffValueTag_;
    art::InputTag particleRatioTag_;
    vector<double> expParticleRatios_;

  };  // CheckMoreProducts


  CheckMoreProducts::CheckMoreProducts(Parameters const& config)
    : EDAnalyzer{config}
    , trkEffTag_{config().trkEffTag()}
    , trkEffValueTag_{config().trkEffValueTag()}
    , particleRatioTag_{config().particleRatioTag()}
    , expParticleRatios_{config().expParticleRatios()}
  {}

  void
  CheckMoreProducts::beginSubRun(art::SubRun const& sr)
  {
    // User-assembled TrackEfficiency check
    auto const& trkEffH = sr.getValidHandle<arttest::TrackEfficiency>(trkEffTag_);
    auto const& trkEffValueH = sr.getValidHandle<double>(trkEffValueTag_);
    BOOST_CHECK(art::same_ranges(trkEffH, trkEffValueH));
    BOOST_CHECK_CLOSE_FRACTION(trkEffH->efficiency(), *trkEffValueH, tolerance);

    // User-assembled ParticleRatio check
    auto const& particleRatioH = sr.getValidHandle<double>(particleRatioTag_);
    BOOST_CHECK_CLOSE_FRACTION(*particleRatioH,
                               expParticleRatios_.at(sr.subRun()),
                               0.01); // 1% tolerance
    BOOST_CHECK(art::same_ranges(particleRatioH, trkEffValueH));
  }

}

DEFINE_ART_MODULE(CheckMoreProducts)
