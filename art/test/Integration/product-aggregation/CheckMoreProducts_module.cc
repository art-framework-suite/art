#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/test/Integration/product-aggregation/Fraction.h"
#include "art/test/Integration/product-aggregation/TaggedValue.h"
#include "art/test/Integration/product-aggregation/TrackEfficiency.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "cetlib/quiet_unit_test.hpp"
#include "fhiclcpp/types/TupleAs.h"

using art::EventRange;
using art::InputTag;
using art::RangeSet;
using arttest::Fraction;
using arttest::TaggedValue;
using fhicl::Name;
using fhicl::Sequence;
using fhicl::TupleAs;
using std::string;
using std::vector;

namespace {

  double constexpr tolerance = std::numeric_limits<double>::epsilon();

  struct Config {
    TupleAs<InputTag(string)> trkEffTag{Name("trkEffTag")};
    TupleAs<InputTag(string)> trkEffValueTag{Name("trkEffValueTag")};
    TupleAs<TaggedValue<unsigned>(string, unsigned)> nParticlesRef{Name("nParticlesRef")};
    TupleAs<TaggedValue<unsigned>(string, unsigned)> seenParticlesRef{Name("seenParticlesRef")};
    TupleAs<TaggedValue<double>(string, double)> particleRatioRef{Name("particleRatioRef")};
  };

  class CheckMoreProducts : public art::EDAnalyzer {
  public:
    using Parameters = EDAnalyzer::Table<Config>;
    explicit CheckMoreProducts(Parameters const& config);

    void beginSubRun(art::SubRun const& r) override;
    void
    analyze(art::Event const&) override
    {}

  private:
    art::InputTag const trkEffTag_;
    art::InputTag const trkEffValueTag_;
    TaggedValue<unsigned> const nParticlesRef_;
    TaggedValue<unsigned> const seenParticlesRef_;
    TaggedValue<double> const particleRatioRef_;

  }; // CheckMoreProducts

  CheckMoreProducts::CheckMoreProducts(Parameters const& config)
    : EDAnalyzer{config}
    , trkEffTag_{config().trkEffTag()}
    , trkEffValueTag_{config().trkEffValueTag()}
    , nParticlesRef_{config().nParticlesRef()}
    , seenParticlesRef_{config().seenParticlesRef()}
    , particleRatioRef_{config().particleRatioRef()}
  {}

  void
  CheckMoreProducts::beginSubRun(art::SubRun const& sr)
  {
    // User-assembled TrackEfficiency check
    auto const& trkEffH =
      sr.getValidHandle<arttest::TrackEfficiency>(trkEffTag_);
    auto const& trkEffValueH = sr.getValidHandle<Fraction>(trkEffValueTag_);
    BOOST_CHECK(art::same_ranges(trkEffH, trkEffValueH));
    BOOST_CHECK_CLOSE_FRACTION(trkEffH->efficiency(), trkEffValueH->value(), tolerance);

    // User-assembled ParticleRatio check
    auto const seenParticles = sr.getByLabel<unsigned>(seenParticlesRef_.tag_);
    auto const nParticles = sr.getByLabel<unsigned>(nParticlesRef_.tag_);
    auto const& particleRatioH = sr.getValidHandle<Fraction>(particleRatioRef_.tag_);
    BOOST_CHECK_EQUAL(seenParticles, seenParticlesRef_.value_);
    BOOST_CHECK_EQUAL(nParticles, nParticlesRef_.value_);
    BOOST_CHECK_CLOSE_FRACTION(particleRatioH->value(),
                               particleRatioRef_.value_,
                               0.01); // 1% tolerance
    BOOST_CHECK(art::same_ranges(particleRatioH, trkEffValueH));
    BOOST_CHECK(!art::disjoint_ranges(particleRatioH, trkEffValueH));
    BOOST_CHECK(!art::overlapping_ranges(particleRatioH, trkEffValueH));
  }

} // namespace

DEFINE_ART_MODULE(CheckMoreProducts)
