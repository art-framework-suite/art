#ifndef art_Framework_Core_detail_Analyzer_h
#define art_Framework_Core_detail_Analyzer_h
// vim: set sw=2 expandtab :

//
//  The base class for all analyzer modules.
//

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/Observer.h"
#include "art/Framework/Core/Services.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/ImplicitConfigs.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/ScheduleID.h"
#include "fhiclcpp/types/ConfigurationTable.h"
#include "fhiclcpp/types/KeysToIgnore.h"
#include "fhiclcpp/types/OptionalTable.h"
#include "fhiclcpp/types/Table.h"
#include "fhiclcpp/types/TableFragment.h"

#include <atomic>
#include <cstddef>
#include <ostream>
#include <string>

namespace art {
  namespace detail {
    class Analyzer : public Observer {
    public: // CONFIGURATION
      template <typename UserConfig, typename UserKeysToIgnore = void>
      class Table : public fhicl::ConfigurationTable {
      private: // TYPES
        template <typename T>
        struct FullConfig {
          fhicl::Atom<std::string> module_type{
            detail::ModuleConfig::plugin_type()};
          fhicl::TableFragment<Observer::EOConfig> eoConfig;
          fhicl::TableFragment<T> user;
        };

        using KeysToIgnore_t = std::conditional_t<
          std::is_void<UserKeysToIgnore>::value,
          detail::ModuleConfig::IgnoreKeys,
          fhicl::KeysToIgnore<detail::ModuleConfig::IgnoreKeys,
                              UserKeysToIgnore>>;

      public: // MEMBER FUNCTIONS -- Special Member Functions
        explicit Table(fhicl::Name&& name) : fullConfig_{std::move(name)} {}
        Table(fhicl::ParameterSet const& pset) : fullConfig_{pset} {}

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
        print_allowed_configuration(std::ostream& os,
                                    std::string const& prefix) const
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
        fhicl::Table<FullConfig<UserConfig>, KeysToIgnore_t> fullConfig_;
      };

    public: // MEMBER FUNCTIONS -- Special Member Functions
      virtual ~Analyzer() noexcept;
      explicit Analyzer(fhicl::ParameterSet const& pset);
      template <typename Config>
      explicit Analyzer(Table<Config> const& config)
        : Observer{config.eoFragment().selectEvents(), config.get_PSet()}
      {}

      void doBeginJob();
      void doEndJob();
      void doRespondToOpenInputFile(FileBlock const& fb);
      void doRespondToCloseInputFile(FileBlock const& fb);
      void doRespondToOpenOutputFiles(FileBlock const& fb);
      void doRespondToCloseOutputFiles(FileBlock const& fb);
      bool doBeginRun(RunPrincipal& rp, ModuleContext const& mc);
      bool doEndRun(RunPrincipal& rp, ModuleContext const& mc);
      bool doBeginSubRun(SubRunPrincipal& srp, ModuleContext const& mc);
      bool doEndSubRun(SubRunPrincipal& srp, ModuleContext const& mc);
      bool doEvent(EventPrincipal& ep,
                   ModuleContext const& mc,
                   std::atomic<std::size_t>& counts_run,
                   std::atomic<std::size_t>& counts_passed,
                   std::atomic<std::size_t>& counts_failed);

    private:
      virtual void setupQueues() = 0;
      virtual void analyzeWithServices(Event const&, Services const&) = 0;
      virtual void beginJobWithServices(Services const&) = 0;
      virtual void endJobWithServices(Services const&) = 0;
      virtual void respondToOpenInputFileWithServices(FileBlock const&,
                                                      Services const&) = 0;
      virtual void respondToCloseInputFileWithServices(FileBlock const&,
                                                       Services const&) = 0;
      virtual void respondToOpenOutputFilesWithServices(FileBlock const&,
                                                        Services const&) = 0;
      virtual void respondToCloseOutputFilesWithServices(FileBlock const&,
                                                         Services const&) = 0;
      virtual void beginRunWithServices(Run const&, Services const&) = 0;
      virtual void endRunWithServices(Run const&, Services const&) = 0;
      virtual void beginSubRunWithServices(SubRun const&, Services const&) = 0;
      virtual void endSubRunWithServices(SubRun const&, Services const&) = 0;
    };

    template <typename T>
    inline std::ostream&
    operator<<(std::ostream& os, Analyzer::Table<T> const& t)
    {
      std::ostringstream config;
      t.print_allowed_configuration(config, std::string(3, ' '));
      return os << config.str();
    }

  } // namespace detail
} // namespace art

#endif /* art_Framework_Core_detail_Analyzer_h */

// Local Variables:
// mode: c++
// End:
