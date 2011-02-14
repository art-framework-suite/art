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


#include "test/Framework/Core/stubs/TestBeginEndJobAnalyzer.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include <memory>

TestBeginEndJobAnalyzer::TestBeginEndJobAnalyzer(const fhicl::ParameterSet& /* iConfig */) {
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
TestBeginEndJobAnalyzer::beginRun(art::Run const&) {
  beginRunCalled = true;
}

void
TestBeginEndJobAnalyzer::endRun(art::Run const&) {
  endRunCalled = true;
}

void
TestBeginEndJobAnalyzer::beginSubRun(art::SubRun const&) {
  beginSubRunCalled = true;
}

void
TestBeginEndJobAnalyzer::endSubRun(art::SubRun const&) {
  endSubRunCalled = true;
}

void
TestBeginEndJobAnalyzer::analyze(const art::Event& /* iEvent */, const art::EventSetup& /* iSetup */) {
}
