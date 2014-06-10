#include "art/Framework/Core/EndPathExecutor.h"

#include "art/Utilities/OutputFileInfo.h"

using std::placeholders::_1;

art::EndPathExecutor::
EndPathExecutor(PathManager & pm,
                ActionTable & actions,
                ActivityRegistry & areg)
  :
  endPathInfo_(pm.endPathInfo()),
  act_table_(&actions),
  actReg_(areg),
  outputWorkers_(),
  workersEnabled_(endPathInfo_.workers().size(), true),
  outputWorkersEnabled_()
{
  // For clarity, don't used doForAllEnabledWorkers_(), here.
  for (auto const & val : endPathInfo_.workers()) {
    auto w = val.second.get();
    OutputWorker * ow = dynamic_cast<OutputWorker *>(w);
    size_t index = 0;
    if (ow) {
      outputWorkers_.emplace_back(ow);
      outputWorkersEnabled_.emplace_back(workersEnabled_[index]);
    }
    ++index;
  }
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
  doForAllEnabledWorkers_
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
  doForAllEnabledOutputWorkers_([this](OutputWorker * ow) {
      actReg_.sPreCloseOutputFile.invoke(ow->label());
      ow->closeFile();
      actReg_.
        sPostCloseOutputFile.invoke(OutputFileInfo(ow->label(),
                                                   ow->lastClosedFileName()));
    }
    );
}

void art::EndPathExecutor::openOutputFiles(FileBlock & fb)
{
  doForAllEnabledOutputWorkers_([this, &fb](OutputWorker * ow) {
      ow->openFile(fb);
      actReg_.
        sPostOpenOutputFile.invoke(ow->label());
    }
    );
}

void art::EndPathExecutor::writeRun(RunPrincipal const & rp)
{
  doForAllEnabledOutputWorkers_(std::bind(&OutputWorker::writeRun, _1, std::cref(rp)));
}

void art::EndPathExecutor::writeSubRun(SubRunPrincipal const & srp)
{
  doForAllEnabledOutputWorkers_(std::bind(&OutputWorker::writeSubRun, _1, std::cref(srp)));
}

bool art::EndPathExecutor::shouldWeCloseOutput() const
{
  return std::any_of(outputWorkers_.cbegin(),
                     outputWorkers_.cend(),
                     std::bind(&OutputWorker::shouldWeCloseFile, _1));
}

void art::EndPathExecutor::respondToOpenInputFile(FileBlock const & fb)
{
  doForAllEnabledWorkers_(std::bind(&Worker::respondToOpenInputFile, _1, std::cref(fb)));
}

void art::EndPathExecutor::respondToCloseInputFile(FileBlock const & fb)
{
  doForAllEnabledWorkers_(std::bind(&Worker::respondToCloseInputFile, _1, std::cref(fb)));
}

void art::EndPathExecutor::respondToOpenOutputFiles(FileBlock const & fb)
{
  doForAllEnabledWorkers_(std::bind(&Worker::respondToOpenOutputFiles, _1, std::cref(fb)));
}

void art::EndPathExecutor::respondToCloseOutputFiles(FileBlock const & fb)
{
  doForAllEnabledWorkers_(std::bind(&Worker::respondToCloseOutputFiles, _1, std::cref(fb)));
}

void art::EndPathExecutor::beginJob()
{
  doForAllEnabledWorkers_(std::bind(&Worker::beginJob, _1));
}

bool
art::EndPathExecutor::
setEndPathModuleEnabled(std::string const & label, bool enable)
{
  bool result;
  auto & workers = endPathInfo_.workers();
  WorkerMap::iterator foundW;
  if ((foundW = workers.find(label)) != workers.end()) {
    size_t index = std::distance(workers.begin(), foundW);
    result = workersEnabled_[index];
    workersEnabled_[index] = enable;
  } else {
    throw Exception(errors::ScheduleExecutionFailure)
      << "Attempt to "
      << (enable?"enable":"disable")
      << " unconfigured module "
      << label
      << ".\n";
  }
  auto owFinder =
    [&label](OutputWorkers::const_reference ow)
  {
    return ow->label() == label;
  };
  OutputWorkers::iterator foundOW;
  if ((foundOW = std::find_if(outputWorkers_.begin(),
                              outputWorkers_.end(),
                              owFinder)) != outputWorkers_.end()) {
    auto index = std::distance(outputWorkers_.begin(), foundOW);
    outputWorkersEnabled_[index] = enable;
  }
  return result;
}

void
art::EndPathExecutor::resetAll()
{
  doForAllEnabledWorkers_(std::bind(&Worker::reset, _1));
}
