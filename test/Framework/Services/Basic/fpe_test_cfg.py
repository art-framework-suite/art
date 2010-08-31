# Unit test configuration file for EnableFloatingPointExceptions service

import FWCore.ParameterSet.python.Config as cms

process = cms.Process("TEST")

import FWCore.Framework.python.test.cmsExceptionsFatal_cff
process.options = FWCore.Framework.python.test.cmsExceptionsFatal_cff.options

process.load("FWCore.Services.InitRootHandlers_cfi")

process.EnableFloatingPointExceptions = cms.Service("EnableFloatingPointExceptions",
    moduleNames = cms.untracked.vstring('default', 
        'module1', 
        'module2', 
        'module3', 
        'module4'),
    default = cms.untracked.PSet(
        enableOverFlowEx = cms.untracked.bool(False),
        enableDivByZeroEx = cms.untracked.bool(False),
        enableInvalidEx = cms.untracked.bool(False),
        enableUnderFlowEx = cms.untracked.bool(False)
    ),
    module4 = cms.untracked.PSet(
        enableOverFlowEx = cms.untracked.bool(False),
        enableDivByZeroEx = cms.untracked.bool(False),
        enableInvalidEx = cms.untracked.bool(False),
        enableUnderFlowEx = cms.untracked.bool(True)
    ),
    module3 = cms.untracked.PSet(
        enableOverFlowEx = cms.untracked.bool(True),
        enableDivByZeroEx = cms.untracked.bool(False),
        enableInvalidEx = cms.untracked.bool(False),
        enableUnderFlowEx = cms.untracked.bool(False)
    ),
    module2 = cms.untracked.PSet(
        enableOverFlowEx = cms.untracked.bool(False),
        enableDivByZeroEx = cms.untracked.bool(False),
        enableInvalidEx = cms.untracked.bool(True),
        enableUnderFlowEx = cms.untracked.bool(False)
    ),
    module1 = cms.untracked.PSet(
        enableOverFlowEx = cms.untracked.bool(False),
        enableDivByZeroEx = cms.untracked.bool(True),
        enableInvalidEx = cms.untracked.bool(False),
        enableUnderFlowEx = cms.untracked.bool(False)
    ),
    setPrecisionDouble = cms.untracked.bool(True),
    reportSettings = cms.untracked.bool(False)
)

process.MessageLogger = cms.Service("MessageLogger",
    fpe_infos = cms.untracked.PSet(
        threshold = cms.untracked.string('INFO'),
        noTimeStamps = cms.untracked.bool(True),
        FwkReport = cms.untracked.PSet(
            limit = cms.untracked.int32(0)
        ),
        preEventProcessing = cms.untracked.PSet(
            limit = cms.untracked.int32(0)
        )
    ),
    debugModules = cms.untracked.vstring('*'),
    categories = cms.untracked.vstring('preEventProcessing', 
        'FwkReport'),
    destinations = cms.untracked.vstring('fpe_infos')
)

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(3)
)

process.source = cms.Source("EmptySource")

process.module1 = cms.EDAnalyzer("UnitTestClient")

process.module2 = cms.EDAnalyzer("UnitTestClient")

process.module3 = cms.EDAnalyzer("UnitTestClient")

process.module4 = cms.EDAnalyzer("UnitTestClient")

process.p = cms.Path(process.module1*process.module2*process.module3*process.module4)


