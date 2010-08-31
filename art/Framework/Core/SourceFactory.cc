// -*- C++ -*-
//
// Package:     Framework
// Class  :     SourceFactory
//
// Implementation:
//     <Notes on implementation>
//
// Author:      Chris Jones
// Created:     Wed May 25 19:27:37 EDT 2005
//
//

// system include files

// user include files
#include "art/Framework/Core/SourceFactory.h"

//
// static member functions
//
namespace edm {
   namespace eventsetup {
      std::string SourceMakerTraits::name() { return "CMS EDM Framework ESSource"; }

   }
}

COMPONENTFACTORY_GET(edm::eventsetup::SourceMakerTraits);
