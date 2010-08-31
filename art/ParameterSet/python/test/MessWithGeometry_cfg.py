import FWCore.ParameterSet.python.Config as cms

process = cms.Process("P")
process.setStrict(True)
process.load("FWCore.ParameterSet.python.test.Geometry_cfi")
process.load("FWCore.ParameterSet.python.test.MessWithGeometry_cff")
process.load("FWCore.ParameterSet.python.test.MessWithPreshower_cff")
#check that both changes got merged
print process.geometry.dumpPython()

