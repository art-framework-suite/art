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
      virtual bool filterWithScheduleID(Event&, ScheduleID) = 0;

      // To be overridden by users
      virtual void beginJob();
      virtual void endJob();
      virtual void respondToOpenInputFile(FileBlock const&);
      virtual void respondToCloseInputFile(FileBlock const&);
      virtual void respondToOpenOutputFiles(FileBlock const&);
      virtual void respondToCloseOutputFiles(FileBlock const&);
      virtual bool beginRun(Run&);
      virtual bool endRun(Run&);
      virtual bool beginSubRun(SubRun&);
      virtual bool endSubRun(SubRun&);

    private:
      bool checkPutProducts_{true};
    };

  } // namespace detail
} // namespace art

#endif /* art_Framework_Core_detail_Filter_h */

// Local Variables:
// mode: c++
// End:
