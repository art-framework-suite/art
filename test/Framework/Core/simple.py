import FWCore.ParameterSet.python.Config as cms

process = cms.Process("PROD")

#import FWCore.Framework.python.test.cmsExceptionsFatalOption_cff
#process.options = cms.untracked.PSet(
#    wantSummary = cms.untracked.bool(True),
#    Rethrow = FWCore.Framework.python.test.cmsExceptionsFatalOption_cff.Rethrow
#)

#process.maxEvents = cms.untracked.PSet(
#    input = cms.untracked.int32(99)
#)

#process.source = cms.Source("EmptySource")
#process.m1a = cms.EDProducer("IntProducer", ivalue = cms.int32(1) )
#process.p1 = cms.Path(process.m1a)


