#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/SummedValue.h"
#include "art/test/Integration/product-aggregation/CalibConstants.h"
#include "art/test/Integration/product-aggregation/Fraction.h"
#include "art/test/Integration/product-aggregation/Geometry.h"
#include "art/test/Integration/product-aggregation/TrackEfficiency.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "cetlib/quiet_unit_test.hpp"
#include "fhiclcpp/types/Sequence.h"
#include "fhiclcpp/types/TupleAs.h"

using art::EventRange;
using art::InputTag;
using art::RangeSet;
using art::SummedValue;
using arttest::Fraction;
using arttest::TrackEfficiency;
using fhicl::Name;
using fhicl::Sequence;
using fhicl::TupleAs;
using std::string;
using std::vector;

namespace {

  double constexpr tolerance = std::numeric_limits<double>::epsilon();

  struct Config {
    TupleAs<InputTag(string)> trkEffTag{Name("trkEffTag")};
    TupleAs<InputTag(string)> nParticlesTag{Name("nParticlesTag")};
    TupleAs<InputTag(string)> seenParticlesTag{Name("seenParticlesTag")};
    Sequence<TupleAs<Fraction(Sequence<unsigned>, Sequence<unsigned>)>> fracs{
      Name("trkEffs")};
    Sequence<double> particleRatios{Name("particleRatios")};
  };

  class AssembleMoreProducts : public art::EDProducer {
  public:
    using Parameters = EDProducer::Table<Config>;
    explicit AssembleMoreProducts(Parameters const& config);

  private:
    void beginSubRun(art::SubRun& sr) override;
    void endSubRun(art::SubRun& sr) override;
    void
    produce(art::Event&) override
    {}

    art::InputTag trkEffTag_;
    art::InputTag nParticlesTag_;
    art::InputTag seenParticlesTag_;
    SummedValue<unsigned> trkEffNum_{};
    SummedValue<unsigned> trkEffDenom_{};
    SummedValue<unsigned> nParticles_{};
    SummedValue<unsigned> seenParticles_{};
    vector<double> seenParticleRatios_;
    vector<Fraction> expectedEffs_;
    vector<double> fullExpTrkEffs_;

  }; // AssembleMoreProducts

  AssembleMoreProducts::AssembleMoreProducts(Parameters const& config)
    : trkEffTag_{config().trkEffTag()}
    , nParticlesTag_{config().nParticlesTag()}
    , seenParticlesTag_{config().seenParticlesTag()}
    , seenParticleRatios_{config().particleRatios()}
    , expectedEffs_{config().fracs()}
    , fullExpTrkEffs_{expectedEffs_.at(0).value(), expectedEffs_.at(1).value()}
  {
    consumes<arttest::TrackEfficiency, art::InSubRun>(trkEffTag_);
    consumes<unsigned, art::InSubRun>(nParticlesTag_);
    consumes<unsigned, art::InSubRun>(seenParticlesTag_);
    produces<double, art::InSubRun>("TrkEffValue");
    produces<double, art::InSubRun>("ParticleRatio");
  }

  void
  AssembleMoreProducts::beginSubRun(art::SubRun& sr)
  {
    // TrackEfficiency check
    RangeSet trkEffRef{sr.run()};
    trkEffRef.emplace_range(sr.subRun(), 1, 101);

    auto const srn = sr.subRun();

    auto const& h = sr.getValidHandle<arttest::TrackEfficiency>(trkEffTag_);
    auto& frac = expectedEffs_.at(srn);
    BOOST_CHECK_CLOSE_FRACTION(frac.front(), h->efficiency(), tolerance);
    frac.pop_front();

    trkEffNum_.update(h, h->num());
    trkEffDenom_.update(h, h->denom());

    if (art::same_ranges(trkEffNum_, trkEffDenom_) &&
        art::same_ranges(trkEffRef, trkEffNum_.rangeOfValidity())) {
      BOOST_CHECK(!art::disjoint_ranges(trkEffNum_, trkEffDenom_));
      BOOST_CHECK(!art::overlapping_ranges(trkEffNum_, trkEffDenom_));
      auto const eff =
        static_cast<double>(trkEffNum_.value()) / trkEffDenom_.value();
      BOOST_CHECK_CLOSE_FRACTION(fullExpTrkEffs_.at(srn), eff, tolerance);
      sr.put(std::make_unique<double>(eff),
             "TrkEffValue",
             art::subRunFragment(trkEffRef));
      trkEffNum_.clear();
      trkEffDenom_.clear();
    }
  }

  void
  AssembleMoreProducts::endSubRun(art::SubRun& sr)
  {
    // nParticles produced at first stage 'eventGen'. Use getByLabel
    // because the product is likely split across multiple files.
    art::Handle<unsigned> nParticlesH;
    if (sr.getByLabel(nParticlesTag_, nParticlesH)) {
      nParticles_.update(nParticlesH);
    }

    // seenParticles produced in this process
    auto const& seenParticlesH = sr.getValidHandle<unsigned>(seenParticlesTag_);
    seenParticles_.update(seenParticlesH);

    if (art::same_ranges(nParticles_, seenParticles_)) {
      auto const ratio =
        static_cast<double>(seenParticles_.value()) / nParticles_.value();
      BOOST_CHECK_CLOSE_FRACTION(
        seenParticleRatios_.at(sr.subRun()), ratio, 0.01); // 1%
      sr.put(std::make_unique<double>(ratio),
             "ParticleRatio",
             art::subRunFragment(nParticles_.rangeOfValidity()));
      nParticles_.clear();
      seenParticles_.clear();
    }
  }
}

DEFINE_ART_MODULE(AssembleMoreProducts)
