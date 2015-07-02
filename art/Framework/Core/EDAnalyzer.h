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
#include "fhiclcpp/types/Table.h"

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
    typedef EDAnalyzer ModuleType;
    typedef WorkerT<EDAnalyzer> WorkerType;

    // Configuration
    struct baseConfig {
      fhicl::Atom<std::string> module_type { fhicl::Key("module_type") };
      fhicl::Table<EventObserver::EOConfig> eoConfig {
        fhicl::Key("SelectEvents"),
          fhicl::Comment("The 'SelectEvents' table below is optional")
          };
    };

    template <typename T>
    struct fullConfig : baseConfig, T {}; // Multiple inheritance
                                          // allows 'module-type' key
                                          // to be listed first in
                                          // description.

    template < typename userConfig >
    class Table : public fhicl::Table<fullConfig<userConfig>> {
    public:

      Table(){}

      Table( fhicl::ParameterSet const& pset ) : Table()
      {
        std::set<std::string> const keys_to_ignore = { "module_label" };
        this->validate_ParameterSet( pset, keys_to_ignore );
        this->set_PSet( pset );
      }

    };

    template <typename Config>
    explicit EDAnalyzer( Table<Config> const& config )
      : EventObserver( config.get_PSet() )
      , EngineCreator()
      , moduleDescription_()
      , current_context_(0)
    {}

    explicit EDAnalyzer( fhicl::ParameterSet const& pset )
      : EventObserver( pset )
      , EngineCreator()
      , moduleDescription_()
      , current_context_(0)
    {}

    virtual ~EDAnalyzer();

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
