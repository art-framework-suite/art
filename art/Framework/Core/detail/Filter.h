#ifndef art_Framework_Core_detail_Filter_h
#define art_Framework_Core_detail_Filter_h
// vim: set sw=2 expandtab :

//==================================================================
// The base class of all modules used to control the flow of
// processing in a trigger path.  Filters can also insert products
// into the event.  These products should be informational products
// about the filter decision.
//==================================================================

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/Modifier.h"
#include "art/Framework/Core/Services.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Utilities/ScheduleID.h"

#include <cstddef>

namespace art {
  namespace detail {
    class Filter : public Modifier {
    public:
      static constexpr bool Pass{true};
      static constexpr bool Fail{false};

      template <typename UserConfig>
      using Table = Modifier::Table<UserConfig>;

      virtual ~Filter() noexcept;
      Filter();
      explicit Filter(fhicl::ParameterSet const&);
      Filter(Filter const&) = delete;
      Filter(Filter&&) = delete;
      Filter& operator=(Filter const&) = delete;
      Filter& operator=(Filter&&) = delete;

      void doBeginJob();
      void doEndJob();
      void doRespondToOpenInputFile(FileBlock const& fb);
      void doRespondToCloseInputFile(FileBlock const& fb);
      void doRespondToOpenOutputFiles(FileBlock const& fb);
      void doRespondToCloseOutputFiles(FileBlock const& fb);
      bool doBeginRun(RunPrincipal& rp, ModuleContext const&);
      bool doEndRun(RunPrincipal& rp, ModuleContext const& mc);
      bool doBeginSubRun(SubRunPrincipal& srp, ModuleContext const& mc);
      bool doEndSubRun(SubRunPrincipal& srp, ModuleContext const& mc);
      bool doEvent(EventPrincipal& ep,
                   ModuleContext const& mc,
                   std::atomic<std::size_t>& counts_run,
                   std::atomic<std::size_t>& counts_passed,
                   std::atomic<std::size_t>& counts_failed);

    private:
      void failureToPutProducts(ModuleDescription const& md);
      virtual void setupQueues() = 0;
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
      virtual bool beginRunWithServices(Run&, Services const&) = 0;
      virtual bool endRunWithServices(Run&, Services const&) = 0;
      virtual bool beginSubRunWithServices(SubRun&, Services const&) = 0;
      virtual bool endSubRunWithServices(SubRun&, Services const&) = 0;
      virtual bool filterWithServices(Event&, Services const&) = 0;

      bool checkPutProducts_{true};
    };

  } // namespace detail
} // namespace art

#endif /* art_Framework_Core_detail_Filter_h */

// Local Variables:
// mode: c++
// End:
