/*
 *  eventsetup_get_check_compile_time_error_t.cc
 *  EDMProto
 *
 *  Created by Chris Jones on 3/25/05.
 *
 */


#include "art/Framework/Core/EventSetup.h"
#include "art/Framework/Core/EventSetupProvider.h"
#include "art/Framework/Core/IOVSyncValue.h"

using namespace edm;
class NotAGoodRecord {};

int main() {
   eventsetup::EventSetupProvider provider;
   EventSetup const& eventSetup = provider.eventSetupForInstance(IOVSyncValue(0));
   //This should cause a compile time failure since NotAGoodRecord
   /// does not inherit from edm::eventsetup::EventSetupRecord
   eventSetup.get<NotAGoodRecord>();

   return 0;
}
