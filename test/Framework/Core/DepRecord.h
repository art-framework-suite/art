#ifndef Framework_DepRecord_h
#define Framework_DepRecord_h
// -*- C++ -*-
//
// Package:     Framework
// Class  :     DepRecord
//
/**\class DepRecord DepRecord.h FWCore/Framework/test/DepRecord.h

 Description: A test Record that is dependent on DummyRecord

 Usage:
    <usage>

*/
//
// Original Author:  Chris Jones
//         Created:  Wed Jun 22 19:46:32 EDT 2005
//
//

// system include files
#include "boost/mpl/vector.hpp"

// user include files

// forward declarations
#include "art/Framework/Core/DependentRecordImplementation.h"
#include "test/Framework/Core/DummyRecord.h"

class DepRecord
: public edm::eventsetup::DependentRecordImplementation<DepRecord, boost::mpl::vector<DummyRecord> >
{
};

#endif
