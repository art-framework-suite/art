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
#include "art/Persistency/Common/Assns.h"
#include "art/Persistency/Common/Ptr.h"
#include "test/TestObjects/AssnTestData.h"

#include <string>
#include <vector>

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
  typedef art::Assns<size_t, std::string, arttest::AssnTestData> Assns_t;
}

arttest::AssnsProducer::AssnsProducer(fhicl::ParameterSet const &p)
{
  produces<std::vector<size_t> >();
  produces<std::vector<std::string> >();
  produces<Assns_t>();
}

arttest::AssnsProducer::~AssnsProducer() {
}

void arttest::AssnsProducer::produce(art::Event &e) {
  std::auto_ptr<std::vector<size_t> > vui(new std::vector<size_t>);
  vui->reserve(3);
  vui->push_back(2);
  vui->push_back(0);
  vui->push_back(1);

  std::auto_ptr<std::vector<std::string> > vs(new std::vector<std::string>);
  vs->reserve(3);
  vs->push_back("one");
  vs->push_back("two");
  vs->push_back("zero");

  std::auto_ptr<Assns_t> a(new Assns_t);
  art::ProductID vui_pid = getProductID<std::vector<size_t> >(e);
  art::ProductID vs_pid = getProductID<std::vector<std::string> >(e);
  a->addSingle(art::Ptr<size_t>(vui_pid, 1, e.productGetter()),
               art::Ptr<std::string>(vs_pid, 2, e.productGetter()),
               AssnTestData(1, 2, "A"));
  a->addSingle(art::Ptr<size_t>(vui_pid, 2, e.productGetter()),
               art::Ptr<std::string>(vs_pid, 0, e.productGetter()),
               AssnTestData(2, 0, "B"));
  a->addSingle(art::Ptr<size_t>(vui_pid, 0, e.productGetter()),
               art::Ptr<std::string>(vs_pid, 1, e.productGetter()),
               AssnTestData(0, 1, "C"));

  e.put(vui);
  e.put(vs);
  e.put(a);

}

DEFINE_ART_MODULE(arttest::AssnsProducer);
