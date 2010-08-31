#ifndef Framework_DummyEventSetupData_h
#define Framework_DummyEventSetupData_h
/*
 *  DummyEventSetupData.h
 *  EDMProto
 *
 *  Created by Chris Jones on 4/4/05.
 *
 */

//used to set the default record
#include "test/Framework/Core/DummyEventSetupRecord.h"

namespace edm {
   struct DummyEventSetupData {
      DummyEventSetupData(int iValue) : value_(iValue) {}
      int value_;
   };
}

#include "art/Framework/Core/data_default_record_trait.h"
EVENTSETUP_DATA_DEFAULT_RECORD(edm::DummyEventSetupData, edm::DummyEventSetupRecord)

#if !defined(TEST_EXCLUDE_DEF)
//NOTE: This should really be put into a .cc file
#include "art/Framework/Core/eventsetupdata_registration_macro.h"
EVENTSETUP_DATA_REG(edm::DummyEventSetupData);
#endif
#endif

