#ifndef art_Framework_Core_WorkerRegistry_h
#define art_Framework_Core_WorkerRegistry_h

// ======================================================================
//
// WorkerRegistry: The Registry of all workers that were requested
//
// Holds all instances of workers.
// In this implementation, Workers are owned.
//
// ======================================================================

#include "art/Framework/Principal/WorkerParams.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Persistency/Provenance/PassID.h"
#include "art/Persistency/Provenance/ReleaseVersion.h"
#include "cpp0x/memory"
#include "fhiclcpp/ParameterSet.h"
#include <map>
#include <string>

// ----------------------------------------------------------------------

namespace art {

  class Worker;

  class WorkerRegistry {

  public:
    WorkerRegistry(WorkerRegistry const&) = delete;
    WorkerRegistry& operator=(WorkerRegistry const&) = delete;

    explicit WorkerRegistry(std::shared_ptr<ActivityRegistry> areg);

    // use compiler-generated copy c'tor, copy assignment, and d'tor

    /// Retrieve the particular instance of the worker
    /** If the worker with that set of parameters does not exist,
        create it
        @note Workers are owned by this class, do not delete them*/
    Worker*  getWorker(WorkerParams const&);
    void clear();

  private:
    /// Get a unique name for the worker
    /** Form a string to be used as a key in the map of workers */
    std::string mangleWorkerParameters(fhicl::ParameterSet const& parameterSet,
                                       std::string const& processName,
                                       ReleaseVersion const& releaseVersion,
                                       PassID const& passID);

    /// the container of workers
    typedef std::map<std::string, std::shared_ptr<Worker> > WorkerMap;

    /// internal map of registered workers (owned).
    WorkerMap m_workerMap;
    std::shared_ptr<ActivityRegistry> actReg_;

  };  // WorkerRegistry


} // art

// ======================================================================

#endif /* art_Framework_Core_WorkerRegistry_h */

// Local Variables:
// mode: c++
// End:
