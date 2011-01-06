# The following comments couldn't be translated into the new config version:

# Configuration file for RootInputTest

import FWCore.ParameterSet.python.Config as cms

process = cms.Process("TESTRECO")
process.load("FWCore.Framework.test.cmsExceptionsFatal_cff")

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(-1)
)
process.OtherThing = cms.EDProducer("OtherThingProducer",
    debugLevel = cms.untracked.int32(1)
)

process.Analysis = cms.EDAnalyzer("OtherThingAnalyzer",
    debugLevel = cms.untracked.int32(1)
)

process.source = cms.Source("RootSource",
    setRunNumber = cms.untracked.uint32(621),
    fileNames = cms.untracked.vstring('file:RootInputTest.root',
        'file:RootInputOther.root')
)

process.p = cms.Path(process.OtherThing*process.Analysis)


