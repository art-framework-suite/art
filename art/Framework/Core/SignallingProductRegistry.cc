// -*- C++ -*-
//
// Package:     Framework
// Class  :     SignallingProductRegistry
//
// Implementation:
//     <Notes on implementation>
//
// Original Author:  Chris Jones
//         Created:  Fri Sep 23 16:52:50 CEST 2005
//
//

// system include files

// user include files
#include "art/Framework/Core/SignallingProductRegistry.h"
#include "art/Utilities/Exception.h"

using namespace art;
//
// member functions
//
namespace {
   struct StackGuard {
      StackGuard(const std::string& iTypeName, std::map<std::string,unsigned int>& ioStack, bool iFromListener):
      numType_(++ioStack[iTypeName]), itr_(ioStack.find(iTypeName)),fromListener_(iFromListener)
      { if(iFromListener) {++(itr_->second);} }

      ~StackGuard() {
         --(itr_->second);
         if(fromListener_){
            --(itr_->second);
         }
      }

      unsigned int numType_;
      std::map<std::string,unsigned int>::iterator itr_;
      bool fromListener_;
   };
}

void SignallingProductRegistry::addCalled(BranchDescription const& iProd, bool iFromListener)
{
   StackGuard guard(iProd.className(), typeAddedStack_, iFromListener);
   if(guard.numType_ > 2) {
      throw artZ::Exception("CircularReference")
      <<"Attempted to register the production of "<<iProd.className()
      <<" from module "
      <<iProd.moduleLabel()
      <<" with product instance \""
      <<iProd.productInstanceName()
      <<"\"\n"
      <<"However, this was in reaction to a registration of a production for the same type \n"
      <<"from another module who was also listening to product registrations.\n"
      <<"This can lead to circular Event::get* calls.\n"
      <<"Please reconfigure job so it does not contain both of the modules.";
   }
   productAddedSignal_(iProd);
}
