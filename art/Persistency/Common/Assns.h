#ifndef art_Persistency_Common_Assns_h
#define art_Persistency_Common_Assns_h

#include "art/Persistency/Common/Ptr.h"
#include "art/Persistency/Common/Wrapper.h"
#include "art/Utilities/Exception.h"
#include "art/Utilities/TypeID.h"

#include "TBuffer.h"
#include "TClassStreamer.h" // Temporary
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

  namespace detail {
    // Temporary streamer class until streamer method registration is
    // working again.
    template <typename L, typename R>
    class AssnsStreamer : public TClassStreamer {
    public:
      void operator()(TBuffer &R_b, void *objp) {
        static TClassRef cl(TClass::GetClass(typeid(Assns<L, R, void>)));
        Assns<L, R, void> *obj = reinterpret_cast<Assns<L, R, void> *>(objp);
        if (R_b.IsReading()) {
          cl->ReadBuffer(R_b, obj);
          obj->fill_transients();
        } else {
          obj->fill_from_transients();
          cl->WriteBuffer(R_b, obj);
        }
      }
    };
  }
}

////////////////////////////////////////////////////////////////////////
// Implementation of the specialization.
template <typename L, typename R>
class art::Assns<L, R, void> {
public:
  typedef L left_t;
  typedef R right_t;
  typedef art::Assns<right_t, left_t, void> partner_t;

private:
  typedef std::vector<std::pair<Ptr<left_t>, Ptr<right_t> > > ptrs_t;
  typedef std::vector<std::pair<RefCore, size_t> > ptr_data_t;

public:
  typedef typename ptrs_t::value_type assn_t;
  typedef typename ptrs_t::const_iterator assn_iterator;

  // Temporary constructor for the purposes of setting the streamer
  // class.
  Assns();
  Assns(partner_t const &other);
  virtual ~Assns();
  // Temporary accessors for testing persistency.
  assn_iterator begin() const { return ptrs_.begin(); }
  assn_iterator end() const { return ptrs_.end(); }
  assn_t const &operator[](typename ptrs_t::size_type index) const;
  assn_t const &at(typename ptrs_t::size_type index) const;
  typename ptrs_t::size_type size() const;

  void addSingle(Ptr<left_t> const &left,
                 Ptr<right_t> const &right);
  void Streamer(TBuffer &R_b);
  static short Class_Version() { return 10; }

  void swap(art::Assns<L, R, void> &other) { swap_(other); }

  std::auto_ptr<EDProduct> makePartner() const { return makePartner_(); }

protected:
  virtual void swap_(art::Assns<L, R, void> &other);
  virtual std::auto_ptr<EDProduct> makePartner_() const;

private:
  friend class detail::AssnsStreamer<left_t, right_t>;
  friend class art::Assns<right_t, left_t, void>; // partner_t.

  // FIXME: The only reason this function is virtual is to cause the
  // correct behavior to occur when the wrong streamer class is
  // called. In future (>5.30.00) versions of ROOT that can register
  // ioread rules for class template instantiations using typedefs, this
  // can be made back into a static function.
#ifdef ROOT_CAN_REGISTER_IOREADS_PROPERLY
  static
#else
  virtual
#endif
  bool left_first();

  void fill_transients();
  void fill_from_transients();

  void init_streamer();

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
  typedef art::Assns<right_t, left_t, data_t> partner_t;
  typedef typename base::assn_iterator assn_iterator;

  Assns();
  Assns(partner_t const &other);

  using base::size;
  using base::begin;
  using base::end;
  using base::operator[];
  using base::at;

  void swap(art::Assns<L, R, D> &other);

  std::auto_ptr<EDProduct> makePartner() const { return makePartner_(); }

  data_t const &data(typename std::vector<data_t>::size_type index) const;
  data_t const &data(assn_iterator it) const;

  void addSingle(Ptr<left_t> const &left,
                 Ptr<right_t> const &right,
                 data_t const &data);
  static short Class_Version() { return 10; }

private:
  friend class art::Assns<right_t, left_t, data_t>; // partner_t.

  virtual void swap_(art::Assns<L, R, void> &other);
  virtual std::auto_ptr<EDProduct> makePartner_() const;

