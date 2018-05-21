////////////////////////////////////////////////////////////////////////
// BlockingPrescaler.
//
// Accept m in n events with offset. So, for blockSize (m) = 5,
// stepSize (n) = 7 and offset = 4, the filter will accept events 5-9
// inclusive, 12-16 inclusive, 19-23 inclusive, etc.
//
// Note that BlockingPrescaler prescales based on the number of events
// seen by this module, *not* the event number as recorded by EventID.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "fhiclcpp/types/Atom.h"

using namespace fhicl;

namespace art {
  class BlockingPrescaler;
}

// ======================================================================

class art::BlockingPrescaler : public SharedFilter {
public:
  struct Config {
    Atom<size_t> blockSize{Name("blockSize"), 1};
    Atom<size_t> stepSize{
      Name("stepSize"),
      Comment(
        "The value of 'stepSize' cannot be less than that of 'blockSize'.")};
    Atom<size_t> offset{Name("offset"), 0};
  };

  using Parameters = Table<Config>;
  explicit BlockingPrescaler(Parameters const&);

private:
  bool filter(Event&, ScheduleID) override;

  size_t count_{};
  size_t const m_; // accept m in n (sequentially).
  size_t const n_;
  size_t const offset_; // First accepted event is 1 + offset.

}; // BlockingPrescaler

// ======================================================================

art::BlockingPrescaler::BlockingPrescaler(Parameters const& config)
  : SharedFilter{config}
  , m_{config().blockSize()}
  , n_{config().stepSize()}
  , offset_{config().offset()}
{
  if (n_ < m_) {
    throw art::Exception{art::errors::Configuration,
                         "There was an error configuring Blocking Prescaler.\n"}
      << "The specified step size (" << n_ << ") is less than the block size ("
      << m_ << ")\n";
  }
  // See note below.
  serialize();
}

bool
art::BlockingPrescaler::filter(Event&, ScheduleID)
{
  // This sequence of operations/comparisons must be serialized.
  // Changing 'count_' to be of type std::atomic<size_t> will not
  // help.
  bool const result{(count_ >= offset_) && ((count_ - offset_) % n_) < m_};
  ++count_;
  return result;
}

DEFINE_ART_MODULE(art::BlockingPrescaler)
