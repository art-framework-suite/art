#include "art/Framework/Core/EndPathExecutor.h"

#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "art/Utilities/OutputFileInfo.h"
#include "cetlib/container_algorithms.h"

#include <memory>
#include <type_traits>
#include <utility>


art::EndPathExecutor::
EndPathExecutor(PathManager & pm,
                ActionTable & actions,
                ActivityRegistry & areg,
                MasterProductRegistry& mpr)
  :
  endPathInfo_(pm.endPathInfo()),
  act_table_(&actions),
  actReg_(areg),
  outputWorkers_(),
  workersToClose_{{}}, // filled by aggregation
  workersEnabled_(endPathInfo_.workers().size(), true),
  outputWorkersEnabled_()
  {
    // For clarity, don't used doForAllEnabledWorkers_(), here.
    size_t index = 0;
    for (auto const & val : endPathInfo_.workers()) {
      auto w = val.second.get();
      if (auto ow = dynamic_cast<OutputWorker*>(w)) {
        outputWorkers_.emplace_back(ow);
        outputWorkersEnabled_.emplace_back(workersEnabled_[index]);
      }
      ++index;
    }
    mpr.registerProductListUpdateCallback(std::bind(&art::EndPathExecutor::selectProducts, this, std::placeholders::_1));
  }

bool art::EndPathExecutor::terminate() const
{
  bool rc = !outputWorkers_.empty() && // Necessary because std::all_of()
                                       // returns true if range is empty.
    std::all_of(outputWorkers_.cbegin(),
                outputWorkers_.cend(),
                [](auto& w){ return w->limitReached(); });
  if (rc) {
    mf::LogInfo("SuccessfulTermination")
      << "The job is terminating successfully because each output module\n"
      << "has reached its configured limit.\n";
  }
  return rc;
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

void art::EndPathExecutor::closeAllOutputFiles()
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

void art::EndPathExecutor::openAllOutputFiles(FileBlock & fb)
{
  doForAllEnabledOutputWorkers_([this, &fb](OutputWorker * ow) {
      ow->openFile(fb);
      actReg_.
        sPostOpenOutputFile.invoke(ow->label());
    }
    );
}

void art::EndPathExecutor::writeRun(RunPrincipal& rp)
{
  doForAllEnabledOutputWorkers_([&rp](auto w){ w->writeRun(rp); });
}

void art::EndPathExecutor::writeSubRun(SubRunPrincipal& srp)
{
  doForAllEnabledOutputWorkers_([&srp](auto w){ w->writeSubRun(srp); });
}

void art::EndPathExecutor::writeEvent(EventPrincipal& ep)
{
  doForAllEnabledOutputWorkers_([&ep](auto w){ w->writeEvent(ep); });
}

void art::EndPathExecutor::selectProducts(FileBlock const& fb)
{
  doForAllEnabledOutputWorkers_([&fb](auto w) { w->selectProducts(fb); });
}

void art::EndPathExecutor::recordOutputClosureRequests()
{
  for(auto ow : outputWorkers_) {
    if (!ow->stagedToCloseFile() && ow->requestsToCloseFile()) {
      workersToClose_[ow->fileSwitchBoundary()].push_back(ow);
      ow->flagToCloseFile(true);
    }
  }
}

bool art::EndPathExecutor::outputToCloseAtBoundary(Boundary const b) const
{
  return !workersToClose_[b].empty();
}

void art::EndPathExecutor::switchOutputFiles(std::size_t const b, FileBlock const& fb)
{
  closeSomeOutputFiles(b);
  openSomeOutputFiles(b,fb);
}

void art::EndPathExecutor::closeSomeOutputFiles(std::size_t const b)
{
  auto respondToCloseOutputFile    = [    ](auto ow){ow->respondToCloseOutputFile();};
  auto closeFile                   = [    ](auto ow){ow->closeFile();};
  auto invoke_sPreCloseOutputFile  = [this](auto ow){actReg_.sPreCloseOutputFile.invoke(ow->label());};
  auto invoke_sPostCloseOutputFile = [this](auto ow){actReg_.sPostCloseOutputFile.invoke(OutputFileInfo{ow->label(), ow->lastClosedFileName()});};

  auto& workers = workersToClose_[b];
  if (workers.empty()) return;

  cet::for_all(workers, respondToCloseOutputFile);
  cet::for_all(workers, invoke_sPreCloseOutputFile);
  cet::for_all(workers, closeFile);
  cet::for_all(workers, invoke_sPostCloseOutputFile);
}

void art::EndPathExecutor::openSomeOutputFiles(std::size_t const b, FileBlock const& fb)
{

  auto respondToOpenOutputFile     = [    ](auto ow){ow->respondToOpenOutputFile();};
  auto openFile                    = [ &fb](auto ow){ow->openFile(fb);};
  auto resetFlagToClose            = [    ](auto ow){ow->flagToCloseFile(false);};
  auto invoke_sPostOpenOutputFile  = [this](auto ow){actReg_.sPostOpenOutputFile.invoke(ow->label());};

  auto& workers = workersToClose_[b];
  if (workers.empty()) return;

  cet::for_all(workers, resetFlagToClose);
  cet::for_all(workers, openFile);
  cet::for_all(workers, invoke_sPostOpenOutputFile);
  cet::for_all(workers, respondToOpenOutputFile);

  workers.clear();
}


void art::EndPathExecutor::respondToOpenInputFile(FileBlock const & fb)
{
  doForAllEnabledWorkers_([&fb](auto w){ w->respondToOpenInputFile(fb); });
}

void art::EndPathExecutor::respondToCloseInputFile(FileBlock const& fb)
{
  doForAllEnabledWorkers_([&fb](auto w){ w->respondToCloseInputFile(fb); });
}

void art::EndPathExecutor::respondToOpenOutputFiles(FileBlock const& fb)
{
  doForAllEnabledWorkers_([   ](auto w){ w->respondToOpenOutputFile(); });
  doForAllEnabledWorkers_([&fb](auto w){ w->respondToOpenOutputFiles(fb); });
}

void art::EndPathExecutor::respondToCloseOutputFiles(FileBlock const& fb)
{
  doForAllEnabledWorkers_([&fb](auto w){ w->respondToCloseOutputFiles(fb); });
  doForAllEnabledWorkers_([   ](auto w){ w->respondToCloseOutputFile(); });
}

void art::EndPathExecutor::respondToOpenOutputFile()
{
  for (std::size_t i = Boundary::Event; i!=Boundary::InputFile; ++i) {
    auto& workers = workersToClose_[i];
    cet::for_all(workers, [](auto w){ w->respondToOpenOutputFile(); });
  }
}

void art::EndPathExecutor::respondToCloseOutputFile()
{
  for (std::size_t i = Boundary::Event; i!=Boundary::InputFile; ++i) {
    auto& workers = workersToClose_[i];
    cet::for_all(workers, [](auto w){ w->respondToCloseOutputFile(); });
  }
}

void art::EndPathExecutor::beginJob()
{
  doForAllEnabledWorkers_([](auto w){ w->beginJob(); });
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
  doForAllEnabledWorkers_([](auto w){ w->reset(); });
}
