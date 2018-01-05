#ifndef art_Framework_Core_EDAnalyzer_h
#define art_Framework_Core_EDAnalyzer_h

// ======================================================================
//
// EDAnalyzer - the base class for all analyzer "modules".
//
// ======================================================================

#include "art/Framework/Core/EngineCreator.h"
#include "art/Framework/Core/EventObserverBase.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/IgnoreModuleLabel.h"
#include "art/Framework/Principal/Consumer.h"
#include "art/Framework/Principal/fwd.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/ConfigurationTable.h"
#include "fhiclcpp/types/KeysToIgnore.h"
#include "fhiclcpp/types/OptionalTable.h"
#include "fhiclcpp/types/Table.h"
#include "fhiclcpp/types/TableFragment.h"

#include <memory>
#include <ostream>
#include <string>

// ----------------------------------------------------------------------

namespace art {
  class EDAnalyzer : public EventObserverBase,
                     public Consumer,
                     public EngineCreator {
  public:
    template <typename T>
    friend class WorkerT;

    using WorkerType = WorkerT<EDAnalyzer>;
    using ModuleType = EDAnalyzer;

    //---------------------------------------------------------------------------
    // Configuration

    template <typename UserConfig, typename UserKeysToIgnore = void>
    class Table : public fhicl::ConfigurationTable {
    public:
      explicit Table(fhicl::Name&& name) : fullConfig_{std::move(name)} {}
      Table(fhicl::ParameterSet const& pset) : fullConfig_{pset} {}

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
      print_allowed_configuration(std::ostream& os,
                                  std::string const& prefix) const
      {
        fullConfig_.print_allowed_configuration(os, prefix);
      }

    private:
      template <typename T>
      struct FullConfig {
        fhicl::Atom<std::string> module_type{fhicl::Name("module_type")};
        fhicl::TableFragment<EventObserverBase::EOConfig> eoConfig;
        fhicl::TableFragment<T> user;
      };

      using KeysToIgnore_t = std::conditional_t<
        std::is_void<UserKeysToIgnore>::value,
        detail::IgnoreModuleLabel,
        fhicl::KeysToIgnore<detail::IgnoreModuleLabel, UserKeysToIgnore>>;

      fhicl::Table<FullConfig<UserConfig>, KeysToIgnore_t> fullConfig_;
      cet::exempt_ptr<fhicl::detail::ParameterBase const>
      get_parameter_base() const override
      {
        return &fullConfig_;
      }
    };

    //---------------------------------------------------------------------------

    template <typename Config>
    explicit EDAnalyzer(Table<Config> const& config)
      : EventObserverBase{config.eoFragment().selectEvents(), config.get_PSet()}
    {}

    explicit EDAnalyzer(fhicl::ParameterSet const& pset);

    virtual ~EDAnalyzer() = default;

    std::string
    workerType() const
    {
      return "WorkerT<EDAnalyzer>";
    }

  protected:
    // The returned pointer will be null unless the this is currently
    // executing its event loop function ('analyze').
    CurrentProcessingContext const* currentContext() const;

  private:
    using CPC_exempt_ptr = cet::exempt_ptr<CurrentProcessingContext const>;

    bool doEvent(EventPrincipal const& ep,
                 CPC_exempt_ptr cpc,
                 CountingStatistics&);
    void doBeginJob();
    void doEndJob();
    bool doBeginRun(RunPrincipal const& rp, CPC_exempt_ptr cpc);
    bool doEndRun(RunPrincipal const& rp, CPC_exempt_ptr cpc);
    bool doBeginSubRun(SubRunPrincipal const& srp, CPC_exempt_ptr cpc);
    bool doEndSubRun(SubRunPrincipal const& srp, CPC_exempt_ptr cpc);
    void doRespondToOpenInputFile(FileBlock const& fb);
    void doRespondToCloseInputFile(FileBlock const& fb);
    void doRespondToOpenOutputFiles(FileBlock const& fb);
    void doRespondToCloseOutputFiles(FileBlock const& fb);

    virtual void analyze(Event const&) = 0;
    virtual void
    beginJob()
    {}
    virtual void
    endJob()
    {}
    virtual void
    beginRun(Run const&)
    {}
    virtual void
    endRun(Run const&)
    {}
    virtual void
    beginSubRun(SubRun const&)
    {}
    virtual void
    endSubRun(SubRun const&)
    {}
    virtual void
    respondToOpenInputFile(FileBlock const&)
    {}
    virtual void
    respondToCloseInputFile(FileBlock const&)
    {}
    virtual void
    respondToOpenOutputFiles(FileBlock const&)
    {}
    virtual void
    respondToCloseOutputFiles(FileBlock const&)
    {}

    void
    setModuleDescription(ModuleDescription const& md)
    {
      moduleDescription_ = md;
      // Since the module description in the Consumer base class is
      // owned by pointer, we must give it the owned object of this
      // class--i.e. moduleDescription_, not md.
      Consumer::setModuleDescription(moduleDescription_);
    }

    ModuleDescription moduleDescription_{};
    CPC_exempt_ptr current_context_{nullptr};
  }; // EDAnalyzer

  template <typename T>
  inline std::ostream&
  operator<<(std::ostream& os, EDAnalyzer::Table<T> const& t)
  {
    std::ostringstream config;
    t.print_allowed_configuration(config, std::string(3, ' '));
    return os << config.str();
  }

} // art

  // ======================================================================

#endif /* art_Framework_Core_EDAnalyzer_h */

// Local Variables:
// mode: c++
// End:
