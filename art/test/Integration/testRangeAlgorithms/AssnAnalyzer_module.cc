////////////////////////////////////////////////////////////////////////
// Class:       AssnAnalyzer
// Plugin Type: analyzer (art v2_05_00)
// File:        AssnAnalyzer_module.cc
//
// Generated at Fri Dec  9 00:12:59 2016 by Saba Sehrish using cetskelgen
// from cetlib version v1_21_00.
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
#include "canvas/Utilities/RangeAlgorithms.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"


#include <set>
#include <algorithm> // std::copy()
#include <iterator> // std::inserter()

class AssnAnalyzer;


class AssnAnalyzer : public art::EDAnalyzer {
public:
  typedef  std::vector<int>             intvec_t;
  typedef  std::vector<std::string>     strvec_t;
  typedef  art::Assns<std::string, int> strintAssns_t;
   
  explicit AssnAnalyzer(fhicl::ParameterSet const & p);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  AssnAnalyzer(AssnAnalyzer const &) = delete;
  AssnAnalyzer(AssnAnalyzer &&) = delete;
  AssnAnalyzer & operator = (AssnAnalyzer const &) = delete;
  AssnAnalyzer & operator = (AssnAnalyzer &&) = delete;

  // Required functions.
  void analyze(art::Event const & e) override;

private:
   art::InputTag fInputLabel;
   
   std::set<std::string> fEnabledTests;
   
   void for_each_group_test(art::Event const & e) const;
   
};


AssnAnalyzer::AssnAnalyzer(fhicl::ParameterSet const & p)
  :
  EDAnalyzer(p)
  , fInputLabel(p.get<art::InputTag>("input_label"))
 // More initializers here.
{
  auto enableTests = p.get<std::vector<std::string>>("enableTests");
  if (enableTests.empty())
    fEnabledTests = { "forEachAssociatedGroup", "associatedGroups" };
  else {
    std::copy(enableTests.begin(), enableTests.end(),
      std::inserter(fEnabledTests, fEnabledTests.begin()));
  }
}


void AssnAnalyzer::analyze(art::Event const & e)
{
     AssnAnalyzer::for_each_group_test(e);
}

void AssnAnalyzer::for_each_group_test(art::Event const & e) const
{
   typedef typename art::Assns<int, std::string> istr_assns;
   auto const & int_to_str_assns = *e.getValidHandle<istr_assns> (fInputLabel);
   auto vs = strvec_t {"one", "one-a", "two", "two-a", "three", "three-a"};
   
   strvec_t strvec;
   auto strings = [&strvec](auto strs) {
      for(auto s=begin(strs); s!=end(strs); ++s) {
         std::cout << *s << std::flush << " \"" << **s << "\"" << std::endl;
         strvec.push_back(**s);
      }
   };
   
   art::for_each_group(int_to_str_assns,
                                   strings);
   
   //strings should be same as vs
   for(auto k=0; k<6;++k) {
      if (strvec[k] != vs[k]) {
        throw art::Exception(art::errors::LogicError)
          << "String #" << k << " expected to be '" << vs[k]
          << "', got '" << strvec[k] << "' instead!\n";
      }
   }
   
} // for_each_group_test()

DEFINE_ART_MODULE(AssnAnalyzer)
