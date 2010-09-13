#ifndef FWCore_ParameterSet_ParameterSetDescriptionFiller_h
#define FWCore_ParameterSet_ParameterSetDescriptionFiller_h

//
// Package:     ParameterSet
// Class  :     ParameterSetDescriptionFiller
//
/**\class ParameterSetDescriptionFiller ParameterSetDescriptionFiller.h FWCore/ParameterSet/interface/ParameterSetDescriptionFiller.h

 Description: A concrete ParameterSetDescription filler which calls a static function of the template argument

 Usage:
    This is an ParameterSetDescription filler adapter class which calls the

void fillDescription(edm::ParameterSetDescription&)

method of the templated argument.  This allows the ParameterSetDescriptionFillerPluginFactory to communicate with existing plugins.

*/


#include "art/ParameterSet/ParameterSetDescriptionFillerBase.h"

#include <string>


namespace edm {

  template< typename T>
    class ParameterSetDescriptionFiller
      : public ParameterSetDescriptionFillerBase
  {
  public:
    ParameterSetDescriptionFiller() {}

    // ---------- const member functions ---------------------
    virtual void fill( ParameterSetDescription& iDesc
                     , std::string const& moduleLabel ) const
    {
      T::fillDescription(iDesc, moduleLabel);
    }

  private:
    // no copying
    ParameterSetDescriptionFiller(const ParameterSetDescriptionFiller&);
    ParameterSetDescriptionFiller& operator=(const ParameterSetDescriptionFiller&);

  };

}  // namespace edm

#endif  // FWCore_ParameterSet_ParameterSetDescriptionFiller_h
