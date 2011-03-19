#ifndef art_Framework_IO_Sources_RawInputSource_h
#define art_Framework_IO_Sources_RawInputSource_h

// ======================================================================
//
// RawInputSource is an abstract base class, intended to make it
// easier to write InputSources that read non-art-format data files.
//
// ======================================================================

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/DecrepitRelicInputSourceImplementation.h"
#include "art/Persistency/Provenance/EventID.h"
#include "boost/shared_ptr.hpp"
#include "fhiclcpp/ParameterSet.h"
#include <memory>

// ----------------------------------------------------------------------

namespace art
{
  class Timestamp;

  class RawInputSource : public DecrepitRelicInputSourceImplementation
  {
  public:
    explicit RawInputSource(fhicl::ParameterSet const& pset,
                            InputSourceDescription const& desc);
    virtual ~RawInputSource();

  protected:
    virtual input::ItemType getNextItemType() = 0;

  private:
    boost::shared_ptr<RunPrincipal> readRun_();
    boost::shared_ptr<SubRunPrincipal> readSubRun_();
    std::auto_ptr<EventPrincipal> readEvent_();
    std::auto_ptr<EventPrincipal> readIt(EventID const&);
    void skip(int);

    RunNumber_t                   runNumber_;
    SubRunNumber_t                subRunNumber_;
    bool                          newRun_;
    bool                          newSubRun_;
    std::auto_ptr<EventPrincipal> ep_;
  };

}  // art

// ======================================================================

#endif /* art_Framework_IO_Sources_RawInputSource_h */

// Local Variables:
// mode: c++
// End:
