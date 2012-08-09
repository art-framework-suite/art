#ifndef art_Framework_Principal_fwd_h
#define art_Framework_Principal_fwd_h

// For enums only.
#include "art/Framework/Principal/ActionCodes.h"
#include "art/Framework/Principal/BranchActionType.h"

namespace art {

  class ActionTable; // Action.h
  class AssnsGroup;
  class CurrentProcessingContext;
  class DataViewImpl;
  class DeferredProductGetter;
  class Event;
  class EventPrincipal;
  class Group;
  class GroupFactory;
  template< typename T > class Handle;
  class NoDelayedReader;
  class Principal;
  class Provenance;
  class Run;
  class RunPrincipal;
  class ProcessNameSelector; // Selector.h
  class ProductInstanceNameSelector; // Selector.h
  class ModuleLabelSelector; // Selector.h
  class MatchAllSelector; // Selector.h
  template <class A, class B> class AndHelper; // Selector.h
  template <class A, class B> class OrHelper; // Selector.h
  template <class A> class NotHelper; // Selector.h
  template <class T> class ComposedSelectorWrapper; // Selector.h
  class RunStopwatch;
  class Selector;
  class SelectorBase;
  class SubRun;
  class SubRunPrincipal;
  template <class T> class View;
  class Worker;
  class WorkerParams;
}  // art

#endif /* art_Framework_Principal_fwd_h */

// Local Variables:
// mode: c++
// End:
