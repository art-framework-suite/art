// -*- C++ -*-
//
// Package:     Framework
// Class  :     DependentEventSetupRecordProvider
//
// Implementation:
//     <Notes on implementation>
//
// Author:      Chris Jones
// Created:     Sun May  1 17:15:52 EDT 2005
//
//

// system include files

// user include files
#include "art/Framework/Core/DependentEventSetupRecordProvider.h"
#include "art/Framework/Core/DependentRecordIntervalFinder.h"
#include "art/Utilities/Algorithms.h"

#include "boost/bind.hpp"

//
// constants, enums and typedefs
//
namespace edm {
   namespace eventsetup {
//
// static data member definitions
//

//
// constructors and destructor
//
//DependentEventSetupRecordProvider::DependentEventSetupRecordProvider()
//{
//}

// DependentEventSetupRecordProvider::DependentEventSetupRecordProvider(const DependentEventSetupRecordProvider& rhs)
// {
//    // do actual copying here;
// }

//DependentEventSetupRecordProvider::~DependentEventSetupRecordProvider()
//{
//}

//
// assignment operators
//
// const DependentEventSetupRecordProvider& DependentEventSetupRecordProvider::operator=(const DependentEventSetupRecordProvider& rhs)
// {
//   //An exception safe implementation is
//   DependentEventSetupRecordProvider temp(rhs);
//   swap(rhs);
//
//   return *this;
// }

//
// member functions
//
void
DependentEventSetupRecordProvider::setDependentProviders(const std::vector< boost::shared_ptr<EventSetupRecordProvider> >& iProviders)
{
   boost::shared_ptr< DependentRecordIntervalFinder > newFinder(
                                       new DependentRecordIntervalFinder(key()));

   boost::shared_ptr<EventSetupRecordIntervalFinder> old = swapFinder(newFinder);
   for_all(iProviders, boost::bind(std::mem_fun(&DependentRecordIntervalFinder::addProviderWeAreDependentOn), &(*newFinder), _1));
  //if a finder was already set, add it as a depedency.  This is done to ensure that the IOVs properly change even if the
  // old finder does not update each time a dependent record does change
  if(old.get() != 0) {
    newFinder->setAlternateFinder(old);
  }
}

//
// const member functions
//

//
// static member functions
//
   }
}
