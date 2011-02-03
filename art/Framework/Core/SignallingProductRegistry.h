#ifndef art_Framework_Core_SignallingProductRegistry_h
#define art_Framework_Core_SignallingProductRegistry_h
// -*- C++ -*-
//
// Package:     Framework
// Class  :     SignallingProductRegistry
//
/**\class SignallingProductRegistry SignallingProductRegistry.h FWCore/Framework/interface/SignallingProductRegistry.h

 Description: <one line class summary>

 Usage:
    <usage>

*/
//
// Original Author:  Chris Jones
//         Created:  Fri Sep 23 16:47:10 CEST 2005
//
//

// system include files
#include "sigc++/signal.h"
#include <map>
#include <string>

// user include files
#include "art/Persistency/Provenance/ProductRegistry.h"

// forward declarations
namespace art {
   class SignallingProductRegistry : public ProductRegistry
   {

   public:
      SignallingProductRegistry() : ProductRegistry(), productAddedSignal_(), typeAddedStack_() {}
      sigc::signal<void, BranchDescription const&> productAddedSignal_;

   private:
      SignallingProductRegistry(const SignallingProductRegistry&); // stop default

      const SignallingProductRegistry& operator=(const SignallingProductRegistry&); // stop default

      virtual void addCalled(BranchDescription const&, bool);
      // ---------- member data --------------------------------
      std::map<std::string, unsigned int> typeAddedStack_;
   };
}

#endif /* art_Framework_Core_SignallingProductRegistry_h */

// Local Variables:
// mode: c++
// End:
