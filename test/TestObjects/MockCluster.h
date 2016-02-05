#ifndef test_TestObjects_MockCluster_h
#define test_TestObjects_MockCluster_h

#include "test/TestObjects/ToyProducts.h"
#include "canvas/Persistency/Common/PtrVector.h"
#include "canvas/Persistency/Provenance/EventID.h"

#include <vector>

namespace arttest {
   struct MockCluster;
}

struct arttest::MockCluster {

   typedef art::PtrVector<arttest::SimpleDerived> CellList;

   MockCluster()
      :
      skew(0),
      cells(),
      eNum(0)
   {}

   short int skew;
   CellList cells;
   art::EventNumber_t eNum;

};

namespace arttest {
   typedef std::vector<arttest::MockCluster> MockClusterList;
}

#endif /* test_TestObjects_MockCluster_h */

// Local Variables:
// mode: c++
// End:

