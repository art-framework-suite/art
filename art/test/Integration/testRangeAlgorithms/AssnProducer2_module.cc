////////////////////////////////////////////////////////////////////////
// Class:       AssnProducer2
// Plugin Type: producer (art v2_05_00)
// File:        AssnProducer2_module.cc
//
// Generated at Tue Dec 13 14:04:05 2016 by Saba Sehrish using cetskelgen
// from cetlib version v1_21_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Persistency/Common/PtrMaker.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

namespace arttest {
  class AssnProducer2;
}

using arttest::AssnProducer2;

class arttest::AssnProducer2 : public art::EDProducer {
public:
  explicit AssnProducer2(fhicl::ParameterSet const& p);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  AssnProducer2(AssnProducer2 const&) = delete;
  AssnProducer2(AssnProducer2&&) = delete;
  AssnProducer2& operator=(AssnProducer2 const&) = delete;
  AssnProducer2& operator=(AssnProducer2&&) = delete;

  // Required functions.
  void produce(art::Event& e) override;

private:
  std::string fInputLabel;
  // Declare member data here.
};

AssnProducer2::AssnProducer2(fhicl::ParameterSet const& p)
  : fInputLabel(p.get<std::string>("input_label"))
// Initialize member data here.
{
  // Call appropriate produces<>() functions here.
  produces<art::Assns<int, std::string>>();
}

void
AssnProducer2::produce(art::Event& e)
{
  art::Handle<std::vector<int>> ih;
  e.getByLabel(fInputLabel, ih);
  art::Handle<std::vector<std::string>> sh;
  e.getByLabel(fInputLabel, sh);

  art::PtrMaker<int> make_intptr(e, ih.id());
  art::PtrMaker<std::string> make_strptr(e, sh.id());

  auto assns = std::make_unique<art::Assns<int, std::string>>();
  for (size_t i = 0; i < 3; ++i) {
    auto p1 = make_intptr(i);
    for (size_t j = 0; j < 2; ++j) {
      auto p2 = make_strptr(i * 2 + j);
      assns->addSingle(p1, p2);
    }
  }
  e.put(std::move(assns));
}

DEFINE_ART_MODULE(AssnProducer2)
