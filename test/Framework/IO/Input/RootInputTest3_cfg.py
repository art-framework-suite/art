# The following comments couldn't be translated into the new config version:

# Configuration file for RootInputTest

import FWCore.ParameterSet.python.Config as cms

process = cms.Process("TESTRECO")
process.load("FWCore.Framework.test.cmsExceptionsFatal_cff")

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(-1)
)
process.Analysis = cms.EDAnalyzer("OtherThingAnalyzer",
    thingWasDropped = cms.untracked.bool(True),
    debugLevel = cms.untracked.int32(1)
)

process.source = cms.Source("RootSource",
    setRunNumber = cms.untracked.uint32(621),
    fileNames = cms.untracked.vstring('file:RootInputDropTest.root')
)

process.p = cms.Path(process.Analysis)


