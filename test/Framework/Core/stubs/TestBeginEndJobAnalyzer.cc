// -*- C++ -*-
//
// Package:    TestBeginEndJobAnalyzer
// Class:      TestBeginEndJobAnalyzer
//

#include "art/Framework/Core/Frameworkfwd.h"
#include "test/Framework/Core/stubs/TestBeginEndJobAnalyzer.h"

#include <memory>

TestBeginEndJobAnalyzer::
TestBeginEndJobAnalyzer(const fhicl::ParameterSet& pset)
  : art::EDAnalyzer(pset)
{
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
