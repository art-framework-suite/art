# Default configuration for the UnixSignalService

import FWCore.ParameterSet.python.Config as cms

UnixSignalService = cms.Service("UnixSignalService",
    EnableCtrlC = cms.untracked.bool(True)
)
