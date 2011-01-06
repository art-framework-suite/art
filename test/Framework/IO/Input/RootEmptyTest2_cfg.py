# The following comments couldn't be translated into the new config version:

# Configuration file for RootInputTest

import FWCore.ParameterSet.python.Config as cms

process = cms.Process("READEMPTY")
process.load("FWCore.Framework.test.cmsExceptionsFatal_cff")

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(-1)
)
process.Thing = cms.EDProducer("ThingProducer",
    debugLevel = cms.untracked.int32(1)
)

process.output = cms.OutputModule("RootOutputModule",
    fileName = cms.untracked.string('RootEmptyTestOut.root')
)

process.source = cms.Source("RootSource",
    fileNames = cms.untracked.vstring('file:RootEmptyTest.root')
)

process.p = cms.Path(process.Thing)
process.ep = cms.EndPath(process.output)


