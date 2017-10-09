//--------------------------------------------------------------------
//
// Main motivation for RunProductFilter is to test product
// aggregation.
//
//--------------------------------------------------------------------

#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/test/Integration/product-aggregation/CalibConstants.h"
#include "art/test/Integration/product-aggregation/TrackEfficiency.h"
#include "fhiclcpp/types/TupleAs.h"

namespace {

  struct Config {
    fhicl::TupleAs<art::InputTag(std::string)> inputTag{
      fhicl::Name("inputTag")};
    fhicl::Atom<double> threshold{fhicl::Name("energyThreshold")};
  };

  class Reconstruction : public art::EDFilter {
    double threshold_;
    art::ProductToken<std::vector<double>> particleEnergiesTkn_;
    unsigned numerator_{};
    unsigned denominator_{};

  public:
    using Parameters = EDFilter::Table<Config>;
    explicit Reconstruction(Parameters const& config)
      : threshold_{config().threshold()}
      , particleEnergiesTkn_{consumes<std::vector<double>>(config().inputTag())}
    {
      produces<arttest::CalibConstants, art::InSubRun>("CalibConstants");
      produces<arttest::TrackEfficiency, art::InSubRun>("TrackEfficiency");
    }

    bool
    beginSubRun(art::SubRun& sr) override
    {
      sr.put(std::make_unique<arttest::CalibConstants>(sr.subRun()),
             "CalibConstants",
             art::fullSubRun());
      return true;
    }

    bool
    filter(art::Event& e) override
    {
      auto const& particleEnergies = *e.getValidHandle(particleEnergiesTkn_);
      bool const pass = std::any_of(
        particleEnergies.cbegin(),
        particleEnergies.cend(),
        [this](double const energy) { return energy >= threshold_; });
      numerator_ += static_cast<unsigned>(pass);
      ++denominator_;
      return pass;
    }

    bool
    endSubRun(art::SubRun& sr) override
    {
      sr.put(
        std::make_unique<arttest::TrackEfficiency>(numerator_, denominator_),
        "TrackEfficiency",
        art::subRunFragment());
      denominator_ = numerator_ = 0u;
      return true;
    }

  }; // Reconstruction

} // namespace

DEFINE_ART_MODULE(Reconstruction)
