
# This configuration is designed to be run as the last
# in a series of fw processes.

# Tests the maxSubRuns parameter

# checks to see that the process level fakeRaw overrides the file based one

import FWCore.ParameterSet.python.Config as cms

process = cms.Process("PROD2TEST")

process.maxSubRuns = cms.untracked.PSet(
  input = cms.untracked.int32(3)
)

process.load("FWCore.MessageService.python.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 1000

import FWCore.Framework.python.test.cmsExceptionsFatalOption_cff
process.options = cms.untracked.PSet(
    allowUnscheduled = cms.untracked.bool(True),
    Rethrow = FWCore.Framework.python.test.cmsExceptionsFatalOption_cff.Rethrow
)


process.source = cms.Source("RootSource",
  fileNames = cms.untracked.vstring('file:testSeriesOfProcessesTEST.root'),
)

# Override the fakeRaw found in the test file
process.fakeRaw = cms.EDProducer(
  "IntProducer",
  ivalue = cms.int32(20)
)


# This module tests to see if the products put in at the first step
# (the fake HLT step) survived through to the last file.  At the PROD
# stage the products were split into two files so this test secondary
# file input.
process.a = cms.EDAnalyzer("TestFindProduct",
  inputTags = cms.untracked.VInputTag( cms.InputTag("fakeRaw"),
                                       cms.InputTag("fakeHLTDebug") ),

  # Test the maxSubRun parameter
  # 3 subRuns contain 15 events
  # Each event contains one product with a value of 20 and
  # one product with a value of 1000
  # If the maxSubRun parameter is working correctly the
  # following should be the sum of all the values.
  # The product values are hard coded into the fake
  # HLT configuration (the first one in this series).
  expectedSum = cms.untracked.int32(15300)
)

process.test1 = cms.Path(process.a)

process.out = cms.OutputModule("RootOutputModule",
  fileName = cms.untracked.string('testSeriesOfProcessesPROD2TEST_u.root'),
  outputCommands = cms.untracked.vstring(
    "keep *_fakeRaw_*_*"
  )
)
process.o = cms.EndPath(process.out)
