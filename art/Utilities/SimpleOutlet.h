#ifndef art_Utilities_SimpleOutlet_h
#define art_Utilities_SimpleOutlet_h
// -*- C++ -*-
//
// Package:     Utilities
// Class  :     SimpleOutlet
//
/**\class SimpleOutlet SimpleOutlet.h FWCore/Utilities/interface/SimpleOutlet.h

 Description: A simple outlet that works with the art::ExtensionCord

 Usage:
    <usage>

*/
//
// Original Author:  Chris Jones
//         Created:  Fri Sep 22 13:10:36 EDT 2006
//
//

// system include files

// user include files
#include "art/Utilities/OutletBase.h"
#include "art/Utilities/ECGetterBase.h"

// forward declarations
namespace art {
  template<class T>
  class SimpleOutlet : private OutletBase<T>
  {

    public:
      SimpleOutlet( extensioncord::ECGetterBase<T>& iGetter,
                    ExtensionCord<T>& iCord) :
        OutletBase<T>(iCord) {
          this->setGetter(&iGetter);
      }

   private:
      SimpleOutlet(const SimpleOutlet&); // stop default

      const SimpleOutlet& operator=(const SimpleOutlet&); // stop default

};

}
#endif /* art_Utilities_SimpleOutlet_h */

// Local Variables:
// mode: c++
// End:
