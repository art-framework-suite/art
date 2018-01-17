#ifndef art_Framework_Principal_fwd_h
#define art_Framework_Principal_fwd_h

// For enums only.
#include "art/Framework/Principal/ActionCodes.h"

// For declaration of std::unique_ptr.
#include <memory>

namespace art {

  class ActionTable; // Action.h
  class AssnsGroup;
  class ConsumesRecorder;
  class CurrentProcessingContext;
  class DataViewImpl;
  class DeferredProductGetter;
  class Event;
  class EventPrincipal;
  class Group;
  class GroupFactory;
  template <typename T>
  class Handle;
  class NoDelayedReader;
  class Principal;
  class Provenance;
  class RangeSetHandler;
  class Run;
  class RunPrincipal;
  class ProcessNameSelector;         // Selector.h
  class ProductInstanceNameSelector; // Selector.h
  class ModuleLabelSelector;         // Selector.h
  class MatchAllSelector;            // Selector.h
  template <class A, class B>
  class AndHelper; // Selector.h
  template <class A, class B>
  class OrHelper; // Selector.h
  template <class A>
  class NotHelper; // Selector.h
  template <class T>
  class ComposedSelectorWrapper; // Selector.h
  class Selector;
  class SelectorBase;
  class SubRun;
  class SubRunPrincipal;
  template <class T>
  class View;
  class Worker;
  struct WorkerParams;
  class BranchDescription;

  namespace gfactory {
    namespace detail {
      template <typename... ARGS>
      std::unique_ptr<Group> make_group(BranchDescription const& pd,
                                        ARGS&&... args);
    }
  }
} // art

#endif /* art_Framework_Principal_fwd_h */

// Local Variables:
// mode: c++
// End:
