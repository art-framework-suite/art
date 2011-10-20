// ======================================================================
//
// SimpleDerivedAnalyzer
//
// ======================================================================

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/View.h"
#include "art/Persistency/Common/PtrVector.h"
#include "art/Persistency/Provenance/EventID.h"
#include "cetlib/exception.h"
#include "cpp0x/memory"
#include "fhiclcpp/ParameterSet.h"
#include "test/TestObjects/ToyProducts.h"
#include <iostream>
#include <string>
#include <vector>

namespace arttest {
  class SimpleDerivedAnalyzer;
}

using arttest::SimpleDerivedAnalyzer;

//--------------------------------------------------------------------
//
// Produces a SimpleProduct product instance.
//
class arttest::SimpleDerivedAnalyzer
  : public art::EDAnalyzer
{
public:
  typedef  std::vector<arttest::SimpleDerived>  SimpleDerivedProduct;

  explicit SimpleDerivedAnalyzer( fhicl::ParameterSet const & p )
    : inputLabel_( p.get<std::string>("input_label") )
    , inputLabel2_( p.get<std::string>("input_label2") )
    , nvalues_   ( p.get<int>("nvalues") )
  { }

  virtual ~SimpleDerivedAnalyzer() { }
  virtual void analyze( art::Event const & e )
  {
    test_getView(e);
    // TODO: uncomment this when getView is modified to return false
    // upon not finding the right label.
    //  test_getViewReturnFalse(e);
    test_getViewThrowing(e);
    test_PtrVector(e);
  }

  void test_getView( art::Event const & e ) const;
  void test_getViewReturnFalse( art::Event const& e) const;
  void test_getViewThrowing( art::Event const& e) const;
  void test_PtrVector( art::Event const & e ) const;


private:
  std::string inputLabel_;
  std::string inputLabel2_;
  unsigned    nvalues_;

};  // SimpleDerivedAnalyzer

template <class T>
void verify_elements(std::vector<T> const& ptrs,
                     size_t sz,
                     art::EventNumber_t event_num,
                     size_t /*nvalues*/)
{
  for (size_t k = 0; k != sz; ++k)
    {
      assert((unsigned)ptrs[k]->key == sz-k+event_num);
      double expect = 1.5 * k + 100.0;
      assert(ptrs[k]->value == expect);
      assert(ptrs[k]->dummy() == 16.25);
    }
}

template <class T>
void
test_view(art::Event const& e,
          std::string const& inputLabel,
          unsigned nvalues)
{
  int event_num = e.id().event();
  art::InputTag tag(inputLabel, "derived", "DEVEL");

  std::vector<T const *> ptrs;
  unsigned sz = e.getView(inputLabel, "derived", ptrs);
  assert (sz == nvalues);
  verify_elements(ptrs, sz, event_num, nvalues);

  ptrs.clear();
  sz = e.getView(tag, ptrs);
  assert(sz == nvalues);
  verify_elements(ptrs, sz, event_num, nvalues);

  art::View<T> v;
  assert(e.getView(inputLabel, "derived", v));
  assert(v.vals().size() == nvalues);
  verify_elements(v.vals(), v.vals().size(), event_num, nvalues);

  art::View<T> v2;
  assert(e.getView(tag, v2));
  assert(v2.vals().size() == nvalues);
  verify_elements(v2.vals(), v2.vals().size(), event_num, nvalues);

  // Fill a PtrVector from the view... after zeroing the first
  // element.
  v.vals().front() = 0; // zero out the first pointer...
  art::PtrVector<T> pvec;
  v.fill(pvec);
  for (size_t i = 0, sz = pvec.size(); i!=sz; ++i)
    assert(*pvec[i] == *v.vals()[i+1]);
}

// dummy is a type we can be sure is not used in any collections in
// the Event; no dictionary exists for it.
struct dummy {};

void
SimpleDerivedAnalyzer::test_getView( art::Event const & e ) const
{
  // Make sure we can get views into products that are present.
  test_view<Simple>(e, inputLabel_, nvalues_);
  test_view<SimpleDerived>(e, inputLabel_, nvalues_);
} // test_getView()

void
SimpleDerivedAnalyzer::test_getViewReturnFalse( art::Event const& e) const
{
  std::vector<int const*> ints;
  try
    {
      assert(e.getView("nothing with this illegal label", ints)==false);
    }
  catch (std::exception& e)
    {
      std::cerr << e.what() << '\n';
      assert("Unexpected exception thrown" == 0);
    }
  catch (...)
    {
      assert("Unexpected exception thrown" == 0);
    }
 }

void
SimpleDerivedAnalyzer::test_getViewThrowing( art::Event const& e) const
{
  //  Make sure attempts to get views into products that are not there
  //  fail correctly.
  std::vector<dummy const*> dummies;
  try
    {
      e.getView(inputLabel_, dummies);
      assert("Failed to throw required exception" == 0);
    }
  catch (art::Exception& e)
    {
      assert(e.categoryCode() == art::errors::DictionaryNotFound);
    }
  catch (...)
    {
      assert("Wrong exception thrown" == 0);
    }
}


void
SimpleDerivedAnalyzer::test_PtrVector( art::Event const & e ) const
{
  int event_num = e.id().event();
  {  // test using derived class
    typedef  art::PtrVector<arttest::SimpleDerived>  product_t;
    art::Handle<product_t>  h;
    e.getByLabel(inputLabel2_, h);

    unsigned sz = h->size();
    if( sz != 16 ) {
      std::cerr
        << "SizeMismatch expected a PtrVector of size " << 16
        << " but the obtained size is " << sz
        << '\n';
      throw cet::exception("SizeMismatch")
        << "Expected a PtrVector of size " << 16
        << " but the obtained size is " << sz
        << '\n';
    }

    for( product_t::const_iterator b = h->begin()
           , e = h->end(); b!= e; ++b ) {
      int k = b - h->begin();
      if( (unsigned)(*b)->key != sz-k+event_num ) {
        std::cerr
          << "ValueMismatch at position " << k
          << " expected key " << sz-k+event_num
          << " but obtained " << (*b)->key
          << '\n';
        throw cet::exception("ValueMismatch")
          << "At position " << (b - h->begin())
          << " expected key " << sz-k+event_num
          << " but obtained " << (*b)->key
          << '\n';
      }
      double expect = 1.5 * k + 100.0;
      if( (*b)->value != expect ) {
        std::cerr
          << "ValueMismatch at position " << k
          << " expected value " << expect
          << " but obtained " << (*b)->value
          << '\n';
        throw cet::exception("ValueMismatch")
          << "At position " << (b - h->begin())
          << " expected value " << expect
          << " but obtained " << (*b)->value
          << '\n';
      }
      if( (*b)->dummy() != 16.25 ) {
        std::cerr
          << "ValueMismatch at position " << k
          << " expected dummy value " << 16.25
          << " but obtained " << (*b)->dummy()
          << '\n';
        throw cet::exception("ValueMismatch")
          << "At position " << k
          << " expected dummy value " << 16.25
          << " but obtained " << (*b)->dummy()
          << '\n';
      }
    }
  }

}  // test_PtrVector()


// ----------------------------------------------------------------------

DEFINE_ART_MODULE(SimpleDerivedAnalyzer)

// ======================================================================
