import FWCore.ParameterSet.python.Config as cms

import FWCore.Framework.python.test.cmsExceptionsFatalOption_cff
options = cms.untracked.PSet(
  Rethrow = FWCore.Framework.python.test.cmsExceptionsFatalOption_cff.Rethrow
)
