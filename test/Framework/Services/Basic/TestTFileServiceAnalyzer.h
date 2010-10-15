#ifndef FWCore_Services_TestTFileServiceAnalyzer_h
#define FWCore_Services_TestTFileServiceAnalyzer_h

#include <string>
#include "TH1.h"


#include "art/Framework/Core/EDAnalyzer.h"
#include "art/ParameterSet/ParameterSetfwd.h"


class TestTFileServiceAnalyzer : public art::EDAnalyzer {
public:
  /// constructor
  TestTFileServiceAnalyzer( const art::ParameterSet & );

private:
  /// process one event
  void analyze( const art::Event& , const art::EventSetup& );
  /// histograms
  TH1F * h_test1, * h_test2;
  /// sub-directory name
  std::string dir1_, dir2_;
};

#endif

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
