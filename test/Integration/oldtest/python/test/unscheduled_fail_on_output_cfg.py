import FWCore.ParameterSet.python.Config as cms

process = cms.Process('Test')

process.source = cms.Source('EmptySource')
process.failing = cms.EDProducer('FailingProducer')
process.i = cms.EDProducer('IntProducer',
                           ivalue = cms.int32(10) )
process.out = cms.OutputModule('RootOutputModule',
                                fileName = cms.untracked.string('unscheduled_fail_on_output.root'))

process.o = cms.EndPath(process.out)
process.p = cms.Path(process.i)

process.options = cms.untracked.PSet( allowUnscheduled = cms.untracked.bool(True) )

process.maxEvents = cms.untracked.PSet(
        input = cms.untracked.int32(10)
        )
