#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Persistency/Common/Ptr.h"
#include "art/Utilities/ensurePointer.h"
#include "fhiclcpp/ParameterSet.h"
#include "test/TestObjects/ToyProducts.h"

#include <list>
#include <vector>


namespace arttest {
  class PtrListAnalyzer : public art::EDAnalyzer {
  public:
    explicit PtrListAnalyzer(fhicl::ParameterSet const & pset);
    virtual ~PtrListAnalyzer();
    virtual void analyze(art::Event const & ev);
  private:
    // input_t is the type of the product we expect to obtain from the
    // Event
    typedef std::vector<int>  input_t;

    void test_fill_list(art::Event const &);
    void test_fill_vector(art::Event const &);

    std::string input_label_;
    unsigned num_expected_;
  };


  PtrListAnalyzer:: ~PtrListAnalyzer()
  { }

  PtrListAnalyzer::PtrListAnalyzer(fhicl::ParameterSet const & pset) :
    input_label_(pset.get<std::string>("input_label")),
    num_expected_(pset.get<unsigned>("nvalues"))
  { }

  void PtrListAnalyzer::analyze(art::Event const & ev)
  {
    art::Handle<input_t>      h;
    ev.getByLabel(input_label_, h);
    assert(h.isValid());
    assert(h->size() == num_expected_);
    this->test_fill_list(ev);
    this->test_fill_vector(ev);
  }

  void PtrListAnalyzer::test_fill_list(art::Event const & ev)
  {
    // This is how to fill a list of Ptr<T> from a Handle<T>.
    art::Handle<input_t>      h;
    ev.getByLabel(input_label_, h);
    assert(h.isValid());
    std::list<art::Ptr<int> > ptrs;
    art::fill_ptr_list(ptrs, h);
    assert(ptrs.size() == num_expected_);
    int expected_value = ev.id().event();
    for (std::list<art::Ptr<int> >::const_iterator
         i = ptrs.begin(),
         e = ptrs.end();
         i != e; ++i) {
      assert(**i == expected_value);
      ++expected_value;
    }
    // Basic test of ensurePointer for Ptrs.
    art::ensurePointer<int const *>(ptrs.begin());
    std::list<art::Ptr<int> > const & ptrscref(ptrs);
    art::ensurePointer<int const *>(ptrscref.begin());
  }

  void PtrListAnalyzer::test_fill_vector(art::Event const & ev)
  {
    // This is how to fill a vector of Ptr<T> from a Handle<T>.
    art::Handle<input_t>      h;
    ev.getByLabel(input_label_, h);
    assert(h.isValid());
    std::vector<art::Ptr<int> > ptrs;
    art::fill_ptr_vector(ptrs, h);
    assert(ptrs.size() == num_expected_);
    int expected_value = ev.id().event();
    for (std::vector<art::Ptr<int> >::const_iterator
         i = ptrs.begin(),
         e = ptrs.end();
         i != e; ++i) {
      assert(**i == expected_value);
      ++expected_value;
    }
  }


}

DEFINE_ART_MODULE(arttest::PtrListAnalyzer);
