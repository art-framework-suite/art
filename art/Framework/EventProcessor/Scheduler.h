#ifndef art_Framework_EventProcessor_Scheduler_h
#define art_Framework_EventProcessor_Scheduler_h

#include "art/Framework/Principal/Actions.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/OptionalDelegatedParameter.h"
#include "fhiclcpp/types/Sequence.h"
#include "fhiclcpp/types/Table.h"
#include "fhiclcpp/types/TableFragment.h"
#include "tbb/task_scheduler_init.h"

#include <string>

namespace art {
  class Scheduler {
  public:
    struct Config {
      using Name = fhicl::Name;
      fhicl::Atom<int> num_threads{Name{"num_threads"}};
      fhicl::Atom<int> num_schedules{Name{"num_schedules"}};
      fhicl::Atom<bool> handleEmptyRuns{Name{"handleEmptyRuns"}, true};
      fhicl::Atom<bool> handleEmptySubRuns{Name{"handleEmptySubRuns"}, true};
      fhicl::Atom<bool> errorOnMissingConsumes{Name{"errorOnMissingConsumes"},
                                               false};
      fhicl::Atom<bool> errorOnFailureToPut{Name{"errorOnFailureToPut"}, true};
      fhicl::Atom<bool> errorOnSIGINT{Name{"errorOnSIGINT"}, true};
      fhicl::Atom<bool> wantSummary{Name{"wantSummary"}, false};
      fhicl::OptionalDelegatedParameter configOut{Name{"configOut"}};
      fhicl::OptionalDelegatedParameter debugConfig{Name{"debugConfig"}};
      fhicl::OptionalDelegatedParameter validateConfig{Name{"validateConfig"}};
      fhicl::TableFragment<ActionTable::Config> actionTable{};
    };
    using Parameters = fhicl::Table<Config>;
    explicit Scheduler(Parameters const& p);

    ActionTable const&
    actionTable() const noexcept
    {
      return actionTable_;
    }

    int
    num_threads() const noexcept
    {
      return nThreads_;
    }
    int
    num_schedules() const noexcept
    {
      return nSchedules_;
    }
    bool
    handleEmptyRuns() const noexcept
    {
      return handleEmptyRuns_;
    }
    bool
    handleEmptySubRuns() const noexcept
    {
      return handleEmptySubRuns_;
    }
    bool
    errorOnMissingConsumes() const noexcept
    {
      return errorOnMissingConsumes_;
    }
    bool
    wantSummary() const noexcept
    {
      return wantSummary_;
    }

  private:
    // A table of responses to be taken on reception of thrown
    // exceptions.
    ActionTable actionTable_;
    int const nThreads_;
    int const nSchedules_;
    bool const handleEmptyRuns_;
    bool const handleEmptySubRuns_;
    bool const errorOnMissingConsumes_;
    bool const wantSummary_;
    tbb::task_scheduler_init tbbManager_{tbb::task_scheduler_init::deferred};
  };
}

#endif
