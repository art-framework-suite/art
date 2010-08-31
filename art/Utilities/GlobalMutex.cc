#include "art/Utilities/GlobalMutex.h"

boost::mutex* edm::rootfix::getGlobalMutex() {
    static boost::mutex m_;
    return &m_;
}
