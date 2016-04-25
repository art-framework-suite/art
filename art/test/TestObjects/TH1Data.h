#ifndef test_TestObjects_TH1Data_h
#define test_TestObjects_TH1Data_h

#include "TH1D.h"

namespace arttest {
  struct TH1Data;
}

struct arttest::TH1Data {
  TH1Data();
  TH1D data;
  void aggregate(TH1Data const& other)
  {
    data.Add(&other.data);
  }
};

#endif /* test_TestObjects_TH1Data_h */

// Local Variables:
// mode: c++
// End:
