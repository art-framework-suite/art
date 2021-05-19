#ifndef art_Framework_EventProcessor_Scheduler_h
#define art_Framework_EventProcessor_Scheduler_h

#include "art/Framework/Principal/Actions.h"
#include "art/Utilities/GlobalTaskGroup.h"
#include "art/Utilities/ScheduleID.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/OptionalTable.h"
#include "fhiclcpp/types/Table.h"
#include "fhiclcpp/types/TableFragment.h"

#include <string>

namespace art {
  class Scheduler {
  public:
    struct Config {
      static constexpr unsigned
      kb()
      {
        return 1024;
      }
      static constexpr unsigned
      mb()
      {
        return kb() * kb();
      }

      // FIXME: The defaults specified here should agree with those
      //        specified in the program-options handlers.
      using Name = fhicl::Name;
      using Comment = fhicl::Comment;
      fhicl::Atom<unsigned> num_threads{Name{"num_threads"}, 1};
      fhicl::Atom<ScheduleID::size_type> num_schedules{Name{"num_schedules"},
                                                       1};
      fhicl::Atom<unsigned> stack_size{
        Name{"stack_size"},
        Comment{"The stack size (in bytes) that the TBB scheduler will use for "
                "its threads.\n"
                "The default stack size TBB specifies is 1 MB, which can be "
                "inadequate for\n"
                "various workflows.  Because of that, art picks a default of "
                "10 MB, which\n"
                "more closely approximates the stack size of the main thread."},
        10 * mb()};
      fhicl::Atom<bool> handleEmptyRuns{Name{"handleEmptyRuns"}, true};
      fhicl::Atom<bool> handleEmptySubRuns{Name{"handleEmptySubRuns"}, true};
      fhicl::Atom<bool> errorOnMissingConsumes{Name{"errorOnMissingConsumes"},
                                               false};
      fhicl::Atom<bool> errorOnSIGINT{Name{"errorOnSIGINT"}, true};
      fhicl::Atom<bool> wantSummary{Name{"wantSummary"}, false};
      fhicl::Atom<bool> pruneConfig{Name{"pruneConfig"}, true};
      fhicl::Atom<bool> reportUnused{Name{"reportUnused"}, true};
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

    unsigned
    num_threads() const noexcept
    {
      return nThreads_;
    }
    ScheduleID::size_type
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

    std::unique_ptr<GlobalTaskGroup> global_task_group();

  private:
    // A table of responses to be taken on reception of thrown
    // exceptions.
    ActionTable actionTable_;
    unsigned const nThreads_;
    unsigned const nSchedules_;
    unsigned const stackSize_;
    bool const handleEmptyRuns_;
    bool const handleEmptySubRuns_;
    bool const errorOnMissingConsumes_;
    bool const wantSummary_;
    std::string const dataDependencyGraph_;
  };
}

#endif /* art_Framework_EventProcessor_Scheduler_h */

// Local Variables:
// mode: c++
// End:
