#ifndef art_Persistency_Common_Assns_h
#define art_Persistency_Common_Assns_h

#include "art/Persistency/Common/Ptr.h"
#include "art/Utilities/TypeID.h"

#include "TBuffer.h"
#include "TClass.h"
#include "TClassRef.h"

#include <vector>

namespace art {
  // General template.
  template <typename L, typename R, typename D = void>
  class Assns;

  // Specialization.
  template <typename L, typename R>
  class Assns<L, R, void>;
}

////////////////////////////////////////////////////////////////////////
// Implementation of the specialization.
template <typename L, typename R>
class art::Assns<L, R, void> {
public:
  typedef L left_t;
  typedef R right_t;

private:
  typedef std::vector<std::pair<Ptr<left_t>, Ptr<right_t> > > ptrs_t;
  typedef std::vector<std::pair<RefCore, size_t> > ptr_data_t;

public:
  // Temporary accessors for testing persistency.
  typename ptrs_t::value_type const &operator[](typename ptrs_t::size_type index) const;
  typename ptrs_t::size_type size() const;

  void addSingle(Ptr<left_t> const &left,
                 Ptr<right_t> const &right);
  void Streamer(TBuffer &R_b);
  static short Class_Version() { return 10; }

private:
  static bool left_first();

  void fill_transients();
  void fill_from_transients();


  mutable ptrs_t ptrs_; //! transient
  mutable ptr_data_t ptr_data_1_;
  mutable ptr_data_t ptr_data_2_;
};

////////////////////////////////////////////////////////////////////////
// Yes, the general implementation inherits from the specific. Head
// exploded yet?
template <typename L, typename R, typename D>
class art::Assns : private art::Assns<L, R> {
private:
  typedef Assns<L, R> base;
public:
  typedef typename base::left_t left_t;
  typedef typename base::right_t right_t;
  typedef D data_t;

  using base::size;
  using base::operator[];

  // Temporary accessor for testing persistency.
  data_t const &data(typename std::vector<data_t>::size_type index) const;

  void addSingle(Ptr<left_t> const &left,
                 Ptr<right_t> const &right,
                 data_t const &data);
  static short Class_Version() { return 10; }

private:
  std::vector<data_t> data_;
};

////////////////////////////////////////////////////////////////////////

template <typename L, typename R>
inline
typename art::Assns<L, R, void>::ptrs_t::value_type const &
art::Assns<L, R, void>::operator[](typename ptrs_t::size_type index) const {
  return ptrs_[index];
}

template <typename L, typename R>
inline
typename art::Assns<L, R, void>::ptrs_t::size_type
art::Assns<L, R, void>::size() const {
  return ptrs_.size();
}

template <typename L, typename R>
inline
void
art::Assns<L, R, void>::addSingle(Ptr<left_t> const &left,
                                  Ptr<right_t> const &right) {
  ptrs_.push_back(std::make_pair(left, right));
}

template <typename L, typename R>
inline
void
art::Assns<L, R, void>::Streamer(TBuffer &R_b) {
  static TClassRef cl(TClass::GetClass(typeid(Assns<L, R, void>)));
  if (R_b.IsReading()) {
    cl->ReadBuffer(R_b, this);
    fill_transients();
  } else {
    fill_from_transients();
    cl->WriteBuffer(R_b, this);
  }
}

template <typename L, typename R>
inline
bool
art::Assns<L, R, void>::left_first() {
  static bool lf_s = (art::TypeID(typeid(left_t)).friendlyClassName() <
                      art::TypeID(typeid(right_t)).friendlyClassName());
  return lf_s;
}

template <typename L, typename R>
void
art::Assns<L, R, void>::fill_transients() {
  // Precondition: ptrs_ is empty.
  // Precondition: ptr_data_1_.size() = ptr_data_2_.size();
  ptrs_.reserve(ptr_data_1_.size());
  ptr_data_t &l_ref = left_first()?ptr_data_1_:ptr_data_2_;
  ptr_data_t &r_ref = left_first()?ptr_data_2_:ptr_data_1_;
  for (typename ptr_data_t::const_iterator
         l = l_ref.begin(),
         e = l_ref.end(),
         r = r_ref.begin();
       l != e;
       ++l, ++r) {
    ptrs_.push_back(std::make_pair(Ptr<left_t>(l->first.id(),
                                               l->second,
                                               l->first.productGetter()),
                                   Ptr<right_t>(r->first.id(),
                                                r->second,
                                                r->first.productGetter())));
  }
  // Empty persistent representation.
  ptr_data_t tmp1, tmp2;
  l_ref.swap(tmp1);
  r_ref.swap(tmp2);
}

template <typename L, typename R>
void
art::Assns<L, R, void>::fill_from_transients() {
  // Precondition: ptr_data_1_ is empty;
  // Precondition: ptr_data_2_ is empty;
  ptr_data_t &l_ref = left_first()?ptr_data_1_:ptr_data_2_;
  ptr_data_t &r_ref = left_first()?ptr_data_2_:ptr_data_1_;
  l_ref.reserve(ptrs_.size());
  r_ref.reserve(ptrs_.size());
  for (typename ptrs_t::const_iterator
         i = ptrs_.begin(),
         e = ptrs_.end();
       i != e;
       ++i) {
    l_ref.push_back(std::make_pair(i->first.refCore(),
                                   i->first.key()));
    r_ref.push_back(std::make_pair(i->second.refCore(),
                                   i->second.key()));
  }
}

template <typename L, typename R, typename D>
inline
typename art::Assns<L, R, D>::data_t const &
art::Assns<L, R, D>::data(typename std::vector<data_t>::size_type index) const {
  return data_[index];
}

template <typename L, typename R, typename D>
inline
void
art::Assns<L, R, D>::addSingle(Ptr<left_t> const &left,
                               Ptr<right_t> const &right,
                               data_t const &data) {
  base::addSingle(left, right);
  data_.push_back(data);
}

#endif /* art_Persistency_Common_Assns_h */

// Local Variables:
// mode: c++
// End:
