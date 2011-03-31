#ifndef art_Persistency_Provenance_TransientStreamer_h
#define art_Persistency_Provenance_TransientStreamer_h

#include "TClassRef.h"
#include "TClassStreamer.h"
#include "TROOT.h"
#include "art/Utilities/TypeID.h"
#include <string>

class TBuffer;

namespace art {

  void setTransientStreamers();

  template <typename T>
  class TransientStreamer : public TClassStreamer {
  public:
    typedef T element_type;
    TransientStreamer();
    void operator() (TBuffer &R_b, void *objp);
  private:
    std::string className_;
    TClassRef cl_;
  };

  template <typename T>
  TransientStreamer<T>::TransientStreamer() :
    className_(TypeID(typeid(T)).className()),
    cl_(className_.c_str())
  {}

  template <typename T>
  void
  TransientStreamer<T>::operator()(TBuffer &R_b, void *objp) {
    if (R_b.IsReading()) {
      cl_->ReadBuffer(R_b, objp);
      // Fill with default constructed object;
      T* obj = static_cast<T *>(objp);
      *obj = T();
    } else {
      cl_->WriteBuffer(R_b, objp);
    }
  }
}

#endif /* art_Persistency_Provenance_TransientStreamer_h */

// Local Variables:
// mode: c++
// End:
