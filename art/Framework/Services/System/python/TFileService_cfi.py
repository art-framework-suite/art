import FWCore.ParameterSet.python.Config as cms

TFileService = cms.Service("TFileService",
    fileName = cms.string('histo.root'),
    closeFileFast = cms.untracked.bool(False)
)
