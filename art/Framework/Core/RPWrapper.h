#ifndef art_Framework_Core_RPWrapper_h
#define art_Framework_Core_RPWrapper_h

#include "art/Framework/Principal/RPWrapperBase.h"

#include <memory>

namespace art {
  template <typename RP>
  class RPWrapper;
}

template <typename RP>
class art::RPWrapper : public art::RPWrapperBase {
public:
  using RPType = RP;

  RPWrapper(RPParams const & p, fhicl::ParameterSet const & ps);

private:
  RP & rp_() override;
  RP const & rp_() const override;

  RP rpPlugin_;
};

template <typename RP>
art::RPWrapper<RP>::
RPWrapper(RPParams const & p, fhicl::ParameterSet const & ps)
:
  RPWrapperBase(p),
  rpPlugin_(ps)
{
}

template <typename RP>
inline
auto
art::RPWrapper<RP>::
rp_() ->
RPType &
{
  return rpPlugin_;
}

template <typename RP>
inline
auto
art::RPWrapper<RP>::
rp_() const ->
RPType const &
{
  return rpPlugin_;
}

#endif /* art_Framework_Core_RPWrapper_h */

// Local Variables:
// mode: c++
// End:
