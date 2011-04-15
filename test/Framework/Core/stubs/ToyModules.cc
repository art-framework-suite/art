
/*----------------------------------------------------------------------

Toy EDProducers and EDProducts for testing purposes only.

----------------------------------------------------------------------*/

#include <cassert>
#include <stdexcept>
#include <string>
#include <vector>

#include "art/Persistency/Common/EDProduct.h"
#include "art/Persistency/Common/Handle.h"
#include "art/Persistency/Common/Ref.h"
#include "art/Persistency/Common/View.h"
#include "art/Persistency/Common/RefVector.h"
#include "art/Persistency/Common/RefToBaseVector.h"
#include "test/TestObjects/ToyProducts.h"

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "fhiclcpp/ParameterSet.h"

#include <boost/shared_ptr.hpp>

#include <iostream>

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
    explicit Int16_tProducer(boost::int16_t i, boost::uint16_t j) : value_(i), uvalue_(j) {
      produces<Int16_tProduct>();
    }
    virtual ~Int16_tProducer() { }
    virtual void produce(art::Event& e, art::EventSetup const& c);

  private:
    boost::int16_t value_;
    boost::uint16_t uvalue_;
  };

  void
  Int16_tProducer::produce(art::Event& e, art::EventSetup const&) {
    // EventSetup is not used.
    std::auto_ptr<Int16_tProduct> p(new Int16_tProduct(value_));
    e.put(p);
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
    std::auto_ptr<DoubleProduct> p(new DoubleProduct(value_));
    e.put(p);
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
    std::auto_ptr<IntProduct> p(new IntProduct(value));
    e.put(p);
  }

  //--------------------------------------------------------------------
  //
  // Produces and SCSimpleProduct product instance.
  //
  class SCSimpleProducer : public art::EDProducer {
  public:
    explicit SCSimpleProducer(fhicl::ParameterSet const& p) :
      size_(p.get<int>("size"))
    {
      produces<SCSimpleProduct>();
      assert ( size_ > 1 );
    }

    explicit SCSimpleProducer(int i) : size_(i)
    {
      produces<SCSimpleProduct>();
      assert ( size_ > 1 );
    }

    virtual ~SCSimpleProducer() { }
    virtual void produce(art::Event& e, art::EventSetup const& c);

  private:
    int size_;  // number of Simples to put in the collection
  };

  void
  SCSimpleProducer::produce(art::Event& e,
			    art::EventSetup const& /* unused */)
  {
    // Fill up a collection so that it is sorted *backwards*.
    std::vector<Simple> guts(size_);
    for (int i = 0; i < size_; ++i)
      {
	guts[i].key = size_ - i;
	guts[i].value = 1.5 * i;
      }

    // Verify that the vector is not sorted -- in fact, it is sorted
    // backwards!
    for (int i = 1; i < size_; ++i)
      {
	assert( guts[i-1].id() > guts[i].id());
      }

    std::auto_ptr<SCSimpleProduct> p(new SCSimpleProduct(guts));

    // Put the product into the Event, thus sorting it.
    e.put(p);

  }

  //--------------------------------------------------------------------
  //
  // Produces and OVSimpleProduct product instance.
  //
  class OVSimpleProducer : public art::EDProducer {
  public:
    explicit OVSimpleProducer(fhicl::ParameterSet const& p) :
      size_(p.get<int>("size"))
    {
      produces<OVSimpleProduct>();
      produces<OVSimpleDerivedProduct>("derived");
      assert ( size_ > 1 );
    }

    explicit OVSimpleProducer(int i) : size_(i)
    {
      produces<OVSimpleProduct>();
      produces<OVSimpleDerivedProduct>("derived");
      assert ( size_ > 1 );
    }

    virtual ~OVSimpleProducer() { }
    virtual void produce(art::Event& e, art::EventSetup const& c);

  private:
    int size_;  // number of Simples to put in the collection
  };

  void
  OVSimpleProducer::produce(art::Event& e,
			    art::EventSetup const& /* unused */)
  {
    // Fill up a collection
    std::auto_ptr<OVSimpleProduct> p(new OVSimpleProduct());

    for (int i = 0; i < size_; ++i)
      {
	std::auto_ptr<Simple> simple(new Simple());
	simple->key = size_ - i;
	simple->value = 1.5 * i;
        p->push_back(simple);
      }

    // Put the product into the Event
    e.put(p);

    // Fill up a collection of SimpleDerived objects
    std::auto_ptr<OVSimpleDerivedProduct> pd(new OVSimpleDerivedProduct());

    for (int i = 0; i < size_; ++i)
      {
	std::auto_ptr<SimpleDerived> simpleDerived(new SimpleDerived());
	simpleDerived->key = size_ - i;
	simpleDerived->value = 1.5 * i + 100.0;
	simpleDerived->dummy = 0.0;
	pd->push_back(simpleDerived);
      }

    // Put the product into the Event
    e.put(pd, "derived");
  }

  //--------------------------------------------------------------------
  //
  // Produces and OVSimpleProduct product instance.
  //
  class VSimpleProducer : public art::EDProducer {
  public:
    explicit VSimpleProducer(fhicl::ParameterSet const& p) :
      size_(p.get<int>("size"))
    {
      produces<VSimpleProduct>();
      assert ( size_ > 1 );
    }

    explicit VSimpleProducer(int i) : size_(i)
    {
      produces<VSimpleProduct>();
      assert ( size_ > 1 );
    }

    virtual ~VSimpleProducer() { }
    virtual void produce(art::Event& e, art::EventSetup const& c);

  private:
    int size_;  // number of Simples to put in the collection
  };

  void
  VSimpleProducer::produce(art::Event& e,
			   art::EventSetup const& /* unused */)
  {
    // Fill up a collection
    std::auto_ptr<VSimpleProduct> p(new VSimpleProduct());

    for (int i = 0; i < size_; ++i)
      {
	Simple simple;
	simple.key = size_ - i;
	simple.value = 1.5 * i;
        p->push_back(simple);
      }

    // Put the product into the Event
    e.put(p);
  }

  //--------------------------------------------------------------------
  //
  // Produces AssociationVector<vector<Simple>, vector<Simple> > object
  // This is used to test a View of an AssociationVector
  //
  class AVSimpleProducer : public art::EDProducer {
  public:

    explicit AVSimpleProducer(fhicl::ParameterSet const& p) :
    src_(p.get<art::InputTag>("src")) {
      produces<AVSimpleProduct>();
    }

    virtual ~AVSimpleProducer() { }
    virtual void produce(art::Event& e, art::EventSetup const& c);

  private:
    art::InputTag src_;
  };

  void
  AVSimpleProducer::produce(art::Event& e,
                            art::EventSetup const& /* unused */)
  {
    art::Handle<std::vector<arttest::Simple> > vs;
    e.getByLabel( src_, vs );
    // Fill up a collection
    std::auto_ptr<AVSimpleProduct> p(new AVSimpleProduct(art::RefProd<std::vector<arttest::Simple> >(vs)));

    for (unsigned int i = 0; i < vs->size(); ++i)
      {
        arttest::Simple simple;
        simple.key = 100 + i;  // just some arbitrary number for testing
        p->setValue(i,simple);
      }

    // Put the product into the Event
    e.put(p);
  }

  //--------------------------------------------------------------------
  //
  // Produces two products:
  //    DSVSimpleProduct
  //    DSVWeirdProduct
  //
  class DSVProducer : public art::EDProducer
  {
  public:

    explicit DSVProducer(fhicl::ParameterSet const& p) :
      size_(p.get<int>("size"))
    {
      produces<DSVSimpleProduct>();
      produces<DSVWeirdProduct>();
      assert(size_ > 1);
    }

    explicit DSVProducer(int i) : size_(i)
    {
      produces<DSVSimpleProduct>();
      produces<DSVWeirdProduct>();
      assert(size_ > 1);
    }

    virtual ~DSVProducer() { }

    virtual void produce(art::Event& e, art::EventSetup const&);

  private:
    template <class PROD> void make_a_product(art::Event& e);
    int size_;
  };

  void
  DSVProducer::produce(art::Event& e,
			     art::EventSetup const& /* unused */)
  {
    this->make_a_product<DSVSimpleProduct>(e);
    this->make_a_product<DSVWeirdProduct>(e);
  }


  template <class PROD>
  void
  DSVProducer::make_a_product(art::Event& e)
  {
    typedef PROD                     product_type;
    typedef typename product_type::value_type detset;
    typedef typename detset::value_type       value_type;

    // Fill up a collection so that it is sorted *backwards*.
    std::vector<value_type> guts(size_);
    for (int i = 0; i < size_; ++i)
      {
	guts[i].data = size_ - i;
      }

    // Verify that the vector is not sorted -- in fact, it is sorted
    // backwards!
    for (int i = 1; i < size_; ++i)
      {
 	assert( guts[i-1].data > guts[i].data);
      }

    std::auto_ptr<product_type> p(new product_type());
    int n=0;
    for (int id=1;id<size_;++id) {
      ++n;
      detset item(id); // this will get DetID id
      item.data.insert(item.data.end(),guts.begin(),guts.begin()+n);
      p->insert(item);
    }

    // Put the product into the Event, thus sorting it ... or not,
    // depending upon the product type.
    e.put(p);
  }

  //--------------------------------------------------------------------
  //
  // Produces two products: (new DataSetVector)
  //    DSTVSimpleProduct
  //    DSTVSimpleDerivedProduct
  //
  class DSTVProducer : public art::EDProducer
  {
  public:

    explicit DSTVProducer(fhicl::ParameterSet const& p) :
      size_(p.get<int>("size"))
    {
      produces<DSTVSimpleProduct>();
      produces<DSTVSimpleDerivedProduct>();
      assert(size_ > 1);
    }

    explicit DSTVProducer(int i) : size_(i)
    {
      produces<DSTVSimpleProduct>();
      produces<DSTVSimpleDerivedProduct>();
      assert(size_ > 1);
    }

    virtual ~DSTVProducer() { }

    virtual void produce(art::Event& e, art::EventSetup const&);

  private:
    template <class PROD> void make_a_product(art::Event& e);
    void fill_a_data(DSTVSimpleProduct::data_type & d, int i);
    void fill_a_data(DSTVSimpleDerivedProduct::data_type & d, int i);

    int size_;
  };

  void
  DSTVProducer::produce(art::Event& e,
			art::EventSetup const& /* unused */)
  {
    this->make_a_product<DSTVSimpleProduct>(e);
    this->make_a_product<DSTVSimpleDerivedProduct>(e);
  }


  void
  DSTVProducer::fill_a_data(DSTVSimpleDerivedProduct::data_type & d, int i)
  {
    d.key = size_ - i;
    d.value = 1.5 * i;
  }

  void
  DSTVProducer::fill_a_data(DSTVSimpleProduct::data_type & d, int i)
  {
    d.data=size_ - i;
  }

  template <class PROD>
  void
  DSTVProducer::make_a_product(art::Event& e)
  {
    typedef PROD                     product_type;
    //FIXME
    typedef typename product_type::FastFiller detset;
    typedef typename detset::value_type       value_type;
    typedef typename detset::id_type       id_type;



    std::auto_ptr<product_type> p(new product_type());
    product_type & v = *p;

    int n=0;
    for (id_type id=1;id<static_cast<id_type>(size_);++id) {
      ++n;
      detset item(v,id); // this will get DetID id
      item.resize(n);
      for (int i=0;i<n;++i)
	fill_a_data(item[i],i);
    }

    // Put the product into the Event, thus sorting is not done by magic,
    // up to one user-line
    e.put(p);
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
    std::auto_ptr<std::vector<int> > p(new std::vector<int>(count_, value_));
    e.put(p);
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
    std::auto_ptr<std::list<int> > p(new std::list<int>(count_, value_));
    e.put(p);
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
    std::auto_ptr<std::deque<int> > p(new std::deque<int>(count_, value_));
    e.put(p);
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
    std::auto_ptr<std::set<int> > p(new std::set<int>());
    for (int i = start_; i < stop_; ++i) p->insert(i);
    e.put(p);
  }

  //--------------------------------------------------------------------
  //
  // Produces an art::RefVector<std::vector<int> > instance.
  // This requires that an instance of IntVectorProducer be run *before*
  // this producer.
  class IntVecRefVectorProducer : public art::EDProducer {
    typedef art::RefVector<std::vector<int> > product_type;

  public:
    explicit IntVecRefVectorProducer(fhicl::ParameterSet const& p) :
      target_(p.get<std::string>("target"))
    {
      produces<product_type>();
    }
    virtual ~IntVecRefVectorProducer() { }
    virtual void produce(art::Event& e, art::EventSetup const& c);

  private:
    std::string target_;
  };

  void
  IntVecRefVectorProducer::produce(art::Event& e, art::EventSetup const&) {
    // EventSetup is not used.
    // Get our input:
    art::Handle<std::vector<int> > input;
    e.getByLabel(target_, input);
    assert(input.isValid());

    std::auto_ptr<product_type> prod(new product_type());

    typedef product_type::value_type ref;
    for (size_t i = 0, sz =input->size(); i!=sz; ++i)
      prod->push_back(ref(input, i));

    e.put(prod);
  }

  //--------------------------------------------------------------------
  //
  // Produces an art::RefToBaseVector<int> instance.
  // This requires that an instance of IntVectorProducer be run *before*
  // this producer. The input collection is read as an art::View<int>
  class IntVecRefToBaseVectorProducer : public art::EDProducer {
    typedef art::RefToBaseVector<int> product_type;

  public:
    explicit IntVecRefToBaseVectorProducer(fhicl::ParameterSet const& p) :
      target_(p.get<std::string>("target"))
    {
      produces<product_type>();
    }
    virtual ~IntVecRefToBaseVectorProducer() { }
    virtual void produce(art::Event& e, art::EventSetup const& c);

  private:
    std::string target_;
  };

  void
  IntVecRefToBaseVectorProducer::produce(art::Event& e, art::EventSetup const&) {
    // EventSetup is not used.
    // Get our input:
    art::Handle<art::View<int> > input;
    e.getByLabel(target_, input);
    assert(input.isValid());

    std::auto_ptr<product_type> prod(new product_type( input->refVector() ));
    e.put(prod);
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

    std::auto_ptr<Prodigal> p(new Prodigal(parent->value));
    e.put(p);
  }

  //--------------------------------------------------------------------
  //
  class SCSimpleAnalyzer : public art::EDAnalyzer
  {
  public:
    SCSimpleAnalyzer(const fhicl::ParameterSet& iPSet) { }

    virtual void
    analyze(art::Event const& e, art::EventSetup const&);
  };


  void
  SCSimpleAnalyzer::analyze(art::Event const& e, art::EventSetup const&)
  {

    // Get the product back out; it should be sorted.
    std::vector< art::Handle<SCSimpleProduct> > hv;
    e.getManyByType(hv);
    assert( hv.size() == 1u );
    art::Handle<SCSimpleProduct> & h = hv[0];
    assert( h.isValid() );

    // Check the sorting. DO NOT DO THIS IN NORMAL CODE; we are
    // copying all the values out of the SortedCollection so we can
    // manipulate them via an interface different from
    // SortedCollection, just so that we can make sure the collection
    // is sorted.
    std::vector<Simple> after( h->begin(), h->end() );
    typedef std::vector<Simple>::size_type size_type;


    // Verify that the vector *is* sorted.

    for (size_type i = 1, end = after.size(); i < end; ++i)
      {
	assert( after[i-1].id() < after[i].id());
      }
  }

}