  std::vector<data_t> data_;
};

////////////////////////////////////////////////////////////////////////
template <typename L, typename R>
inline
art::Assns<L, R, void>::Assns()
  :
  ptrs_(),
  ptr_data_1_(),
  ptr_data_2_()
{
  init_streamer();
}

template <typename L, typename R>
inline
art::Assns<L, R, void>::Assns(partner_t const &other)
  :
  ptrs_(),
  ptr_data_1_(),
  ptr_data_2_()
{
  ptrs_.reserve(other.ptrs_.size());
  for (typename partner_t::ptrs_t::const_iterator
         i = other.ptrs_.begin(),
         e = other.ptrs_.end();
       i != e;
       ++i) {
    ptrs_.push_back(std::make_pair(i->second, i->first));
  }
  init_streamer();
}

template <typename L, typename R>
inline
art::Assns<L, R, void>::~Assns()
{
}

template <typename L, typename R>
inline
typename art::Assns<L, R, void>::ptrs_t::value_type const &
art::Assns<L, R, void>::operator[](typename ptrs_t::size_type index) const {
  return ptrs_[index];
}

template <typename L, typename R>
inline
typename art::Assns<L, R, void>::ptrs_t::value_type const &
art::Assns<L, R, void>::at(typename ptrs_t::size_type index) const {
  return ptrs_.at(index);
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
void
art::Assns<L, R, void>::swap_(art::Assns<L, R, void> &other) {
  using std::swap;
  swap(ptrs_, other.ptrs_);
  swap(ptr_data_1_, other.ptr_data_1_);
  swap(ptr_data_2_, other.ptr_data_2_);
}

template <typename L, typename R>
std::auto_ptr<art::EDProduct>
art::Assns<L, R, void>::makePartner_() const{
  return
    std::auto_ptr<EDProduct>
    (new Wrapper<partner_t>
     (std::auto_ptr<partner_t>(new partner_t(*this))));
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

template <typename L, typename R>
void
art::Assns<L, R, void>::init_streamer()
{
  static TClassRef cl(TClass::GetClass(typeid(Assns<L,R,void>)));
  if (cl->GetStreamer() == 0) {
    cl->AdoptStreamer(new detail::AssnsStreamer<L,R>);
  }
}

template <typename L, typename R, typename D>
inline
art::Assns<L, R, D>::Assns()
  :
  Assns<L, R, void>()
{
  STATIC_ASSERT((!std::is_pointer<D>::value), "Data template argument must not be pointer type!");
}

template <typename L, typename R, typename D>
art::Assns<L, R, D>::Assns(partner_t const &other)
  :
  base(other),
  data_(other.data_)
{
}

template <typename L, typename R, typename D>
inline
void
art::Assns<L, R, D>::swap(Assns<L, R, D> &other) {
  using std::swap;
  base::swap_(other);
  swap(data_, other.data_);
}

template <typename L, typename R, typename D>
inline
typename art::Assns<L, R, D>::data_t const &
art::Assns<L, R, D>::data(typename std::vector<data_t>::size_type index) const {
  return data_.at(index);
}

template <typename L, typename R, typename D>
inline
typename art::Assns<L, R, D>::data_t const &
art::Assns<L, R, D>::data(assn_iterator it) const {
  return data_.at(it - begin());
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

template <typename L, typename R, typename D>
inline
void
art::Assns<L, R, D>::swap_(Assns<L, R, void> &other) {
  try {
    swap(dynamic_cast<Assns<L, R, D> &>(other));
  }
  catch (std::bad_cast &) {
    throw Exception(errors::LogicError, "AssnsBadCast")
      << "Attempt to swap base with derived!\n";
  }
}

template <typename L, typename R, typename D>
std::auto_ptr<art::EDProduct>
art::Assns<L, R, D>::makePartner_() const{
  return
    std::auto_ptr<EDProduct>
    (new Wrapper<partner_t>
     (std::auto_ptr<partner_t>(new partner_t(*this))));
}

#endif /* art_Persistency_Common_Assns_h */

// Local Variables:
// mode: c++
// End:
