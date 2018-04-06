#ifndef art_Framework_EventProcessor_Scheduler_h
#define art_Framework_EventProcessor_Scheduler_h

#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/Sequence.h"
#include "fhiclcpp/types/Table.h"
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
      fhicl::Atom<bool> errorOnMissingConsumes{Name{"errorOnMissingConsumes"}, false};
      fhicl::Atom<bool> errorOnFailureToPut{Name{"errorOnFailureToPut"}, true};
      fhicl::Atom<bool> errorOnSIGINT{Name{"errorOnSIGINT"}, true};
      fhicl::Atom<bool> defaultExceptions{Name{"defaultExceptions"}, true};
      fhicl::Atom<bool> wantSummary{Name{"wantSummary"}, false};
      fhicl::Atom<bool> wantTracer{Name{"wantTracer"}, false};
      fhicl::Sequence<std::string> ignoreCompletely{Name{"IgnoreCompletely"}, {}};
      fhicl::Sequence<std::string> rethrow{Name{"Rethrow"}, {}};
      fhicl::Sequence<std::string> skipEvent{Name{"SkipEvent"}, {}};
      fhicl::Sequence<std::string> failModule{Name{"FailModule"}, {}};
      fhicl::Sequence<std::string> failPath{Name{"FailPath"}, {}};
    };
    using Parameters = fhicl::Table<Config>;
    explicit Scheduler(Parameters const& p);

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

  private:
    int const nThreads_;
    int const nSchedules_;
    bool const handleEmptyRuns_;
    bool const handleEmptySubRuns_;
    bool const errorOnMissingConsumes_;
    tbb::task_scheduler_init tbbManager_{tbb::task_scheduler_init::deferred};
  };
}

#endif