using arttest::FailingProducer;
using arttest::NonProducer;
using arttest::Int16_tProducer;
using arttest::ToyDoubleProducer;
using arttest::SCSimpleProducer;
using arttest::OVSimpleProducer;
using arttest::VSimpleProducer;
using arttest::AVSimpleProducer;
using arttest::DSTVProducer;
using arttest::DSVProducer;
using arttest::SCSimpleAnalyzer;
using arttest::DSVAnalyzer;
using arttest::AddIntsProducer;
using arttest::IntVectorProducer;
using arttest::IntListProducer;
using arttest::IntDequeProducer;
using arttest::IntSetProducer;
using arttest::IntVecRefVectorProducer;
using arttest::IntVecRefToBaseVectorProducer;
using arttest::ProdigalProducer;
DEFINE_ART_MODULE(FailingProducer);
DEFINE_ART_MODULE(NonProducer);
DEFINE_ART_MODULE(Int16_tProducer);
DEFINE_ART_MODULE(ToyDoubleProducer);
DEFINE_ART_MODULE(SCSimpleProducer);
DEFINE_ART_MODULE(OVSimpleProducer);
DEFINE_ART_MODULE(VSimpleProducer);
DEFINE_ART_MODULE(AVSimpleProducer);
DEFINE_ART_MODULE(DSVProducer);
DEFINE_ART_MODULE(DSTVProducer);
DEFINE_ART_MODULE(SCSimpleAnalyzer);
DEFINE_ART_MODULE(DSVAnalyzer);
DEFINE_ART_MODULE(AddIntsProducer);
DEFINE_ART_MODULE(IntVectorProducer);
DEFINE_ART_MODULE(IntListProducer);
DEFINE_ART_MODULE(IntDequeProducer);
DEFINE_ART_MODULE(IntSetProducer);
DEFINE_ART_MODULE(IntVecRefVectorProducer);
DEFINE_ART_MODULE(IntVecRefToBaseVectorProducer);
DEFINE_ART_MODULE(ProdigalProducer);

