#ifndef art_Framework_Core_EDAnalyzer_h
#define art_Framework_Core_EDAnalyzer_h
// vim: set sw=2 expandtab :

//
//  The base class for all analyzer modules.
//

#include "art/Framework/Core/EventObserverBase.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/IgnoreModuleLabel.h"
#include "art/Framework/Principal/fwd.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/ConfigurationTable.h"
#include "fhiclcpp/types/KeysToIgnore.h"
#include "fhiclcpp/types/OptionalTable.h"
#include "fhiclcpp/types/Table.h"
#include "fhiclcpp/types/TableFragment.h"
#include "hep_concurrency/SerialTaskQueueChain.h"

#include <atomic>
#include <cstddef>
#include <memory>
#include <ostream>
#include <string>

namespace art {

class EDAnalyzer : public EventObserverBase {

  // Allow the WorkerT<T> ctor to call setModuleDescription() and workerType().
  template <typename T> friend class WorkerT;

public: // TYPES

  // These two are needed by the module macros.
  using WorkerType = WorkerT<EDAnalyzer>;
  using ModuleType = EDAnalyzer;

public: // CONFIGURATION

  template <typename UserConfig, typename UserKeysToIgnore = void>
  class Table : public fhicl::ConfigurationTable {

  private: // TYPES

    template <typename T>
    struct FullConfig {

      fhicl::Atom<std::string>
      module_type{ fhicl::Name("module_type") };

      fhicl::TableFragment<EventObserverBase::EOConfig>
      eoConfig;

      fhicl::TableFragment<T>
      user;

    };

    using KeysToIgnore_t = std::conditional_t <
                           std::is_void<UserKeysToIgnore>::value,
                           detail::IgnoreModuleLabel,
                           fhicl::KeysToIgnore<detail::IgnoreModuleLabel, UserKeysToIgnore >>;

  public: // MEMBER FUNCTIONS -- Special Member Functions

    explicit
    Table(fhicl::Name&& name)
      : fullConfig_{std::move(name)}
    {
    }

    Table(fhicl::ParameterSet const& pset)
      : fullConfig_{pset}
    {
    }

  public: // MEMBER FUNCTIONS -- User-facing API

    auto const&
    operator()() const
    {
      return fullConfig_().user();
    }

    auto const&
    eoFragment() const
    {
      return fullConfig_().eoConfig();
    }

    auto const&
    get_PSet() const
    {
      return fullConfig_.get_PSet();
    }

    void
    print_allowed_configuration(std::ostream& os, std::string const& prefix) const
    {
      fullConfig_.print_allowed_configuration(os, prefix);
    }

  private: // MEMBER FUNCTIONS -- Implementation details.

    cet::exempt_ptr<fhicl::detail::ParameterBase const>
    get_parameter_base() const override
    {
      return &fullConfig_;
    }

  private: // MEMBER DATA

    fhicl::Table<FullConfig<UserConfig>, KeysToIgnore_t>
    fullConfig_;

  };

public: // MEMBER FUNCTIONS -- Special Member Functions

  virtual
  ~EDAnalyzer();

  explicit
  EDAnalyzer(fhicl::ParameterSet const& pset);

  template <typename Config>
  explicit
  EDAnalyzer(Table<Config> const& config)
    : EventObserverBase{config.eoFragment().selectEvents(), config.get_PSet()}
  {
  }

public: // MEMBER FUNCTIONS --

  std::string
  workerType() const;

private: // MEMBER FUNCTIONS -- API required by EventProcessor, Schedule, and EndPathExecutor

  virtual
  void
  doBeginJob();

  void
  doEndJob();

  void
  doRespondToOpenInputFile(FileBlock const& fb);

  void
  doRespondToCloseInputFile(FileBlock const& fb);

  void
  doRespondToOpenOutputFiles(FileBlock const& fb);

  void
  doRespondToCloseOutputFiles(FileBlock const& fb);

  bool
  doBeginRun(RunPrincipal& rp, cet::exempt_ptr<CurrentProcessingContext const> cpc);

