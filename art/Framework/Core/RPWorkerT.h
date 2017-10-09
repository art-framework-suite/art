#ifndef art_Framework_Core_RPWorkerT_h
#define art_Framework_Core_RPWorkerT_h

#include "art/Framework/Principal/RPWorker.h"

#include <memory>

namespace art {
  template <typename RP>
  class RPWorkerT;
}

template <typename RP>
class art::RPWorkerT : public art::RPWorker {
public:
  using RPType = RP;

  RPWorkerT(RPParams const& p, fhicl::ParameterSet const& ps);

private:
  RP& rp_() override;
  RP const& rp_() const override;

  RP rpPlugin_;
};

template <typename RP>
art::RPWorkerT<RP>::RPWorkerT(RPParams const& p, fhicl::ParameterSet const& ps)
  : RPWorker(p), rpPlugin_(ps)
{}

template <typename RP>
inline auto
art::RPWorkerT<RP>::rp_() -> RPType&
{
  return rpPlugin_;
}

template <typename RP>
inline auto
art::RPWorkerT<RP>::rp_() const -> RPType const&
{
  return rpPlugin_;
}

#endif /* art_Framework_Core_RPWorkerT_h */

// Local Variables:
// mode: c++
// End:
