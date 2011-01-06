# Define the default configuration for the framework
import FWCore.ParameterSet.python.Config as mu2e

# Give this job a name
process = mu2e.Process("RNGTest02")

# Maximum number of events to do
process.maxEvents = mu2e.untracked.PSet(
    input = mu2e.untracked.int32(3)
)

# Configure the random number service
process.add_(mu2e.Service( "RandomNumberGeneratorService",
                           debug = mu2e.untracked.bool(True),
            )            )

# Define and configure some modules to do work on each event
process.source = mu2e.Source( "EmptySource" )
process.rngstest = mu2e.EDAnalyzer( "RNGS_producer",
                                    seed1 = mu2e.untracked.vint32(70933),
                                    seed2 = mu2e.untracked.vint32(33907),
                                  )
##process.randomsaver = mu2e.EDAnalyzer("RandomNumberSaver",
##                                      debug = mu2e.untracked.bool(True)
##                                      )
##process.outfile = mu2e.OutputModule("RootOutputModule",
##                      fileName = mu2e.untracked.string('file:randomtest_01.root'),
##)

# Tell the system to execute all paths
##process.output = mu2e.EndPath(  process.rngtest*process.randomsaver*process.outfile );
process.output = mu2e.EndPath(  process.rngstest );
