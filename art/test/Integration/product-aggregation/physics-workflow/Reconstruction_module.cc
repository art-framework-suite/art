// vim: set sw=2 expandtab :
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/SharedFilter.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/test/Integration/product-aggregation/physics-workflow/CalibConstants.h"
#include "art/test/Integration/product-aggregation/physics-workflow/TrackEfficiency.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/Name.h"
#include "fhiclcpp/types/Table.h"
#include "fhiclcpp/types/TupleAs.h"
#include "hep_concurrency/tsan.h"

#include <atomic>
#include <memory>
#include <string>
#include <vector>

#include <unistd.h>

using namespace art;
using namespace std;

namespace arttest {

  struct Config {
    fhicl::TupleAs<InputTag(string)> inputTag{fhicl::Name("inputTag")};
    fhicl::Atom<double> threshold{fhicl::Name("energyThreshold")};
  };

  class Reconstruction : public SharedFilter {
  public:
    using Parameters = Table<Config>;
    explicit Reconstruction(Parameters const&);

  private:
    bool beginSubRun(SubRun&, Services const&) override;
    bool endSubRun(SubRun&, Services const&) override;
    bool filter(Event&, Services const&) override;

    double const threshold_;
    ProductToken<vector<double>> const particleEnergiesTkn_;
    std::atomic<unsigned> numerator_;
    std::atomic<unsigned> denominator_;
  };

  Reconstruction::Reconstruction(Parameters const& config)
    : SharedFilter{config}
    , threshold_{config().threshold()}
    , particleEnergiesTkn_{consumes<vector<double>>(config().inputTag())}
  {
    // We must initialize these here because the thread sanitizer does not
    // handle ctor initializers properly.
    numerator_ = 0u;
    denominator_ = 0u;
    produces<arttest::CalibConstants, InSubRun>("CalibConstants");
    produces<arttest::TrackEfficiency, InSubRun>("TrackEfficiency");
  }

  bool
  Reconstruction::beginSubRun(SubRun& sr, Services const&)
  {
    sr.put(make_unique<arttest::CalibConstants>(sr.subRun()),
           "CalibConstants",
           fullSubRun());
    return true;
  }

  bool
  Reconstruction::endSubRun(SubRun& sr, Services const&)
  {
    sr.put(make_unique<arttest::TrackEfficiency>(numerator_.load(),
                                                 denominator_.load()),
           "TrackEfficiency",
           subRunFragment());
    numerator_ = 0u;
    denominator_ = 0u;
    return true;
  }

  bool
  Reconstruction::filter(Event& e, Services const&)
  {
    auto const& particleEnergies = *e.getValidHandle(particleEnergiesTkn_);
    bool pass = false;
    for (auto const& val : particleEnergies) {
      if (val >= threshold_) {
        pass = true;
        ++numerator_;
        break;
      }
    }
    ++denominator_;
    return pass;
  }

} // namespace arttest

DEFINE_ART_MODULE(arttest::Reconstruction)
