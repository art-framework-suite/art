import FWCore.ParameterSet.python.Config as cms
import FWCore.ParameterSet.python.parseConfig as cmsParse
from sys import argv

fileInPath = argv[1]

if fileInPath.endswith('cfg'):
    print cmsParse.dumpCfg(fileInPath)
else:
    print cmsParse.dumpCff(fileInPath)

