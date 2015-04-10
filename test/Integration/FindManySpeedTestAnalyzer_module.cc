////////////////////////////////////////////////////////////////////////
// Class:       FindManySpeedTestAnalyzer
// Module Type: analyzer
// File:        FindManySpeedTestAnalyzer_module.cc
//
// Generated at Wed Sep 10 15:06:37 2014 by Christopher Green using artmod
// from cetpkgsupport v1_07_01.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/FindManyP.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/View.h"
#include "art/Utilities/InputTag.h"
#include "cetlib/cpu_timer.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "test/TestObjects/ToyProducts.h"

#include <cassert>
#include <iostream>

namespace arttest {
  class FindManySpeedTestAnalyzer;
}

class arttest::FindManySpeedTestAnalyzer : public art::EDAnalyzer {
public:
  explicit FindManySpeedTestAnalyzer(fhicl::ParameterSet const & p);
  // The destructor generated by the compiler is fine for classes
  // without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  FindManySpeedTestAnalyzer(FindManySpeedTestAnalyzer const &) = delete;
  FindManySpeedTestAnalyzer(FindManySpeedTestAnalyzer &&) = delete;
  FindManySpeedTestAnalyzer & operator = (FindManySpeedTestAnalyzer const &) = delete;
  FindManySpeedTestAnalyzer & operator = (FindManySpeedTestAnalyzer &&) = delete;

  // Required functions.
  void analyze(art::Event const & e) override;


private:
  std::string const producerLabel_;
  bool const perTrackDiag_;
};


arttest::FindManySpeedTestAnalyzer::FindManySpeedTestAnalyzer(fhicl::ParameterSet const & p)
  :
  EDAnalyzer(p),
  producerLabel_(p.get<std::string>("producerLabel")),
  perTrackDiag_(p.get<bool>("perTrackDiag", false))
{}

void arttest::FindManySpeedTestAnalyzer::analyze(art::Event const & e)
{
  auto hH = e.getValidHandle<std::vector<Hit> >(producerLabel_);
  std::cout << "Hit collection size: " << hH->size() << ".\n";
  auto hT = e.getValidHandle<std::vector<Track> >(producerLabel_);
  std::cout << "Track collection size: " << hT->size() << ".\n";
  auto hA = e.getValidHandle<art::Assns<Hit, Track> >(producerLabel_);
  std::cout << "Assns size = " << hA->size() << ".\n";

  // Make a collection of Ptrs so we can exercise the algorithm at
  // issue.
  art::View<Track> tv;
  e.getView<Track>(producerLabel_, tv);
  art::PtrVector<Track> tPtrs;
  tv.fill(tPtrs);
  assert(tPtrs.size() == hT->size());

  // Time the activity under test.
  cet::cpu_timer timer;
  timer.start();
  art::FindManyP<Hit> fmp(tPtrs, e, producerLabel_);
  timer.stop();
  std::cout << "FindManyP size = " << fmp.size() << ".\n";
  std::cout << "FindManyP construction time (CPU, real): (" << timer.cpuTime() << ", " << timer.realTime() << ") s.\n";
  if (perTrackDiag_) {
    for (size_t i = 0, e = fmp.size(); i != e; ++i) {
      std::cout << "Track # " << i << " has " << fmp.at(i).size() << " associated hits.\n";
    }
  }
}

DEFINE_ART_MODULE(arttest::FindManySpeedTestAnalyzer)
