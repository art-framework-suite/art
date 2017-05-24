////////////////////////////////////////////////////////////////////////
// Class:       ConstAssnsIterAnalyzer
// File:        ConstAssnsIterAnalyzer_module.cc
//
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Persistency/Common/Assns.h"
#include "canvas/Persistency/Common/Ptr.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

//#include "artdata/Utilities/ForEachAssociatedGroup.h"
#include "canvas/Persistency/Common/AssnsIter.h"

class ConstAssnsIterAnalyzer;

using namespace art;

class ConstAssnsIterAnalyzer : public art::EDAnalyzer {
public:
  typedef  std::vector<int>             intvec_t;
  typedef  std::vector<std::string>     strvec_t;
  typedef  std::vector<float>           floatvec_t;
   
  explicit ConstAssnsIterAnalyzer(fhicl::ParameterSet const & p);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  ConstAssnsIterAnalyzer(ConstAssnsIterAnalyzer const &) = delete;
  ConstAssnsIterAnalyzer(ConstAssnsIterAnalyzer &&) = delete;
  ConstAssnsIterAnalyzer & operator = (ConstAssnsIterAnalyzer const &) = delete;
  ConstAssnsIterAnalyzer & operator = (ConstAssnsIterAnalyzer &&) = delete;

  // Required functions.
  void analyze(art::Event const & e) override;

private:
   art::InputTag fInputLabel;
   void test(art::Event const & e) const;
};


ConstAssnsIterAnalyzer::ConstAssnsIterAnalyzer(fhicl::ParameterSet const & p)
  :
  EDAnalyzer(p)
  , fInputLabel(p.get<art::InputTag>("input_label"))
 // More initializers here.
{}



void ConstAssnsIterAnalyzer::analyze(art::Event const & e)
{
   ConstAssnsIterAnalyzer::test(e);
}

void ConstAssnsIterAnalyzer::test(art::Event const & e) const 
{
  typedef typename art::Assns<int, float, std::string> assns_t;
  typedef typename art::const_AssnsIter<int, float, std::string> assnsiter_t;

   //vectors to verify values 
  auto const vi = intvec_t {1, 1, 2, 2, 3, 3};
  auto const vs = strvec_t {"one", "one-a", "two", "two-a", "three", "three-a"};
  auto const vf = floatvec_t {1.0, 1.1, 2.0, 2.1, 3.0, 3.1 };

  const assns_t assns {*e.getValidHandle<assns_t>(fInputLabel)};
    //iterator increment and dereference test
  auto my_begin = assns.begin();
  auto my_end = assns.end();
  assert(my_begin!=my_end);
  int k = 0;
  for(auto p = my_begin; p!=my_end; ++p) {
      assert(*((*p).data)==vs[k]);
      assert(*((*p).second.get())==vf[k]);
      assert(*((*p).first.get())==vi[k]);
      ++k;
  }
} 


DEFINE_ART_MODULE(ConstAssnsIterAnalyzer)
