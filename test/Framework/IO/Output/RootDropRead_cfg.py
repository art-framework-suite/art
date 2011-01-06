import FWCore.ParameterSet.python.Config as cms

process = cms.Process("TESTDROPREAD")
process.load("FWCore.Framework.test.cmsExceptionsFatal_cff")

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(-1)
)
process.source = cms.Source("RootSource",
    fileNames = cms.untracked.vstring('file:RootDropTest.root')
)



