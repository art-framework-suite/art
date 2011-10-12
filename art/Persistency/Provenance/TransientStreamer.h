#ifndef art_Persistency_Provenance_TransientStreamer_h
#define art_Persistency_Provenance_TransientStreamer_h

#include "TClassRef.h"
#include "TClassStreamer.h"
#include "art/Utilities/TypeID.h"
#include <string>

class TBuffer;

namespace art {

  template <typename> class TransientStreamer;

  void setProvenanceTransientStreamers();

  namespace detail {
    template <typename> void SetTransientStreamer();
    template <typename T> void SetTransientStreamer(T const &);
  }
}

template <typename T>
class art::TransientStreamer : public TClassStreamer {
public:
  typedef T element_type;
  TransientStreamer();
  void operator()(TBuffer & R_b, void * objp);
private:
  std::string className_;
  TClassRef cl_;
};

template <typename T>
art::TransientStreamer<T>::TransientStreamer() :
  className_(TypeID(typeid(T)).className()),
  cl_(className_.c_str())
{}

template <typename T>
void
art::TransientStreamer<T>::operator()(TBuffer & R_b, void * objp)
{
  if (R_b.IsReading()) {
    cl_->ReadBuffer(R_b, objp);
    // Fill with default constructed object;
    T * obj = static_cast<T *>(objp);
    *obj = T();
  }
  else {
    cl_->WriteBuffer(R_b, objp);
  }
}

template <typename T>
void
art::detail::SetTransientStreamer()
{
  TClass * cl = TClass::GetClass(typeid(T));
  if (cl->GetStreamer() == 0) {
    cl->AdoptStreamer(new TransientStreamer<T>());
  }
}

template <typename T>
void
art::detail::SetTransientStreamer(T const &)
{
  TClass * cl = TClass::GetClass(typeid(T));
  if (cl->GetStreamer() == 0) {
    cl->AdoptStreamer(new TransientStreamer<T>());
  }
}


#endif /* art_Persistency_Provenance_TransientStreamer_h */

// Local Variables:
// mode: c++
// End:
