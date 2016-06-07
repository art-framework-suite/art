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
      fhicl::TableFragment<EventObserver::EOConfig> eoConfig;
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

      auto const& eoFragment()  const { return fullConfig_().eoConfig(); }
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
      : EventObserver{config.eoFragment().selectEvents(), config.get_PSet()}
      , EngineCreator{}
    {}

    explicit EDAnalyzer(fhicl::ParameterSet const& pset)
      : EventObserver{pset}
      , EngineCreator{}
    {}

    virtual ~EDAnalyzer() = default;

    std::string workerType() const {return "WorkerT<EDAnalyzer>";}

  protected:
    // The returned pointer will be null unless the this is currently
    // executing its event loop function ('analyze').
    CurrentProcessingContext const* currentContext() const;

  private:

    using CPC_exempt_ptr = cet::exempt_ptr<CurrentProcessingContext const>;

    bool doEvent(EventPrincipal const& ep, CPC_exempt_ptr cpc);
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
    void doRespondToOpenOutputFile();
    void doRespondToCloseOutputFile();

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
    virtual void respondToOpenOutputFile() {}
    virtual void respondToCloseOutputFile() {}

    void setModuleDescription(ModuleDescription const& md) {
      moduleDescription_ = md;
    }

    ModuleDescription moduleDescription_ {};
    CPC_exempt_ptr current_context_ {nullptr};
  };  // EDAnalyzer

  template <typename T, typename U>
  inline decltype(auto) operator<<(T&& t, EDAnalyzer::Table<U> const& u)
  {
    std::ostringstream oss;
    u.print_allowed_configuration(oss, std::string(3,' '));
    return std::forward<T>(t) << oss.str();
  }

}  // art

// ======================================================================

#endif /* art_Framework_Core_EDAnalyzer_h */

// Local Variables:
// mode: c++
// End:
