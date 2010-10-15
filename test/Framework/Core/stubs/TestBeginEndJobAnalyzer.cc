// -*- C++ -*-
//
// Package:    TestBeginEndJobAnalyzer
// Class:      TestBeginEndJobAnalyzer
//
/**\class TestBeginEndJobAnalyzer TestBeginEndJobAnalyzer.cc stubs/TestBeginEndJobAnalyzer/src/TestBeginEndJobAnalyzer.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Chris Jones
//         Created:  Fri Sep  2 13:54:17 EDT 2005
//
//
//


#include "FWCore/Framework/test/stubs/TestBeginEndJobAnalyzer.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include <memory>

TestBeginEndJobAnalyzer::TestBeginEndJobAnalyzer(const art::ParameterSet& /* iConfig */) {
}

TestBeginEndJobAnalyzer::~TestBeginEndJobAnalyzer() {
  destructorCalled = true;
}

bool TestBeginEndJobAnalyzer::beginJobCalled = false;
bool TestBeginEndJobAnalyzer::endJobCalled = false;
bool TestBeginEndJobAnalyzer::beginRunCalled = false;
bool TestBeginEndJobAnalyzer::endRunCalled = false;
bool TestBeginEndJobAnalyzer::beginSubRunCalled = false;
bool TestBeginEndJobAnalyzer::endSubRunCalled = false;
bool TestBeginEndJobAnalyzer::destructorCalled = false;

void
TestBeginEndJobAnalyzer::beginJob(const art::EventSetup&) {
  beginJobCalled = true;
}

void
TestBeginEndJobAnalyzer::endJob() {
  endJobCalled = true;
}

void
TestBeginEndJobAnalyzer::beginRun(art::Run const&, art::EventSetup const&) {
  beginRunCalled = true;
}

void
TestBeginEndJobAnalyzer::endRun(art::Run const&, art::EventSetup const&) {
  endRunCalled = true;
}

void
TestBeginEndJobAnalyzer::beginSubRun(art::SubRun const&, art::EventSetup const&) {
  beginSubRunCalled = true;
}

void
TestBeginEndJobAnalyzer::endSubRun(art::SubRun const&, art::EventSetup const&) {
  endSubRunCalled = true;
}

void
TestBeginEndJobAnalyzer::analyze(const art::Event& /* iEvent */, const art::EventSetup& /* iSetup */) {
}
