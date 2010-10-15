// -*- C++ -*-
//
// Package:     Framework
// Class  :     IOVSyncValue
//
// Implementation:
//     <Notes on implementation>
//
// Original Author:  Chris Jones
//         Created:  Wed Aug  3 18:35:35 EDT 2005
//
//

// system include files

// user include files
#include "art/Framework/Core/IOVSyncValue.h"


//
// constants, enums and typedefs
//
namespace art {

//
// static data member definitions
//


//
// constructors and destructor
//
IOVSyncValue::IOVSyncValue(): eventID_(), subRunID_(0), time_(),
haveID_(true), haveTime_(true)
{
}

IOVSyncValue::IOVSyncValue(const EventID& iID, SubRunNumber_t iSubRun) : eventID_(iID), subRunID_(iSubRun), time_(),
haveID_(true), haveTime_(false)
{
}

IOVSyncValue::IOVSyncValue(const Timestamp& iTime) : eventID_(), subRunID_(0),time_(iTime),
haveID_(false), haveTime_(true)
{
}

IOVSyncValue::IOVSyncValue(const EventID& iID, SubRunNumber_t iSubRun, const Timestamp& iTime) :
eventID_(iID), subRunID_(iSubRun), time_(iTime),
haveID_(true), haveTime_(true)
{
}

// IOVSyncValue::IOVSyncValue(const IOVSyncValue& rhs)
// {
//    // do actual copying here;
// }

//IOVSyncValue::~IOVSyncValue()
//{
//}

//
// assignment operators
//
// const IOVSyncValue& IOVSyncValue::operator=(const IOVSyncValue& rhs)
// {
//   //An exception safe implementation is
//   IOVSyncValue temp(rhs);
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
const IOVSyncValue&
IOVSyncValue::invalidIOVSyncValue() {
   static IOVSyncValue s_invalid;
   return s_invalid;
}
const IOVSyncValue&
IOVSyncValue::endOfTime() {
   static IOVSyncValue s_endOfTime(EventID(0xFFFFFFFFUL, EventID::maxEventNumber()),
                                   SubRunID::maxSubRunNumber(),
                                   Timestamp::endOfTime());
   return s_endOfTime;
}
const IOVSyncValue&
IOVSyncValue::beginOfTime() {
   static IOVSyncValue s_beginOfTime(EventID(1,0), 0, Timestamp::beginOfTime());
   return s_beginOfTime;
}
}
