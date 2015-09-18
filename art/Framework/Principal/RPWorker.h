#ifndef art_Framework_Principal_RPWorker_h
#define art_Framework_Principal_RPWorker_h

#include "art/Framework/Principal/RPParams.h"
#include "art/Persistency/Provenance/ModuleDescription.h"

namespace art {
  class RPWorker;

  class ResultsProducer; // Forward declaration.
}

class art::RPWorker {
public:
  RPWorker(RPParams const & p);
  virtual ~RPWorker() = default;

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
art::RPWorker::
RPWorker(RPParams const & p)
:
  p_(p),
  md_()
{
}

inline
art::ResultsProducer &
art::RPWorker::
rp()
{
  return rp_();
}

inline
art::ResultsProducer const &
art::RPWorker::
rp() const
{
  return rp_();
}

inline
art::RPParams const &
art::RPWorker::
params() const
{
  return p_;
}

inline
art::ModuleDescription const &
art::RPWorker::
moduleDescription() const
{
  return md_;
}

inline
void
art::RPWorker::
setModuleDescription(art::ModuleDescription const & md)
{
  md_ = md;
}

inline
void
art::RPWorker::
setModuleDescription(art::ModuleDescription && md)
{
  md_ = std::move(md);
}

#endif /* art_Framework_Principal_RPWorker_h */

// Local Variables:
// mode: c++
// End:
