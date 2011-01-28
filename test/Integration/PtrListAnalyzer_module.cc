#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Persistency/Common/Handle.h"
#include "art/Persistency/Common/Ptr.h"
#include "fhiclcpp/ParameterSet.h"
#include "test/TestObjects/ToyProducts.h"

#include <list>
#include <vector>


// namespace art
// {  
//   template <class ELEMENT, class COLLECTION>
//   void
//   fill_ptr_list(std::list<art::Ptr<ELEMENT> >& ptrs, 
// 		art::Handle<COLLECTION> const& h)
//   {
//     for (size_t i = 0; i != h->size(); ++i)
//       ptrs.push_back(art::Ptr<ELEMENT>(h, i));
//   }
// }

namespace arttest
{
  class PtrListAnalyzer : public art::EDAnalyzer
  {
  public:
    explicit PtrListAnalyzer(fhicl::ParameterSet const& pset);
    virtual ~PtrListAnalyzer();
    virtual void analyze(art::Event const& ev);
  private:
    std::string input_label_;
    int num_expected_;
  };

  
  PtrListAnalyzer:: ~PtrListAnalyzer()
  { }

  PtrListAnalyzer::PtrListAnalyzer(fhicl::ParameterSet const& pset) :
    input_label_(pset.get<std::string>("input_label")),
    num_expected_(pset.get<int>("nvalues"))
  { }

  void PtrListAnalyzer::analyze(art::Event const& ev)
  {
    typedef std::vector<int>  input_t;
    art::Handle<input_t>      h;
    ev.getByLabel(input_label_, h);
    assert(h.isValid());
    assert(h->size() == num_expected_);


    // This is how to fill a list of Ptr<T> from a Handle<T>.
    std::list<art::Ptr<int> > ptrs;
    art::fill_ptr_list(ptrs, h);


    assert(ptrs.size() == num_expected_);
    int expected_value = ev.id().event();
    for (std::list<art::Ptr<int> >::const_iterator 
	   i = ptrs.begin(),
	   e = ptrs.end();
	 i != e; ++i)
      {
	assert( **i == expected_value);
	++expected_value;
      }        
  }
}

DEFINE_ART_MODULE(arttest::PtrListAnalyzer);
