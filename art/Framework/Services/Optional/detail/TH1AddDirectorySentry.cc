// -*- C++ -*-
//
// Package:     UtilAlgos
// Class  :     TH1AddDirectorySentry
//
// Implementation:
//     <Notes on implementation>
//
// Original Author:  Chris Jones
//         Created:  Thu Nov  8 12:16:02 EST 2007
//
//


#include "TH1.h"

#include "art/Framework/Services/Optional/detail/TH1AddDirectorySentry.h"


//
// constructors and destructor
//
TH1AddDirectorySentry::TH1AddDirectorySentry():
  status_(TH1::AddDirectoryStatus())
{
  TH1::AddDirectory(true);
}

TH1AddDirectorySentry::~TH1AddDirectorySentry()
{
  TH1::AddDirectory(status_);
}

