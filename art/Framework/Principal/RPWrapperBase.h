#ifndef art_Framework_Principal_RPWrapperBase_h
#define art_Framework_Principal_RPWrapperBase_h

#include "art/Framework/Principal/RPParams.h"
#include "art/Persistency/Provenance/ModuleDescription.h"

namespace art {
  class RPWrapperBase;

  class ResultsProducer; // Forward declaration.
}

class art::RPWrapperBase {
public:
  RPWrapperBase(RPParams const & p, ModuleDescription const & md);
  virtual ~RPWrapperBase() = default;

  ResultsProducer & rp();
  ResultsProducer const & rp() const;

private:
  virtual ResultsProducer & rp_() = 0;
  virtual ResultsProducer const & rp_() const = 0;

  RPParams p_;
  ModuleDescription md_;
};

inline
art::RPWrapperBase::
RPWrapperBase(RPParams const & p, ModuleDescription const & md)
:
  p_(p),
  md_(md)
{
}

inline
art::ResultsProducer &
art::RPWrapperBase::
rp()
{
  return rp_();
}

inline
art::ResultsProducer const &
art::RPWrapperBase::
rp() const
{
  return rp_();
}

#endif /* art_Framework_Principal_RPWrapperBase_h */

// Local Variables:
// mode: c++
// End:
