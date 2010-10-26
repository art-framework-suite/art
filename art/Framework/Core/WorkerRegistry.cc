
#include "art/Framework/Core/WorkerRegistry.h"

#include "art/Framework/Core/Factory.h"
#include "art/Framework/Core/Worker.h"

#include <sstream>

using fhicl::ParameterSet;

namespace 
{
  ModuleDescription
  createModuleDescription(WorkerParams const &p)
  {
    ParameterSet const& procParams = *p.procPset_;
    ParameterSet const& conf = *p.pset_;
    ModuleDescription md;
    md.parameterSetID_ = conf.id();
    md.moduleName_ = conf.get<std::string>("@module_type");
    md.moduleLabel_ = conf.get<std::string>("@module_label");
    md.processConfiguration_ = ProcessConfiguration(p.processName_, procParams.id(), p.releaseVersion_, p.passID_);
    return md;
  }
  
} // anonymous namespace

namespace art {

  WorkerRegistry::WorkerRegistry(boost::shared_ptr<ActivityRegistry> areg) :
    m_workerMap(),
    actReg_(areg)
  { }

  WorkerRegistry:: ~WorkerRegistry() {
    m_workerMap.clear();
  }

  void WorkerRegistry::clear() {
    m_workerMap.clear();
  }

  Worker* WorkerRegistry::getWorker(const WorkerParams& p) {

    std::string workerid =
      mangleWorkerParameters(*p.pset_, p.processName_,
			     p.releaseVersion_,p.passID_);

    WorkerMap::iterator workerIt = m_workerMap.find(workerid);

    // if the worker is not there, make it
    if (workerIt == m_workerMap.end())
      {
	ModuleDescription moduleDesc(createModuleDescription(p));
	actReg_->preModuleConstructionSignal_(moduleDesc);

	// This is what is getting refactored.
	//std::auto_ptr<Worker> workerPtr = Factory::get()->makeWorker(p, moduleDesc);
	std::auto_ptr<Worker> workerPtr = Factory::makeWorker(p, moduleDesc);

	actReg_->postModuleConstructionSignal_(moduleDesc);
	workerPtr->setActivityRegistry(actReg_);

        // Transfer ownership of worker to the registry
	m_workerMap[workerid].reset(workerPtr.release());
	return m_workerMap[workerid].get();
      }

    return (workerIt->second.get());

  }


  std::string WorkerRegistry::mangleWorkerParameters(ParameterSet const& parameterSet,
						     std::string const& processName,
						     ReleaseVersion const& releaseVersion,
						     PassID const& passID) {

  std::stringstream mangled_parameters;
  mangled_parameters<< parameterSet.to_string()
                    << processName
                    << releaseVersion
                    << passID;

    return mangled_parameters.str();

  }

}  // namespace art