  bool
  doEndRun(RunPrincipal& rp, cet::exempt_ptr<CurrentProcessingContext const> cpc);

  bool
  doBeginSubRun(SubRunPrincipal& srp, cet::exempt_ptr<CurrentProcessingContext const> cpc);

  bool
  doEndSubRun(SubRunPrincipal& srp, cet::exempt_ptr<CurrentProcessingContext const> cpc);

  bool
  doEvent(EventPrincipal& ep, int streamIndex, CurrentProcessingContext const* cpc,
          std::atomic<std::size_t>& counts_run,
          std::atomic<std::size_t>& counts_passed,
          std::atomic<std::size_t>& counts_failed);

protected: // MEMBER FUNCTIONS -- Implementation API, intended to be provided by derived classes.

  virtual
  void
  reconfigure(fhicl::ParameterSet const&);

  virtual
  void
  beginJob();

  virtual
  void
  endJob();

  virtual
  void
  respondToOpenInputFile(FileBlock const&);

  virtual
  void
  respondToCloseInputFile(FileBlock const&);

  virtual
  void
  respondToOpenOutputFiles(FileBlock const&);

  virtual
  void
  respondToCloseOutputFiles(FileBlock const&);

  virtual
  void
  beginRun(Run const&);

  virtual
  void
  endRun(Run const&);

  virtual
  void
  beginSubRun(SubRun const&);

  virtual
  void
  endSubRun(SubRun const&);

  virtual
  void
  analyze(Event const&) = 0;

  //virtual
  //void
  //analyze_in_stream(Event const&, int streamIndex);

};

namespace one {

class EDAnalyzer : public art::EDAnalyzer {

  // Allow the WorkerT<T> ctor to call setModuleDescription() and workerType().
  template <typename T> friend class WorkerT;

public: // MEMBER FUNCTIONS -- Special Member Functions

  virtual
  ~EDAnalyzer();

  explicit
  EDAnalyzer(fhicl::ParameterSet const&);

  template <typename Config>
  explicit
  EDAnalyzer(Table<Config> const& config)
    : art::EDAnalyzer(config)
  {
  }

private: // MEMBER FUNCTIONS -- API required by EventProcessor, Schedule, and EndPathExecutor

  void
  doBeginJob() override;

};

} // namespace one

namespace stream {

class EDAnalyzer : public art::EDAnalyzer {

  // Allow the WorkerT<T> ctor to call setModuleDescription() and workerType().
  template <typename T> friend class WorkerT;

public: // MEMBER FUNCTIONS -- Special Member Functions

  virtual
  ~EDAnalyzer();

  explicit
  EDAnalyzer(fhicl::ParameterSet const&);

  template <typename Config>
  explicit
  EDAnalyzer(Table<Config> const& config)
    : art::EDAnalyzer(config)
  {
  }

private: // MEMBER FUNCTIONS -- API required by EventProcessor, Schedule, and EndPathExecutor

  void
  doBeginJob() override;

};

} // namespace stream

namespace global {

class EDAnalyzer : public art::EDAnalyzer {

  // Allow the WorkerT<T> ctor to call setModuleDescription() and workerType().
  template <typename T> friend class WorkerT;

public: // MEMBER FUNCTIONS -- Special Member Functions

  virtual
  ~EDAnalyzer();

  explicit
  EDAnalyzer(fhicl::ParameterSet const&);

  template <typename Config>
  explicit
  EDAnalyzer(Table<Config> const& config)
    : art::EDAnalyzer(config)
  {
  }

private: // MEMBER FUNCTIONS -- API required by EventProcessor, Schedule, and EndPathExecutor

  void
  doBeginJob() override;

};

} // namespace global

template <typename T>
inline
std::ostream&
operator<<(std::ostream& os, EDAnalyzer::Table<T> const& t)
{
  std::ostringstream config;
  t.print_allowed_configuration(config, std::string(3, ' '));
  return os << config.str();
}

} // namespace art

#endif /* art_Framework_Core_EDAnalyzer_h */

// Local Variables:
// mode: c++
// End:
