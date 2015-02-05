
/*----------------------------------------------------------------------

Toy EDProducers and EDProducts for testing purposes only.

----------------------------------------------------------------------*/

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Persistency/Common/EDProduct.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/View.h"
#include "fhiclcpp/ParameterSet.h"
#include "test/TestObjects/ToyProducts.h"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

extern "C"
{
  int a_trick_for_toys = 0;
}

namespace arttest {

  //--------------------------------------------------------------------
  //
  // Toy producers
  //
  //--------------------------------------------------------------------

  //--------------------------------------------------------------------
  //
  // throws an exception.
  // Announces an IntProduct but does not produce one;
  // every call to FailingProducer::produce throws a cms exception
  //
  class FailingProducer : public art::EDProducer {
  public:
    explicit FailingProducer(fhicl::ParameterSet const& /*p*/) {
      produces<IntProduct>();
    }
    virtual ~FailingProducer() { }
    virtual void produce(art::Event& e, art::EventSetup const& c);
  };

  void
  FailingProducer::produce(art::Event&, art::EventSetup const&) {
    // We throw an edm exception with a configurable action.
    throw art::Exception(art::errors::NotFound) << "Intentional 'NotFound' exception for testing purposes\n";
  }

  //--------------------------------------------------------------------
  //
  // Announces an IntProduct but does not produce one;
  // every call to NonProducer::produce does nothing.
  //
  class NonProducer : public art::EDProducer {
  public:
    explicit NonProducer(fhicl::ParameterSet const& /*p*/) {
      produces<IntProduct>();
    }
    virtual ~NonProducer() { }
    virtual void produce(art::Event& e, art::EventSetup const& c);
  };

  void
  NonProducer::produce(art::Event&, art::EventSetup const&) {
  }

  //--------------------------------------------------------------------
  //
  // Produces an Int16_tProduct instance.
  //
  class Int16_tProducer : public art::EDProducer {
  public:
    explicit Int16_tProducer(fhicl::ParameterSet const& p) :
      value_(p.get<int>("ivalue")) {
      produces<Int16_tProduct>();
    }
    explicit Int16_tProducer(std::int16_t i, std::uint16_t j) : value_(i), uvalue_(j) {
      produces<Int16_tProduct>();
    }
    virtual ~Int16_tProducer() { }
    virtual void produce(art::Event& e, art::EventSetup const& c);

  private:
    std::int16_t value_;
    std::uint16_t uvalue_;
  };

  void
  Int16_tProducer::produce(art::Event& e, art::EventSetup const&) {
    // EventSetup is not used.
    std::unique_ptr<Int16_tProduct> p(new Int16_tProduct(value_));
    e.put(std::move(p));
  }

  //--------------------------------------------------------------------
  //
  // Produces an DoubleProduct instance.
  //

  class ToyDoubleProducer : public art::EDProducer {
  public:
    explicit ToyDoubleProducer(fhicl::ParameterSet const& p) :
      value_(p.get<double>("dvalue")) {
      produces<DoubleProduct>();
    }
    explicit ToyDoubleProducer(double d) : value_(d) {
      produces<DoubleProduct>();
    }
    virtual ~ToyDoubleProducer() { }
    virtual void produce(art::Event& e, art::EventSetup const& c);
  private:
    double value_;
  };

  void
  ToyDoubleProducer::produce(art::Event& e, art::EventSetup const&) {

    // Make output
    std::unique_ptr<DoubleProduct> p(new DoubleProduct(value_));
    e.put(std::move(p));
  }

  //--------------------------------------------------------------------
  //
  // Produces an IntProduct instance, using an IntProduct as input.
  //

  class AddIntsProducer : public art::EDProducer {
  public:
    explicit AddIntsProducer(fhicl::ParameterSet const& p) :
      labels_(p.get<std::vector<std::string> >("labels")) {
        produces<IntProduct>();
      }
    virtual ~AddIntsProducer() { }
    virtual void produce(art::Event& e, art::EventSetup const& c);
  private:
    std::vector<std::string> labels_;
  };

  void
  AddIntsProducer::produce(art::Event& e, art::EventSetup const&) {
    // EventSetup is not used.
    int value = 0;
    for(std::vector<std::string>::iterator itLabel = labels_.begin(), itLabelEnd = labels_.end();
        itLabel != itLabelEnd; ++itLabel) {
      art::Handle<IntProduct> anInt;
      e.getByLabel(*itLabel, anInt);
      value +=anInt->value;
    }
    std::unique_ptr<IntProduct> p(new IntProduct(value));
    e.put(std::move(p));
  }

