//
// Package:     ServiceRegistry
// Class  :     ServiceToken
//

#include "art/Framework/Services/Registry/ServiceToken.h"
#include "art/Framework/Services/Registry/ServicesManager.h"

//
// member functions
//
void
art::ServiceToken::connectTo(art::ActivityRegistry& iConnectTo)
{
   if(0!=manager_.get()){
      manager_->connectTo(iConnectTo);
   }
}
void
art::ServiceToken::connect(art::ActivityRegistry& iConnectTo)
{
   if(0!=manager_.get()){
      manager_->connect(iConnectTo);
   }
}

void
art::ServiceToken::copySlotsTo(art::ActivityRegistry& iConnectTo)
{
  if(0!=manager_.get()){
    manager_->copySlotsTo(iConnectTo);
  }
}
void
art::ServiceToken::copySlotsFrom(art::ActivityRegistry& iConnectTo)
{
  if(0!=manager_.get()){
    manager_->copySlotsFrom(iConnectTo);
  }
}
