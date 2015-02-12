#ifndef art_Framework_Services_Optional_SimpleMemoryCheck_h
#define art_Framework_Services_Optional_SimpleMemoryCheck_h

#ifdef __linux__
#  include "art/Framework/Services/Optional/SimpleMemoryCheckLinux.h"
#elif  __APPLE__
#  include "art/Framework/Services/Optional/SimpleMemoryCheckDarwin.h"
#endif

#endif // art_Framework_Services_Optional_SimpleMemoryCheck_h
