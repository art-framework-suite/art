# The following comments couldn't be translated into the new config version:

# Configuration file for PreRootInputTest

import FWCore.ParameterSet.python.Config as cms

process = cms.Process("TESTPROD")
process.load("FWCore.Framework.test.cmsExceptionsFatal_cff")

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(11)
)
process.Thing = cms.EDProducer("ThingProducer",
    debugLevel = cms.untracked.int32(1)
)

process.output = cms.OutputModule("RootOutputModule",
    fileName = cms.untracked.string('RootInputTest.root')
)

process.source = cms.Source("EmptySource",
    firstSubRun = cms.untracked.uint32(6),
    numberEventsInSubRun = cms.untracked.uint32(3),
    firstRun = cms.untracked.uint32(561),
    numberEventsInRun = cms.untracked.uint32(7)
)

process.p = cms.Path(process.Thing)
process.ep = cms.EndPath(process.output)


