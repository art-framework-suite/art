#ifndef art_Framework_Core_EDAnalyzer_h
#define art_Framework_Core_EDAnalyzer_h

// ======================================================================
//
// EDAnalyzer - the base class for all analyzer "modules".
//
// ======================================================================

#include "art/Framework/Core/EngineCreator.h"
#include "art/Framework/Core/EventObserver.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/WorkerT.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/OptionalTable.h"
#include "fhiclcpp/types/Table.h"
#include "fhiclcpp/types/TableFragment.h"

#include <memory>
#include <ostream>
#include <string>

// ----------------------------------------------------------------------

namespace art
{
  class MasterProductRegistry;

  class EDAnalyzer
    : public EventObserver,
      public EngineCreator
  {
  public:
    template <typename T> friend class WorkerT;

    using WorkerType = WorkerT<EDAnalyzer>;
    using ModuleType = EDAnalyzer;

    // Configuration
    template <typename T>
    struct FullConfig {
      fhicl::Atom<std::string> module_type { fhicl::Name("module_type") };
      fhicl::OptionalTable<EventObserver::EOConfig> eoConfig { fhicl::Name("SelectEvents") };
      fhicl::TableFragment<T> user;
    };

    template < typename UserConfig >
    class Table {
    public:

      Table() = default;

      Table( fhicl::ParameterSet const& pset ) : Table()
      {
        std::set<std::string> const keys_to_ignore { "module_label" };
        fullConfig_.validate_ParameterSet( pset, keys_to_ignore );
      }

      auto const& operator()() const { return fullConfig_().user(); }

      auto const& eoTable()  const { return fullConfig_().eoConfig; }
      auto const& get_PSet() const { return fullConfig_.get_PSet(); }

      void print_allowed_configuration(std::ostream& os, std::string const& prefix) const
      {
        fullConfig_.print_allowed_configuration(os, prefix);
      }

    private:
      fhicl::Table<FullConfig<UserConfig>> fullConfig_ { fhicl::Name{"<module_label>"} };
    };

    template <typename Config>
    explicit EDAnalyzer(Table<Config> const& config)
      : EventObserver{config.eoTable()}
      , EngineCreator{}
      , moduleDescription_{}
      , current_context_{0}
    {}

    explicit EDAnalyzer(fhicl::ParameterSet const& pset)
      : EventObserver(pset)
      , EngineCreator()
      , moduleDescription_()
      , current_context_(0)
    {}

    virtual ~EDAnalyzer() = default;

    std::string workerType() const {return "WorkerT<EDAnalyzer>";}

  protected:
    // The returned pointer will be null unless the this is currently
    // executing its event loop function ('analyze').
    CurrentProcessingContext const* currentContext() const;

  private:
    bool doEvent(EventPrincipal const& ep,
                   CurrentProcessingContext const* cpc);
    void doBeginJob();
    void doEndJob();
    bool doBeginRun(RunPrincipal const& rp,
                   CurrentProcessingContext const* cpc);
    bool doEndRun(RunPrincipal const& rp,
                   CurrentProcessingContext const* cpc);
    bool doBeginSubRun(SubRunPrincipal const& srp,
                   CurrentProcessingContext const* cpc);
    bool doEndSubRun(SubRunPrincipal const& srp,
                   CurrentProcessingContext const* cpc);
    void doRespondToOpenInputFile(FileBlock const& fb);
    void doRespondToCloseInputFile(FileBlock const& fb);
    void doRespondToOpenOutputFiles(FileBlock const& fb);
    void doRespondToCloseOutputFiles(FileBlock const& fb);

    virtual void analyze(Event const&) = 0;
    virtual void beginJob(){}
    virtual void endJob(){}
    virtual void reconfigure(fhicl::ParameterSet const&);
    virtual void beginRun(Run const&){}
    virtual void endRun(Run const&){}
    virtual void beginSubRun(SubRun const&){}
    virtual void endSubRun(SubRun const&){}
    virtual void respondToOpenInputFile(FileBlock const&) {}
    virtual void respondToCloseInputFile(FileBlock const&) {}
    virtual void respondToOpenOutputFiles(FileBlock const&) {}
    virtual void respondToCloseOutputFiles(FileBlock const&) {}

    void setModuleDescription(ModuleDescription const& md) {
      moduleDescription_ = md;
    }

    ModuleDescription moduleDescription_;
    CurrentProcessingContext const* current_context_;
  };  // EDAnalyzer

}  // art

// ======================================================================

#endif /* art_Framework_Core_EDAnalyzer_h */

// Local Variables:
// mode: c++
// End:
