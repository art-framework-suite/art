// -*- C++ -*-
//
// Package:     ServiceRegistry
// Class  :     ServiceToken
//
// Implementation:
//     <Notes on implementation>
//
// Original Author:  Chris Jones
//         Created:  Thu Sep  8 04:26:12 EDT 2005
//
//

// system include files

// user include files
#include "art/Framework/Services/Registry/ServiceToken.h"
#include "art/Framework/Services/Registry/ServicesManager.h"


//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
//ServiceToken::ServiceToken()
//{
//}

// ServiceToken::ServiceToken(const ServiceToken& rhs)
// {
//    // do actual copying here;
// }

//ServiceToken::~ServiceToken()
//{
//}

//
// assignment operators
//
// const ServiceToken& ServiceToken::operator=(const ServiceToken& rhs)
// {
//   //An exception safe implementation is
//   ServiceToken temp(rhs);
//   swap(rhs);
//
//   return *this;
// }

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

//
// const member functions
//

//
// static member functions
//