  //--------------------------------------------------------------------
  //
  // Produces an std::vector<int> instance.
  //
  class IntVectorProducer : public art::EDProducer {
  public:
    explicit IntVectorProducer(fhicl::ParameterSet const& p) :
      value_(p.get<int>("ivalue")),
      count_(p.get<int>("count"))
    {
      produces<std::vector<int> >();
    }
    virtual ~IntVectorProducer() { }
    virtual void produce(art::Event& e, art::EventSetup const& c);
  private:
    int    value_;
    size_t count_;
  };

  void
  IntVectorProducer::produce(art::Event& e, art::EventSetup const&) {
    // EventSetup is not used.
    std::unique_ptr<std::vector<int> > p(new std::vector<int>(count_, value_));
    e.put(std::move(p));
  }

  //--------------------------------------------------------------------
  //
  // Produces an std::list<int> instance.
  //
  class IntListProducer : public art::EDProducer {
  public:
    explicit IntListProducer(fhicl::ParameterSet const& p) :
      value_(p.get<int>("ivalue")),
      count_(p.get<int>("count"))
    {
      produces<std::list<int> >();
    }
    virtual ~IntListProducer() { }
    virtual void produce(art::Event& e, art::EventSetup const& c);
  private:
    int    value_;
    size_t count_;
  };

  void
  IntListProducer::produce(art::Event& e, art::EventSetup const&) {
    // EventSetup is not used.
    std::unique_ptr<std::list<int> > p(new std::list<int>(count_, value_));
    e.put(std::move(p));
  }

  //--------------------------------------------------------------------
  //
  // Produces an std::deque<int> instance.
  //
  class IntDequeProducer : public art::EDProducer {
  public:
    explicit IntDequeProducer(fhicl::ParameterSet const& p) :
      value_(p.get<int>("ivalue")),
      count_(p.get<int>("count"))
    {
      produces<std::deque<int> >();
    }
    virtual ~IntDequeProducer() { }
    virtual void produce(art::Event& e, art::EventSetup const& c);
  private:
    int    value_;
    size_t count_;
  };

  void
  IntDequeProducer::produce(art::Event& e, art::EventSetup const&) {
    // EventSetup is not used.
    std::unique_ptr<std::deque<int> > p(new std::deque<int>(count_, value_));
    e.put(std::move(p));
  }

  //--------------------------------------------------------------------
  //
  // Produces an std::set<int> instance.
  //
  class IntSetProducer : public art::EDProducer {
  public:
    explicit IntSetProducer(fhicl::ParameterSet const& p) :
      start_(p.get<int>("start")),
      stop_(p.get<int>("stop"))
    {
      produces<std::set<int> >();
    }
    virtual ~IntSetProducer() { }
    virtual void produce(art::Event& e, art::EventSetup const& c);
  private:
    int start_;
    int stop_;
  };

  void
  IntSetProducer::produce(art::Event& e, art::EventSetup const&) {
    // EventSetup is not used.
    std::unique_ptr<std::set<int> > p(new std::set<int>());
    for (int i = start_; i < stop_; ++i) p->insert(i);
    e.put(std::move(p));
  }

  //--------------------------------------------------------------------
  //
  // Produces an Prodigal instance.
  //
  class ProdigalProducer : public art::EDProducer {
  public:
    explicit ProdigalProducer(fhicl::ParameterSet const& p) :
      label_(p.get<std::string>("label")) {
      produces<Prodigal>();
    }
    virtual ~ProdigalProducer() { }
    virtual void produce(art::Event& e, art::EventSetup const& c);

  private:
    std::string label_;
  };

  void
  ProdigalProducer::produce(art::Event& e, art::EventSetup const&) {
    // EventSetup is not used.

    // The purpose of Prodigal is testing of *not* keeping
    // parentage. So we need to get a product...
    art::Handle<IntProduct> parent;
    e.getByLabel(label_, parent);

    std::unique_ptr<Prodigal> p(new Prodigal(parent->value));
    e.put(std::move(p));
  }

}

using arttest::FailingProducer;
using arttest::NonProducer;
using arttest::Int16_tProducer;
using arttest::ToyDoubleProducer;
using arttest::AddIntsProducer;
using arttest::IntVectorProducer;
using arttest::IntListProducer;
using arttest::IntDequeProducer;
using arttest::IntSetProducer;
using arttest::ProdigalProducer;
DEFINE_ART_MODULE(FailingProducer)
DEFINE_ART_MODULE(NonProducer)
DEFINE_ART_MODULE(Int16_tProducer)
DEFINE_ART_MODULE(ToyDoubleProducer)
DEFINE_ART_MODULE(AddIntsProducer)
DEFINE_ART_MODULE(IntVectorProducer)
DEFINE_ART_MODULE(IntListProducer)
DEFINE_ART_MODULE(IntDequeProducer)
DEFINE_ART_MODULE(IntSetProducer)
DEFINE_ART_MODULE(ProdigalProducer)

