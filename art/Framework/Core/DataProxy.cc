// -*- C++ -*-
//
// Package:     Framework
// Class  :     DataProxy
//
// Implementation:
//     <Notes on implementation>
//
// Author:      Chris Jones
// Created:     Thu Mar 31 12:49:19 EST 2005
//
//

// system include files

// user include files
#include "art/Framework/Core/DataProxy.h"
#include "art/Framework/Core/ComponentDescription.h"


//
// constants, enums and typedefs
//
namespace edm {
   namespace eventsetup {
//
// static data member definitions
//
static
const ComponentDescription*
dummyDescription()
{
   static ComponentDescription s_desc;
   return &s_desc;
}
//
// constructors and destructor
//
DataProxy::DataProxy() :
   cacheIsValid_(false),
   description_(dummyDescription())
{
}

// DataProxy::DataProxy(const DataProxy& rhs)
// {
//    // do actual copying here;
// }

DataProxy::~DataProxy()
{
}

//
// assignment operators
//
// const DataProxy& DataProxy::operator=(const DataProxy& rhs)
// {
//   //An exception safe implementation is
//   DataProxy temp(rhs);
//   swap(rhs);
//
//   return *this;
// }

//
// member functions
//

//
// const member functions
//

//
// static member functions
//
   }
}
