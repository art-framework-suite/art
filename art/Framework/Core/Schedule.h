#ifndef art_Framework_Core_Schedule_h
#define art_Framework_Core_Schedule_h
// vim: set sw=2 expandtab :

//
//  A schedule is a sequence of trigger paths. After construction, events
//  can be fed to the object and passed through all the modules in the
//  schedule. All accounting about processing of events by modules and
//  paths is contained here or in object held by containment.
//
//  The trigger results producer is generated and managed here. This
//  class also manages calls to endjob and beginjob.
//
//  A TriggerResults object will always be inserted into the event for
//  any schedule. The producer of the TriggerResults EDProduct is always
//  the last module in the trigger path. The TriggerResultInserter is
//  given a fixed label of "TriggerResults".
//
//  Processing of an event happens by pushing the event through the
//  Paths. The scheduler performs the reset() on each of the workers
//  independent of the Path objects.
//

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/Path.h"
#include "art/Framework/Core/PathManager.h"
#include "art/Framework/Principal/Actions.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/Provenance.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceRegistry.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Utilities/Transition.h"
#include "canvas/Persistency/Common/HLTGlobalStatus.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ProvenanceFwd.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/exempt_ptr.h"
#include "cetlib/trim.h"
#include "fhiclcpp/ParameterSet.h"
#include "hep_concurrency/WaitingTask.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace art {

class ActivityRegistry;
class TriggerNamesService;
class Schedule;

class Schedule {

public:

  Schedule(int stream, PathManager&, std::string const& processName, fhicl::ParameterSet const& proc_pset, MasterProductRegistry&,
           ActionTable&, ActivityRegistry&);

  void
  process(Transition, Principal&);

  void
  process_event(hep::concurrency::WaitingTask* endPathTask, EventPrincipal&, int streamIndex);

  void
  beginJob();

  void
  endJob();

  void
  respondToOpenInputFile(FileBlock const&);

  void
  respondToCloseInputFile(FileBlock const&);

  void
  respondToOpenOutputFiles(FileBlock const&);

  void
  respondToCloseOutputFiles(FileBlock const&);

private:

  void
  process_event_pathsDone(hep::concurrency::WaitingTask* endPathTask, EventPrincipal&, int streamIndex);

private:

  int const
  stream_;

  fhicl::ParameterSet
  process_pset_;

  MasterProductRegistry&
  mpr_;

  ActionTable&
  actionTable_;

  ActivityRegistry&
  actReg_;

  std::string
  processName_;

  PathsInfo&
  triggerPathsInfo_;

  Worker*
  results_inserter_{};

};

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif /* art_Framework_Core_Schedule_h */
