////////////////////////////////////////////////////////////////////////
// Class:       AssnsProducer
// Module Type: producer
// File:        AssnsProducer_module.cc
//
// Generated at Thu Jul  7 13:34:45 2011 by Chris Green using artmod
// from art v0_07_12.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Persistency/Common/Assns.h"
#include "art/Persistency/Common/Ptr.h"
#include "test/TestObjects/AssnTestData.h"

#include <memory>
#include <string>
#include <vector>

using std::string;
using std::vector;
using std::make_unique;

using art::Ptr;

using uintvec = vector<size_t>;
using stringvec = vector<string>;

namespace arttest {
  class AssnsProducer;
}

class arttest::AssnsProducer : public art::EDProducer {
public:
  explicit AssnsProducer(fhicl::ParameterSet const &p);
  virtual ~AssnsProducer();

  virtual void produce(art::Event &e);
};

namespace {
  typedef art::Assns<size_t, string, arttest::AssnTestData> Assns_t;
  typedef art::Assns<string, size_t, arttest::AssnTestData> AssnsBA_t;
  typedef art::Assns<size_t, string> AssnsV_t;
}

arttest::AssnsProducer::AssnsProducer(fhicl::ParameterSet const &)
{
  produces<uintvec>();
  produces<stringvec>();
  produces<Assns_t>();
  produces<AssnsV_t>();
  produces<Assns_t>("M");
  produces<AssnsV_t>("M");
}

arttest::AssnsProducer::~AssnsProducer() {
}

void arttest::AssnsProducer::produce(art::Event &e) {

  auto vui = make_unique<uintvec>(uintvec { 2, 0, 1 } );
  auto vs = make_unique<stringvec>(stringvec {"one", "two", "zero"});

  std::unique_ptr<Assns_t> a(new Assns_t);
  std::unique_ptr<AssnsV_t> av(new AssnsV_t);
  art::ProductID vui_pid = getProductID<uintvec >(e);
  art::ProductID vs_pid = getProductID<stringvec >(e);
  a->addSingle(Ptr<size_t>(vui_pid, 1, e.productGetter(vui_pid)),
               Ptr<string>(vs_pid, 2, e.productGetter(vs_pid)),
               AssnTestData(1, 2, "A"));
  av->addSingle(Ptr<size_t>(vui_pid, 1, e.productGetter(vui_pid)),
                Ptr<string>(vs_pid, 2, e.productGetter(vs_pid)));
  a->addSingle(Ptr<size_t>(vui_pid, 2, e.productGetter(vui_pid)),
               Ptr<string>(vs_pid, 0, e.productGetter(vs_pid)),
               AssnTestData(2, 0, "B"));
  av->addSingle(Ptr<size_t>(vui_pid, 2, e.productGetter(vui_pid)),
                Ptr<string>(vs_pid, 0, e.productGetter(vs_pid)));
  a->addSingle(Ptr<size_t>(vui_pid, 0, e.productGetter(vui_pid)),
               Ptr<string>(vs_pid, 1, e.productGetter(vs_pid)),
               AssnTestData(0, 1, "C"));
  av->addSingle(Ptr<size_t>(vui_pid, 0, e.productGetter(vui_pid)),
                Ptr<string>(vs_pid, 1, e.productGetter(vs_pid)));

  auto am = make_unique<Assns_t>(*a);
  auto avm = make_unique<AssnsV_t>(*av);

  am->addSingle(Ptr<size_t>(vui_pid, 1, e.productGetter(vui_pid)),
                Ptr<string>(vs_pid, 2, e.productGetter(vs_pid)),
                AssnTestData(1, 2, "AA"));
  avm->addSingle(Ptr<size_t>(vui_pid, 1, e.productGetter(vui_pid)),
                 Ptr<string>(vs_pid, 2, e.productGetter(vs_pid)));

  e.put(std::move(vui));
  e.put(std::move(vs));
  e.put(std::move(a));
  e.put(std::move(av));
  e.put(std::move(am), "M");
  e.put(std::move(avm), "M");
}

DEFINE_ART_MODULE(arttest::AssnsProducer)
