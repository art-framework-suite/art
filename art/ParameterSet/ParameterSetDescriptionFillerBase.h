#ifndef FWCore_ParameterSet_ParameterSetDescriptionFillerBase_h
#define FWCore_ParameterSet_ParameterSetDescriptionFillerBase_h

//
// Package:     ParameterSet
// Class  :     ParameterSetDescriptionFillerBase
//
/**\class ParameterSetDescriptionFillerBase ParameterSetDescriptionFillerBase.h FWCore/ParameterSet/interface/ParameterSetDescriptionFillerBase.h

 Description: Base class for a component which can fill a ParameterSetDescription object

 Usage:
    This base class provides an abstract interface for filling a ParameterSetDescription object.  This allows one to used by the
ParameterSetDescriptionFillerPluginFactory to load a component of any type (e.g. fw Source, fw EDProducer or even a tracking plugin)
and query the component for its allowed ParameterSetDescription.

*/


#include "art/ParameterSet/ParameterSetfwd.h"

#include <string>


namespace edm {

  class ParameterSetDescriptionFillerBase
  {
  public:
    ParameterSetDescriptionFillerBase() {}
    virtual ~ParameterSetDescriptionFillerBase();

    // ---------- const member functions ---------------------
    virtual void fill(ParameterSetDescription& iDesc, std::string const& moduleLabel) const = 0;

  private:
    // no copying
    ParameterSetDescriptionFillerBase(const ParameterSetDescriptionFillerBase&);
    ParameterSetDescriptionFillerBase& operator=(const ParameterSetDescriptionFillerBase&);

  };

}  // namespace edm

#endif  // FWCore_ParameterSet_ParameterSetDescriptionFillerBase_h
