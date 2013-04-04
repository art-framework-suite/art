////////////////////////////////////////////////////////////////////////
// BlockingPrescaler.
//
// Accept m in n events with offset. So, for blockSize (m) = 5,
// stepSize (n) = 7 and offset = 4, the filter will accept events 5-9
// inclusive, 12-16 inclusive, 19-23 inclusive, etc.
//
// Note that BlockingPrescaler prescales based on the numebr of events
// seen by this module, *not* the event number as recorded by EventID.
//
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "fhiclcpp/ParameterSet.h"

namespace art {
  class BlockingPrescaler;
}

// ======================================================================

class art::BlockingPrescaler : public EDFilter {
public:
  explicit BlockingPrescaler(fhicl::ParameterSet const &);

  virtual bool filter(Event &) override;

private:
  size_t count_;
  size_t const m_;      // accept m in n (sequentially).
  size_t const n_;
  size_t const offset_; // First accepted event is 1 + offset.

};  // BlockingPrescaler

// ======================================================================

art::BlockingPrescaler::
BlockingPrescaler(fhicl::ParameterSet const & ps)
  :
  EDFilter(),
  count_(0),
  m_(ps.get<size_t>("blockSize", 1)),
  n_(ps.get<size_t>("stepSize")),
  offset_(ps.get<size_t>("offset"))
{
}

bool
art::BlockingPrescaler::
filter(Event &)
{
  bool result { (count_ >= offset_) && ((count_ - offset_) % n_) < m_ };
  ++count_;
  return result;
}
DEFINE_ART_MODULE(art::BlockingPrescaler)
