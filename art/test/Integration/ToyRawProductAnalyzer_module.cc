#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"

#include <cassert>
#include <iostream>

namespace arttest {
  class ToyRawProductAnalyzer;
}

using namespace fhicl;

class arttest::ToyRawProductAnalyzer : public art::EDAnalyzer {
public:
  struct Config {
    Atom<bool> beginRun{Name{"beginRun"}, true};
    Atom<bool> beginSubRun{Name{"beginSubRun"}, true};
  };
  using Parameters = Table<Config>;
  explicit ToyRawProductAnalyzer(Parameters const& p);

private:
  void analyze(art::Event const& e) override;
  void beginRun(art::Run const& r) override;
  void beginSubRun(art::SubRun const& sr) override;

  bool const doBeginRun_;
  bool const doBeginSubRun_;
};

arttest::ToyRawProductAnalyzer::ToyRawProductAnalyzer(Parameters const& p)
  : art::EDAnalyzer{p}
  , doBeginRun_{p().beginRun()}
  , doBeginSubRun_{p().beginSubRun()}
{}

void
arttest::ToyRawProductAnalyzer::analyze(art::Event const& e)
{
  e.getRun(); // Will throw if subRun or run are unavailable.
  std::vector<art::Handle<int>> hv;
  e.getManyByType(hv);
  assert(hv.size() == 1u);
  art::Handle<int>& h = hv[0];
  std::cerr << e.id() << " int = " << (*h) << "\n";
  art::Handle<bool> hb1, hb2;
  assert(e.getByLabel("m2", "a", hb1));
  std::cerr << e.id() << " bool a = " << (*hb1) << "\n";
  assert(e.getByLabel(art::InputTag("m2", "b"), hb2));
  std::cerr << e.id() << " bool b = " << (*hb2) << "\n";
  auto ph = e.getValidHandle<art::Ptr<double>>("m3");
  assert(**ph == 3.7);
}

void
arttest::ToyRawProductAnalyzer::beginRun(art::Run const& r)
{
  if (!doBeginRun_)
    return;
  std::vector<art::Handle<double>> hv;
  r.getManyByType(hv);
  assert(hv.size() == 1u);
  art::Handle<double>& h = hv[0];
  std::cerr << r.id() << " double = " << (*h) << "\n";
}

void
arttest::ToyRawProductAnalyzer::beginSubRun(art::SubRun const& sr)
{
  if (!doBeginSubRun_)
    return;
  sr.getRun(); // Will throw if not available.
  std::vector<art::Handle<double>> hv;
  sr.getManyByType(hv);
  assert(hv.size() == 1u);
  art::Handle<double>& h = hv[0];
  std::cerr << sr.id() << " double = " << (*h) << "\n";
}

DEFINE_ART_MODULE(arttest::ToyRawProductAnalyzer)
