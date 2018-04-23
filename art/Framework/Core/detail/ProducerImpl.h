#ifndef art_Framework_Core_ProducerImpl_h
#define art_Framework_Core_ProducerImpl_h
// vim: set sw=2 expandtab :

//
// The base class of "modules" whose main purpose is to
// insert new EDProducts into an Event.
//

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/ProducerBase.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Principal/DataViewImpl.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Principal/WorkerParams.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/ScheduleID.h"
#include "art/Utilities/CPCSentry.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "fhiclcpp/ParameterSet.h"

#include <atomic>
#include <cstddef>
#include <memory>
#include <string>

namespace art {
  namespace detail {

    class ProducerImpl : public ProducerBase {
    public:
      template <typename UserConfig, typename KeysToIgnore = void>
      using Table = ProducerBase::Table<UserConfig, KeysToIgnore>;

    public: // MEMBER FUNCTIONS -- Special Member Functions
      virtual ~ProducerImpl() noexcept;
      ProducerImpl();
      ProducerImpl(ProducerImpl const&) = delete;
      ProducerImpl(ProducerImpl&&) = delete;
      ProducerImpl& operator=(ProducerImpl const&) = delete;
      ProducerImpl& operator=(ProducerImpl&&) = delete;

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
                   ScheduleID const,
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


#endif /* art_Framework_Core_detail_ProducerImpl_h */

// Local Variables:
// mode: c++
// End:
