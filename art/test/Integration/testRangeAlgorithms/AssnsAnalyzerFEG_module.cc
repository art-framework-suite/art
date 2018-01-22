////////////////////////////////////////////////////////////////////////
// Class:       AssnsAnalyzerFEG
// Plugin Type: analyzer (art v2_05_00)
// File:        AssnsAnalyzerFEG_module.cc
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
#include "canvas/Persistency/Common/AssnsAlgorithms.h"
#include "canvas/Persistency/Common/Ptr.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <algorithm> // std::copy()
#include <iterator>  // std::inserter()
#include <set>

class AssnsAnalyzerFEG;

class AssnsAnalyzerFEG : public art::EDAnalyzer {
public:
  typedef std::vector<int> intvec_t;
  typedef std::vector<std::string> strvec_t;
  typedef art::Assns<std::string, int> strintAssns_t;

  explicit AssnsAnalyzerFEG(fhicl::ParameterSet const& p);

  // Plugins should not be copied or assigned.
  AssnsAnalyzerFEG(AssnsAnalyzerFEG const&) = delete;
  AssnsAnalyzerFEG(AssnsAnalyzerFEG&&) = delete;
  AssnsAnalyzerFEG& operator=(AssnsAnalyzerFEG const&) = delete;
  AssnsAnalyzerFEG& operator=(AssnsAnalyzerFEG&&) = delete;

private:
  void analyze(art::Event const& e) override;
  void for_each_group_test(art::Event const& e) const;
  art::InputTag fInputLabel;
};

AssnsAnalyzerFEG::AssnsAnalyzerFEG(fhicl::ParameterSet const& p)
  : EDAnalyzer(p), fInputLabel(p.get<art::InputTag>("input_label"))
// More initializers here.
{}

void
AssnsAnalyzerFEG::analyze(art::Event const& e)
{
  AssnsAnalyzerFEG::for_each_group_test(e);
}

void
AssnsAnalyzerFEG::for_each_group_test(art::Event const& e) const
{
  typedef art::Assns<int, std::string> istr_assns;
  auto const& int_to_str_assns = *e.getValidHandle<istr_assns>(fInputLabel);
  auto vs = strvec_t{"one", "one-a", "two", "two-a", "three", "three-a"};

  strvec_t strvec;
  auto strings = [&strvec](auto strs) {
    for (auto s = begin(strs); s != end(strs); ++s) {
      std::cout << *s << std::flush << " \"" << **s << "\"" << std::endl;
      strvec.push_back(**s);
    }
  };

  art::for_each_group(int_to_str_assns, strings);

  // strings should be same as vs
  for (auto k = 0; k < 6; ++k) {
    if (strvec[k] != vs[k]) {
      throw art::Exception(art::errors::LogicError)
        << "String #" << k << " expected to be '" << vs[k] << "', got '"
        << strvec[k] << "' instead!\n";
    }
  }

} // for_each_group_test()

DEFINE_ART_MODULE(AssnsAnalyzerFEG)
