// =================================================================
// Whereas AssembleMoreProducts tested the basic functionality of
// SummedValue, this module shows the caching the users must perform
// whenever the input files are presented in out-of-(sub)run order.
// =================================================================

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

#include <map>
#include <vector>

using art::EventRange;
using art::InputTag;
using art::RangeSet;
using art::SubRunNumber_t;
using art::SummedValue;
using arttest::Fraction;
using arttest::TrackEfficiency;
using fhicl::Atom;
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
    Atom<double> trkEff{Name("trkEffPerSubRun")};
  };

  class AssembleMoreProductsRobust : public art::EDProducer {
  public:
    using Parameters = EDProducer::Table<Config>;
    explicit AssembleMoreProductsRobust(Parameters const& config);

    void beginSubRun(art::SubRun& sr) override;
    void endSubRun(art::SubRun& sr) override;
    void
    produce(art::Event&) override
    {}

  private:
    art::InputTag const trkEffTag_;
    double const expTrkEff_;
    art::InputTag const nParticlesTag_;
    art::InputTag const seenParticlesTag_;
    std::map<SubRunNumber_t, SummedValue<unsigned>> trkEffNum_{};
    std::map<SubRunNumber_t, SummedValue<unsigned>> trkEffDenom_{};
    std::map<SubRunNumber_t, SummedValue<unsigned>> nParticles_{};
    std::map<SubRunNumber_t, SummedValue<unsigned>> seenParticles_{};

  }; // AssembleMoreProductsRobust

  AssembleMoreProductsRobust::AssembleMoreProductsRobust(
    Parameters const& config)
    : trkEffTag_{config().trkEffTag()}
    , expTrkEff_{config().trkEff()}
    , nParticlesTag_{config().nParticlesTag()}
    , seenParticlesTag_{config().seenParticlesTag()}
  {
    produces<Fraction, art::InSubRun>("TrkEffValue");
    produces<Fraction, art::InSubRun>("ParticleRatio");
  }

  void
  AssembleMoreProductsRobust::beginSubRun(art::SubRun& sr)
  {
    // TrackEfficiency check
    RangeSet trkEffRef{sr.run()};
    trkEffRef.emplace_range(sr.subRun(), 1, 101);

    auto const srn = sr.subRun();

    auto& trkEffNum = trkEffNum_[srn];
    auto& trkEffDenom = trkEffDenom_[srn];
    auto const& h = sr.getValidHandle<arttest::TrackEfficiency>(trkEffTag_);
    trkEffNum.update(h, h->num());
    trkEffDenom.update(h, h->denom());

    if (art::same_ranges(trkEffNum, trkEffDenom) &&
        art::same_ranges(trkEffRef, trkEffNum.rangeOfValidity())) {
      BOOST_CHECK(!art::disjoint_ranges(trkEffNum, trkEffDenom));
      BOOST_CHECK(!art::overlapping_ranges(trkEffNum, trkEffDenom));
      auto trkEff =
        std::make_unique<Fraction>(trkEffNum.value(), trkEffDenom.value());
      BOOST_CHECK_CLOSE_FRACTION(expTrkEff_, trkEff->value(), tolerance);
      sr.put(move(trkEff), "TrkEffValue", art::subRunFragment(trkEffRef));
      trkEffNum.clear();
      trkEffDenom.clear();
    }
  }

  void
  AssembleMoreProductsRobust::endSubRun(art::SubRun& sr)
  {
    SubRunNumber_t const srn{sr.subRun()};
    auto& nParticles = nParticles_[srn];
    auto& seenParticles = seenParticles_[srn];
    // nParticles produced at first stage 'eventGen'. Use getByLabel
    // because the product is likely split across multiple files.
    art::Handle<unsigned> nParticlesH;
    if (sr.getByLabel(nParticlesTag_, nParticlesH)) {
      nParticles.update(nParticlesH);
    }

    // seenParticles produced in this process
    auto const& seenParticlesH = sr.getValidHandle<unsigned>(seenParticlesTag_);
    seenParticles.update(seenParticlesH);

    if (art::same_ranges(nParticles, seenParticles)) {
      sr.put(
        std::make_unique<Fraction>(seenParticles.value(), nParticles.value()),
        "ParticleRatio",
        art::subRunFragment(nParticles.rangeOfValidity()));
      nParticles.clear();
      seenParticles.clear();
    }
  }
}

DEFINE_ART_MODULE(AssembleMoreProductsRobust)
