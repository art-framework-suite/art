#ifndef art_Framework_Core_RPWorkerT_h
#define art_Framework_Core_RPWorkerT_h
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/RPWorker.h"

#include <memory>

namespace art {

  template <typename RP>
  class RPWorkerT : public RPWorker {

  public:
    using RPType = RP;

  public:
    RPWorkerT(RPParams const& p, fhicl::ParameterSet const& ps);

  private:
    RP& rp_() override;

    RP const& rp_() const override;

  private:
    RP rpPlugin_;
  };

  template <typename RP>
  RPWorkerT<RP>::RPWorkerT(RPParams const& p, fhicl::ParameterSet const& ps)
    : RPWorker(p), rpPlugin_(ps)
  {}

  template <typename RP>
  inline auto
  RPWorkerT<RP>::rp_() -> RPType&
  {
    return rpPlugin_;
  }

  template <typename RP>
  inline auto
  RPWorkerT<RP>::rp_() const -> RPType const&
  {
    return rpPlugin_;
  }

} // namespace art

#endif /* art_Framework_Core_RPWorkerT_h */

// Local Variables:
// mode: c++
// End:
