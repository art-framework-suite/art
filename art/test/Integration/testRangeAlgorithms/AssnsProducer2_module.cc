////////////////////////////////////////////////////////////////////////
// Class:       AssnsProducer2
// Plugin Type: producer (art v2_05_00)
// File:        AssnsProducer2_module.cc
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
  class AssnsProducer2;
}

using arttest::AssnsProducer2;

class arttest::AssnsProducer2 : public art::EDProducer {
public:
  explicit AssnsProducer2(fhicl::ParameterSet const& p);

  // Plugins should not be copied or assigned.
  AssnsProducer2(AssnsProducer2 const&) = delete;
  AssnsProducer2(AssnsProducer2&&) = delete;
  AssnsProducer2& operator=(AssnsProducer2 const&) = delete;
  AssnsProducer2& operator=(AssnsProducer2&&) = delete;

private:
  void produce(art::Event& e) override;

  std::string fInputLabel;
};

AssnsProducer2::AssnsProducer2(fhicl::ParameterSet const& p)
  : fInputLabel(p.get<std::string>("input_label"))
{
  produces<art::Assns<int, std::string>>();
}

void
AssnsProducer2::produce(art::Event& e)
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

DEFINE_ART_MODULE(AssnsProducer2)
