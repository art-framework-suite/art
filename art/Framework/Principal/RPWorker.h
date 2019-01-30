#ifndef art_Framework_Principal_RPWorker_h
#define art_Framework_Principal_RPWorker_h
// vim: set sw=2 expandtab :
////////////////////////////////////////////////////////////////////////
// RPWorker
//
// "Worker"-style wrapper base class for ResultsProducers.
//
// Allows for access to particular parameters and module descriptions
// without their having to be passed via the ResultsProducer
// constructor.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Principal/RPParams.h"
#include "art/Persistency/Provenance/ModuleDescription.h"

namespace art {

  class ResultsProducer;

  class RPWorker {

  public:
    virtual ~RPWorker() = default;

    RPWorker(RPParams const& p);

  public:
    ResultsProducer& rp();

    ResultsProducer const& rp() const;

    RPParams const& params() const;

    ModuleDescription const& moduleDescription() const;

    void setModuleDescription(ModuleDescription const&);

    void setModuleDescription(ModuleDescription&&);

  private:
    virtual ResultsProducer& rp_() = 0;

    virtual ResultsProducer const& rp_() const = 0;

  private:
    RPParams p_;

    ModuleDescription md_;
  };

  inline RPWorker::RPWorker(RPParams const& p) : p_(p), md_() {}

  inline ResultsProducer&
  RPWorker::rp()
  {
    return rp_();
  }

  inline ResultsProducer const&
  RPWorker::rp() const
  {
    return rp_();
  }

  inline RPParams const&
  RPWorker::params() const
  {
    return p_;
  }

  inline ModuleDescription const&
  RPWorker::moduleDescription() const
  {
    return md_;
  }

  inline void
  RPWorker::setModuleDescription(ModuleDescription const& md)
  {
    md_ = md;
  }

  inline void
  RPWorker::setModuleDescription(ModuleDescription&& md)
  {
    md_ = std::move(md);
  }

} // namespace art

#endif /* art_Framework_Principal_RPWorker_h */

// Local Variables:
// mode: c++
// End:
