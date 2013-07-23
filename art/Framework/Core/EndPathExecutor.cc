#include "art/Framework/Core/EndPathExecutor.h"

#include "art/Framework/Core/OutputFileInfo.h"

using std::placeholders::_1;

art::EndPathExecutor::
EndPathExecutor(PathManager & pm,
                ActionTable & actions,
                std::shared_ptr<ActivityRegistry> areg)
  :
  endPathInfo_(pm.endPathInfo()),
  act_table_(&actions),
  actReg_(areg),
  outputWorkers_()
{
  // All the workers should be in all_workers_ by this point. Thus
  // we can now fill all_output_workers_.  We provide a little
  // sanity-check to make sure no code modifications alter the
  // number of workers at a later date... Much better would be to
  // refactor this huge constructor into a series of well-named
  // private functions.
  doForAllWorkers_([this](Worker * w) {
      OutputWorker * ow = dynamic_cast<OutputWorker *>(w);
      if (ow) { outputWorkers_.emplace_back(ow); }
    });
}

bool art::EndPathExecutor::terminate() const
{
  if (!outputWorkers_.empty() && // Necessary because std::all_of()
                                 // returns true if range is empty.
      std::all_of(outputWorkers_.cbegin(),
                  outputWorkers_.cend(),
                  std::bind(&OutputWorker::limitReached, _1))) {
    mf::LogInfo("SuccessfulTermination")
      << "The job is terminating successfully because each output module\n"
      << "has reached its configured limit.\n";
    return true;
  }
  else {
    return false;
  }
}

void
art::EndPathExecutor::
endJob()
{
  bool failure = false;
  Exception error(errors::EndJobFailure);
  doForAllWorkers_
    ([&failure, &error](Worker * w)
     {
       try {
         w->endJob();
       }
       catch (cet::exception & e) {
         error << "cet::exception caught in Schedule::endJob\n"
               << e.explain_self();
         failure = true;
       }
       catch (std::exception & e) {
         error << "Standard library exception caught in Schedule::endJob\n"
               << e.what();
         failure = true;
       }
       catch (...) {
         error << "Unknown exception caught in Schedule::endJob\n";
         failure = true;
       }
     });
  if (failure) { throw error; }
}

void art::EndPathExecutor::closeOutputFiles()
{
  doForAllOutputWorkers_([this](OutputWorker * ow) {
      ow->closeFile();
      actReg_->
        sPostCloseOutputFile.invoke(OutputFileInfo(ow->label(),
                                                   ow->lastClosedFileName()));
    }
    );
}

void art::EndPathExecutor::openOutputFiles(FileBlock & fb)
{
  doForAllOutputWorkers_(std::bind(&OutputWorker::openFile, _1, std::cref(fb)));
}

void art::EndPathExecutor::writeRun(RunPrincipal const & rp)
{
  doForAllOutputWorkers_(std::bind(&OutputWorker::writeRun, _1, std::cref(rp)));
}

void art::EndPathExecutor::writeSubRun(SubRunPrincipal const & srp)
{
  doForAllOutputWorkers_(std::bind(&OutputWorker::writeSubRun, _1, std::cref(srp)));
}

bool art::EndPathExecutor::shouldWeCloseOutput() const
{
  return std::any_of(outputWorkers_.cbegin(),
                     outputWorkers_.cend(),
                     std::bind(&OutputWorker::shouldWeCloseFile, _1));
}

void art::EndPathExecutor::respondToOpenInputFile(FileBlock const & fb)
{
  doForAllWorkers_(std::bind(&Worker::respondToOpenInputFile, _1, std::cref(fb)));
}

void art::EndPathExecutor::respondToCloseInputFile(FileBlock const & fb)
{
  doForAllWorkers_(std::bind(&Worker::respondToCloseInputFile, _1, std::cref(fb)));
}

void art::EndPathExecutor::respondToOpenOutputFiles(FileBlock const & fb)
{
  doForAllWorkers_(std::bind(&Worker::respondToOpenOutputFiles, _1, std::cref(fb)));
}

void art::EndPathExecutor::respondToCloseOutputFiles(FileBlock const & fb)
{
  doForAllWorkers_(std::bind(&Worker::respondToCloseOutputFiles, _1, std::cref(fb)));
}

void art::EndPathExecutor::beginJob()
{
  doForAllWorkers_(std::bind(&Worker::beginJob, _1));
}

void
art::EndPathExecutor::resetAll()
{
  doForAllWorkers_(std::bind(&Worker::reset, _1));
}
