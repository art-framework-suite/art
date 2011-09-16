#ifndef test_Persistency_Provenance_RootClassMapping_t_h
#define test_Persistency_Provenance_RootClassMapping_t_h

#include <iostream>

#include "TBuffer.h"
#include "TClass.h"
#include "TClassRef.h"
#include "TClassStreamer.h"

template <typename A, typename B>
class TestProdStreamer;

template <typename A, typename B>
class TestProd {
public:
  TestProd()
    :
    data() {
    static TClassRef cl(TClass::GetClass(typeid(TestProd<A, B>)));
    if (cl->GetStreamer() == 0) {
      cl->AdoptStreamer(new TestProdStreamer<A, B>);
    }
  }
  std::vector<size_t> data;
};

template <typename A, typename B>
class TestProdStreamer : public TClassStreamer {
public:
  void operator()(TBuffer & R_b, void * objp) {
    static TClassRef cl(TClass::GetClass(typeid(TestProd<A, B>)));
    TestProd<A, B> *obj = reinterpret_cast<TestProd<A, B>*>(objp);
    if (R_b.IsReading()) {
      std::cerr
        << "Attempting to read stored object as a "
        << cl->GetName()
        << ".\n";
      cl->ReadBuffer(R_b, obj);
    }
    else {
      std::cerr
        << "Attempting to write a "
        << cl->GetName()
        << ".\n";
      cl->WriteBuffer(R_b, obj);
    }
  }
};

#endif /* test_Persistency_Provenance_RootClassMapping_t_h */

// Local Variables:
// mode: c++
// End:
