////////////////////////////////////////////////////////////////////////
// Class:       AssnsWrongPut
// Module Type: producer
// File:        AssnsWrongPut_module.cc
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "canvas/Persistency/Common/Assns.h"

namespace art {
  namespace test {
    class AssnsWrongPut;
  }
} // namespace art

class art::test::AssnsWrongPut : public EDProducer {
public:
  struct Config {
  };
  using Parameters = EDProducer::Table<Config>;
  explicit AssnsWrongPut(Parameters const&);

private:
  void produce(art::Event&) override;
};

using std::size_t;
using std::string;

art::test::AssnsWrongPut::AssnsWrongPut(Parameters const& ps) : EDProducer{ps}
{
  produces<Assns<size_t, string>>();
}

void
art::test::AssnsWrongPut::produce(Event& e)
{
  // Put in an Assns with reversed template arguments
  e.put(std::make_unique<Assns<string, size_t>>());
}

DEFINE_ART_MODULE(art::test::AssnsWrongPut)
