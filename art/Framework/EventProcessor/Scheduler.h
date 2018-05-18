#ifndef art_Framework_EventProcessor_Scheduler_h
#define art_Framework_EventProcessor_Scheduler_h

#include "art/Framework/Principal/Actions.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/OptionalDelegatedParameter.h"
#include "fhiclcpp/types/OptionalTable.h"
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
      using Comment = fhicl::Comment;
      fhicl::Atom<int> num_threads{Name{"num_threads"}};
      fhicl::Atom<int> num_schedules{Name{"num_schedules"}};
      fhicl::Atom<bool> handleEmptyRuns{Name{"handleEmptyRuns"}, true};
      fhicl::Atom<bool> handleEmptySubRuns{Name{"handleEmptySubRuns"}, true};
      fhicl::Atom<bool> errorOnMissingConsumes{Name{"errorOnMissingConsumes"},
                                               false};
      fhicl::Atom<bool> errorOnFailureToPut{Name{"errorOnFailureToPut"}, true};
      fhicl::Atom<bool> errorOnSIGINT{Name{"errorOnSIGINT"}, true};
      fhicl::Atom<bool> wantSummary{Name{"wantSummary"}, false};
      fhicl::Atom<bool> pruneConfig{Name{"pruneConfig"}, false};
      fhicl::Atom<std::string> dataDependencyGraph{Name{"dataDependencyGraph"},
                                                   {}};
      struct DebugConfig {
        fhicl::Atom<std::string> fileName{Name{"fileName"}};
        fhicl::Atom<std::string> option{Name{"option"}};
        fhicl::Atom<std::string> printMode{Name{"printMode"}};
      };
      fhicl::OptionalTable<DebugConfig> debug{
        Name{"debug"},
        Comment{
          "The entries in the 'debug' table below are filled whenever the\n"
          "command-line program options are parsed.  Any user-provided values\n"
          "will be overriden by the command-line."}};
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
    std::string const&
    dataDependencyGraph() const noexcept
    {
      return dataDependencyGraph_;
    }

    void initialize_task_manager();

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
    std::string const dataDependencyGraph_;
    tbb::task_scheduler_init tbbManager_{tbb::task_scheduler_init::deferred};
  };
}

#endif /* art_Framework_EventProcessor_Scheduler_h */

// Local Variables:
// mode: c++
// End:
