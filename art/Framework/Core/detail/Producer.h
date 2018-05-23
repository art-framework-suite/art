#ifndef art_Framework_Core_detail_Producer_h
#define art_Framework_Core_detail_Producer_h
// vim: set sw=2 expandtab :

//=====================================================
// The base class of modules whose main purpose is to
// insert products into an Event.
//=====================================================

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

    class Producer : public Modifier {
    public:
      template <typename UserConfig, typename KeysToIgnore = void>
      using Table = Modifier::Table<UserConfig, KeysToIgnore>;

      virtual ~Producer() noexcept;
      Producer();
      explicit Producer(fhicl::ParameterSet const&);
      Producer(Producer const&) = delete;
      Producer(Producer&&) = delete;
      Producer& operator=(Producer const&) = delete;
      Producer& operator=(Producer&&) = delete;

      // Interface provided for the worker.
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
      virtual void beginRunWithServices(Run&, Services const&) = 0;
      virtual void endRunWithServices(Run&, Services const&) = 0;
      virtual void beginSubRunWithServices(SubRun&, Services const&) = 0;
      virtual void endSubRunWithServices(SubRun&, Services const&) = 0;
      virtual void produceWithServices(Event&, Services const&) = 0;

      bool checkPutProducts_{true};
    };

  } // namespace detail
} // namespace art

#endif /* art_Framework_Core_detail_Producer_h */

// Local Variables:
// mode: c++
// End:
