import FWCore.ParameterSet.python.Config as cms
process = cms.Process("C")
process.source = cms.Source("RootSource", fileNames=cms.untracked.vstring("file:b.root"))
process.test = cms.OutputModule("ProvenanceCheckerOutputModule")
process.o = cms.EndPath(process.test)
