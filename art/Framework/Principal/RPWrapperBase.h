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
  RPWrapperBase(RPParams const & p);
  virtual ~RPWrapperBase() = default;

  ResultsProducer & rp();
  ResultsProducer const & rp() const;

  RPParams const & params() const;
  ModuleDescription const & moduleDescription() const;

  void setModuleDescription(ModuleDescription const &);
  void setModuleDescription(ModuleDescription &&);

private:
  virtual ResultsProducer & rp_() = 0;
  virtual ResultsProducer const & rp_() const = 0;

  RPParams p_;
  ModuleDescription md_;
};

inline
art::RPWrapperBase::
RPWrapperBase(RPParams const & p)
:
  p_(p),
  md_()
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

inline
art::RPParams const &
art::RPWrapperBase::
params() const
{
  return p_;
}

inline
art::ModuleDescription const &
art::RPWrapperBase::
moduleDescription() const
{
  return md_;
}

inline
void
art::RPWrapperBase::
setModuleDescription(art::ModuleDescription const & md)
{
  md_ = md;
}

inline
void
art::RPWrapperBase::
setModuleDescription(art::ModuleDescription && md)
{
  md_ = std::move(md);
}

#endif /* art_Framework_Principal_RPWrapperBase_h */

// Local Variables:
// mode: c++
// End:
