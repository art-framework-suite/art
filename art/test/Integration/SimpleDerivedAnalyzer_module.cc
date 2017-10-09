// ======================================================================
//
// SimpleDerivedAnalyzer
//
// ======================================================================

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/View.h"
#include "art/test/TestObjects/ToyProducts.h"
#include "canvas/Persistency/Common/PtrVector.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSet.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace arttest {
  class SimpleDerivedAnalyzer;
}

using arttest::SimpleDerivedAnalyzer;

template <typename T>
void
check_for_conversion(art::View<T> const& v)
{
  assert(v.vals().size() > 0);
}

//--------------------------------------------------------------------
//
// Produces a SimpleProduct product instance.
//
class arttest::SimpleDerivedAnalyzer : public art::EDAnalyzer {
public:
  using SimpleDerivedProduct = std::vector<arttest::SimpleDerived>;

  struct Config {
    fhicl::Atom<std::string> input_label{fhicl::Name{"input_label"}};
    fhicl::Atom<std::string> input_label2{fhicl::Name{"input_label2"}};
    fhicl::Atom<std::size_t> nvalues{fhicl::Name{"nvalues"}};
  };
  using Parameters = art::EDAnalyzer::Table<Config>;
  explicit SimpleDerivedAnalyzer(Parameters const& p);

  void analyze(art::Event const& e) override;

private:
  void test_getView(art::Event const& e) const;
  void test_getViewReturnFalse(art::Event const& e) const;
  void test_getViewThrowing(art::Event const& e) const;

  void test_PtrVector(art::Event const& e) const;

  static constexpr std::size_t
  expectedSize()
  {
    return 16;
  }

  template <typename DATA_TYPE>
  void test_PtrVector(art::PtrVector<DATA_TYPE> const& v,
                      art::EventNumber_t event_num,
                      std::size_t nElements = expectedSize()) const;

  std::string inputLabel_;
  std::string inputLabel2_;
  std::size_t nvalues_;

}; // SimpleDerivedAnalyzer

SimpleDerivedAnalyzer::SimpleDerivedAnalyzer(Parameters const& p)
  : art::EDAnalyzer{p}
  , inputLabel_{p().input_label()}
  , inputLabel2_{p().input_label2()}
  , nvalues_{p().nvalues()}
{}

void
SimpleDerivedAnalyzer::analyze(art::Event const& e)
{
  test_getView(e);
  // TODO: uncomment this when getView is modified to return false
  // upon not finding the right label.
  //  test_getViewReturnFalse(e);
  test_getViewThrowing(e);
  test_PtrVector(e);
  art::View<arttest::SimpleDerived> v;
  art::View<arttest::Simple> vB;
  art::InputTag tag(inputLabel_, "derived", "DEVEL");
  e.getView(tag, v);
  assert(v.isValid());
  check_for_conversion(v);
  e.getView(tag, vB);
  assert(vB.isValid());
  check_for_conversion(vB);
}

template <class T>
void
verify_elements(std::vector<T> const& ptrs,
                std::size_t sz,
                art::EventNumber_t event_num,
                std::size_t /*nvalues*/)
{
  for (std::size_t k = 0; k != sz; ++k) {
    assert((unsigned)ptrs[k]->key == sz - k + event_num);
    double expect = 1.5 * k + 100.0;
    assert(ptrs[k]->value == expect);
    assert(ptrs[k]->dummy() == 16.25);
  }
}

template <class T>
void
test_view(art::Event const& e,
          std::string const& inputLabel,
          std::size_t const nvalues)
{
  auto const event_num = e.id().event();
  art::InputTag tag(inputLabel, "derived", "DEVEL");
  std::vector<T const*> ptrs;
  auto sz = e.getView(inputLabel, "derived", ptrs);
  assert(sz == nvalues);
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
  for (std::size_t i = 0, sz = pvec.size(); i != sz; ++i) {
    assert(*pvec[i] == *v.vals()[i + 1]);
  }
}

// dummy is a type we can be sure is not used in any collections in
// the Event; no dictionary exists for it.
struct dummy {
};

void
SimpleDerivedAnalyzer::test_getView(art::Event const& e) const
{
  // Make sure we can get views into products that are present.
  test_view<Simple>(e, inputLabel_, nvalues_);
  test_view<SimpleDerived>(e, inputLabel_, nvalues_);
} // test_getView()

void
SimpleDerivedAnalyzer::test_getViewReturnFalse(art::Event const& e) const
{
  std::vector<int const*> ints;
  try {
    assert(e.getView("nothing with this illegal label", ints) == false);
  }
  catch (std::exception& e) {
    std::cerr << e.what() << '\n';
    assert("Unexpected exception thrown" == 0);
  }
  catch (...) {
    assert("Unexpected exception thrown" == 0);
  }
}

void
SimpleDerivedAnalyzer::test_getViewThrowing(art::Event const& e) const
{
  //  Make sure attempts to get views into products that are not there
  //  fail correctly.
  std::vector<dummy const*> dummies;
  try {
    e.getView(inputLabel_, dummies);
    assert("Failed to throw required exception" == 0);
  }
  catch (art::Exception& e) {
    assert(e.categoryCode() == art::errors::ProductNotFound);
  }
  catch (...) {
    assert("Wrong exception thrown" == 0);
  }
}

void
SimpleDerivedAnalyzer::test_PtrVector(art::Event const& e) const
{
  auto const event_num = e.id().event();
  using product_t = art::PtrVector<arttest::SimpleDerived>;
  using base_product_t = art::PtrVector<arttest::Simple>;

  // Read the data.
  auto const& d = e.getByLabel<product_t>(art::InputTag{inputLabel2_});
  auto const sz = d.size();
  if (sz != expectedSize()) {
    throw cet::exception("SizeMismatch")
      << "Expected a PtrVector of size " << expectedSize()
      << " but the obtained size is " << sz << '\n';
  }
  // Test the data
  test_PtrVector(d, event_num);
  // Construct from PtrVector<U>
  {
    base_product_t s(d);
    test_PtrVector(s, event_num);
    product_t p(s);
    test_PtrVector(p, event_num);
  }
  // Construct from initializer list.
  {
    auto i(d.cbegin());
    auto il = {*(i++), *i};
    base_product_t s(il);
    test_PtrVector(s, event_num, 2);
    product_t p({s.front(), s.back()});
    test_PtrVector(p, event_num, 2);
  }
  // Operator= from PtrVector<U>.
  {
    base_product_t s;
    s = d;
    test_PtrVector(s, event_num);
    product_t p;
    p = s;
    test_PtrVector(p, event_num);
  }
  // Operator= from initializer list.
  {
    auto i(d.cbegin());
    base_product_t s;
    s = {*(i++), *i};
    test_PtrVector(s, event_num, 2);
    product_t p;
    p = {s.front(), s.back()};
    test_PtrVector(p, event_num, 2);
  }
  // Assign from Ptr<U>.
  {
    base_product_t s(d);
    s.assign(1, d.front());
    test_PtrVector(s, event_num, 1);
    product_t p(s);
    p.assign(1, s.front());
    test_PtrVector(p, event_num, 1);
  }
  // Assign from iterators.
  {
    base_product_t s(d);
    s.assign(d.cbegin(), d.cend());
    test_PtrVector(s, event_num);
    product_t p(s);
    p.assign(s.cbegin(), s.cend());
    test_PtrVector(p, event_num);
  }
  // Assign from initializer list.
  {
    auto i(d.cbegin());
    base_product_t s(d);
    s.assign({*(i++), *i});
    test_PtrVector(s, event_num, 2);
    product_t p(s);
    p.assign({s.front(), s.back()});
    test_PtrVector(p, event_num, 2);
  }
  // Push back.
  {
    base_product_t s;
    s.push_back(d.front());
    test_PtrVector(s, event_num, 1);
    product_t p;
    p.push_back(s.front());
    test_PtrVector(p, event_num, 1);
  }
  // Insert from Ptr<U>.
  {
    base_product_t s({d[1]});
    s.insert(s.begin(), d.front());
    test_PtrVector(s, event_num, 2);
    base_product_t p({s.back()});
    p.insert(p.begin(), s.front());
    test_PtrVector(p, event_num, 2);
  }
  // Insert from iterators.
  {
    base_product_t s;
    s.insert(s.begin(), d.cbegin(), d.cend());
    test_PtrVector(s, event_num);
    product_t p;
    p.insert(p.begin(), s.cbegin(), s.cend());
    test_PtrVector(p, event_num);
  }
  // Erase.
  {
    base_product_t s(d);
    s.erase(s.end() - 1);
    test_PtrVector(s, event_num, expectedSize() - 1);
    s.erase(s.begin() + 1, s.end());
    test_PtrVector(s, event_num, 1);
  }
}

template <typename DATA_TYPE>
void
SimpleDerivedAnalyzer::test_PtrVector(art::PtrVector<DATA_TYPE> const& v,
                                      art::EventNumber_t const event_num,
                                      std::size_t const nElements) const
{
  auto const sz(v.size());
  if (sz != nElements) {
    throw cet::exception("SizeMismatch")
      << "Expected size " << nElements << " but got " << sz << ".\n";
  }
  auto const b = v.cbegin();
  auto const e = v.cbegin() + nElements;
  for (auto i = b; i != e; ++i) {
    std::size_t const k = i - b;
    if (static_cast<std::size_t>((*i)->key) != expectedSize() - k + event_num) {
      throw cet::exception("KeyMismatch")
        << "At position " << k << " expected key "
        << expectedSize() - k + event_num << " but obtained " << (*i)->key
        << ".\n";
    }
    assert(*i == *i); // Check operator ==.
    if (k == 0 && sz > 1) {
      assert(*i != *(i + 1));  // Check operator !=.
      assert(*(i) < *(i + 1)); // Check operator <.
    }

    double const expect{1.5 * k + 100.0};
    if ((*i)->value != expect) {
      throw cet::exception("ValueMismatch")
        << "At position " << k << " expected value " << expect
        << " but obtained " << (*i)->value << ".\n";
    }
    if ((*i)->dummy() != 16.25) {
      throw cet::exception("ValueMismatch")
        << "At position " << k << " expected dummy value " << 16.25
        << " but obtained " << (*i)->dummy() << ".\n";
    }
  }
} // test_PtrVector()

DEFINE_ART_MODULE(SimpleDerivedAnalyzer)
