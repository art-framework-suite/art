//--------------------------------------------------------------------
//
// Analyzes products inserted by ToyProductProducer
//   - tests products with identical names in different branch types
//
//--------------------------------------------------------------------

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/test/TestObjects/ToyProducts.h"
#include "fhiclcpp/types/Atom.h"

namespace arttest {

  class ToyProductAnalyzer : public art::EDAnalyzer {
    struct Config {
      fhicl::Atom<std::string> inputLabel {fhicl::Name{"inputLabel"}};
    };
  public:

    using Parameters = art::EDAnalyzer::Table<Config>;
    explicit ToyProductAnalyzer(Parameters const& pset);

  private:

    void beginRun(art::Run const& r) override;
    void endRun(art::Run const& r) override;
    void beginSubRun(art::SubRun const& sr) override;
    void endSubRun(art::SubRun const& sr) override;
    void analyze(art::Event const& e) override;

    std::string const inputLabel_;
    // Event token
    art::ProductToken<StringProduct> eToken_;
    // SubRun tokens
    art::ProductToken<StringProduct> srToken_;
    art::ProductToken<StringProduct> bsrToken_;
    art::ProductToken<StringProduct> esrToken_;
    // Run tokens
    art::ProductToken<StringProduct> rToken_;
    art::ProductToken<StringProduct> brToken_;
    art::ProductToken<StringProduct> erToken_;
  };
}

arttest::ToyProductAnalyzer::ToyProductAnalyzer(Parameters const& pset) :
  art::EDAnalyzer{pset},
  inputLabel_{pset().inputLabel()},
  eToken_{consumes<StringProduct>(inputLabel_)},
  srToken_{consumes<StringProduct, art::InSubRun>(inputLabel_)},
  bsrToken_{consumes<StringProduct, art::InSubRun>({inputLabel_, "bgnSubRun"})},
  esrToken_{consumes<StringProduct, art::InSubRun>({inputLabel_, "endSubRun"})},
  rToken_{consumes<StringProduct, art::InRun>(inputLabel_)},
  brToken_{consumes<StringProduct, art::InRun>({inputLabel_, "bgnRun"})},
  erToken_{consumes<StringProduct, art::InRun>({inputLabel_, "endRun"})}
{
}


void
arttest::ToyProductAnalyzer::beginRun(art::Run const& r)
{
  r.getValidHandle(brToken_);
  r.getValidHandle(rToken_);
}

void
arttest::ToyProductAnalyzer::beginSubRun(art::SubRun const& sr)
{
  sr.getValidHandle(bsrToken_);
  sr.getValidHandle(srToken_);
}

void
arttest::ToyProductAnalyzer::analyze(art::Event const& e)
{
  e.getValidHandle(eToken_);
}

void
arttest::ToyProductAnalyzer::endSubRun(art::SubRun const& sr)
{
  sr.getValidHandle(esrToken_);
}

void
arttest::ToyProductAnalyzer::endRun(art::Run const& r)
{
  r.getValidHandle(erToken_);
}

DEFINE_ART_MODULE(arttest::ToyProductAnalyzer)
