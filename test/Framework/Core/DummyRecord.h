#ifndef Framework_DummyRecord_h
#define Framework_DummyRecord_h
/*
 *  DummyRecord.h
 *  EDMProto
 *
 *  Created by Chris Jones on 4/4/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#include "art/Framework/Core/EventSetupRecordImplementation.h"

class DummyRecord : public art::eventsetup::EventSetupRecordImplementation<DummyRecord> {};

#endif
