#ifndef art_Framework_Core_detail_Producer_h
#define art_Framework_Core_detail_Producer_h
// vim: set sw=2 expandtab :

//=====================================================
// The base class of modules whose main purpose is to
// insert products into an Event.
//=====================================================

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/ProducerBase.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/CPCSentry.h"
#include "art/Utilities/ScheduleID.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"

#include <atomic>
#include <cstddef>
#include <string>

namespace art {
  namespace detail {

    class Producer : public ProducerBase {
    public:
      template <typename UserConfig, typename KeysToIgnore = void>
      using Table = ProducerBase::Table<UserConfig, KeysToIgnore>;

    public: // MEMBER FUNCTIONS -- Special Member Functions
      virtual ~Producer() noexcept;
      Producer();
      Producer(Producer const&) = delete;
      Producer(Producer&&) = delete;
      Producer& operator=(Producer const&) = delete;
      Producer& operator=(Producer&&) = delete;

      // The doBeginJob function depends on the module threading type.
      void doBeginJob() = delete;
      void doEndJob();
      void doRespondToOpenInputFile(FileBlock const& fb);
      void doRespondToCloseInputFile(FileBlock const& fb);
      void doRespondToOpenOutputFiles(FileBlock const& fb);
      void doRespondToCloseOutputFiles(FileBlock const& fb);
      bool doBeginRun(RunPrincipal& rp,
                      cet::exempt_ptr<CurrentProcessingContext const> cpc);
      bool doEndRun(RunPrincipal& rp,
                    cet::exempt_ptr<CurrentProcessingContext const> cpc);
      bool doBeginSubRun(SubRunPrincipal& srp,
                         cet::exempt_ptr<CurrentProcessingContext const> cpc);
      bool doEndSubRun(SubRunPrincipal& srp,
                       cet::exempt_ptr<CurrentProcessingContext const> cpc);
      bool doEvent(EventPrincipal& ep,
                   ScheduleID,
                   CurrentProcessingContext const* cpc,
                   std::atomic<std::size_t>& counts_run,
                   std::atomic<std::size_t>& counts_passed,
                   std::atomic<std::size_t>& counts_failed);

      void failureToPutProducts(ModuleDescription const& md);

    protected:
      virtual void beginJob();
      virtual void endJob();
      virtual void respondToOpenInputFile(FileBlock const&);
      virtual void respondToCloseInputFile(FileBlock const&);
      virtual void respondToOpenOutputFiles(FileBlock const&);
      virtual void respondToCloseOutputFiles(FileBlock const&);
      virtual void beginRun(Run&);
      virtual void endRun(Run&);
      virtual void beginSubRun(SubRun&);
      virtual void endSubRun(SubRun&);
      virtual void produce(Event&) = 0;

    private:
      bool checkPutProducts_{true};
    };

  } // namespace detail
} // namespace art

#endif /* art_Framework_Core_detail_Producer_h */

// Local Variables:
// mode: c++
// End:
