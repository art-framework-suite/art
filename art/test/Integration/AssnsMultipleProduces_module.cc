////////////////////////////////////////////////////////////////////////
// Class:       AssnsMultipleProduces
// Module Type: producer
// File:        AssnsMultipleProduces_module.cc
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "canvas/Persistency/Common/Assns.h"

namespace art {
  namespace test {
    class AssnsMultipleProduces;
  }
} // namespace art

class art::test::AssnsMultipleProduces : public EDProducer {
public:
  struct Config {};
  using Parameters = EDProducer::Table<Config>;
  explicit AssnsMultipleProduces(Parameters const&);
  void produce(art::Event&) override{};
};

using std::size_t;
using std::string;

art::test::AssnsMultipleProduces::AssnsMultipleProduces(Parameters const&)
{
  produces<Assns<size_t, string>>();
  produces<Assns<string, size_t>>();
}

DEFINE_ART_MODULE(art::test::AssnsMultipleProduces)
