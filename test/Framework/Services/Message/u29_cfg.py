# Unit test configuration file for MessageLogger service
# Tests the hardwired defaults
# Does not include MessageLogger.cfi nor explicitly mention MessageLogger
# or MessageService at all.
# Not suitable for unit test because the time stamps will not be disabled

import FWCore.ParameterSet.python.Config as cms

process = cms.Process("TEST")

import FWCore.Framework.python.test.cmsExceptionsFatal_cff
process.options = FWCore.Framework.python.test.cmsExceptionsFatal_cff.options

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(2)
)

process.source = cms.Source("EmptySource")

process.sendSomeMessages = cms.EDAnalyzer("UnitTestClient_A")

process.p = cms.Path(process.sendSomeMessages)
