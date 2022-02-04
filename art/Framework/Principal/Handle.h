#ifndef art_Framework_Principal_Handle_h
#define art_Framework_Principal_Handle_h
// vim: set sw=2 expandtab :

//==========================================================================
//  Handle: Non-owning "smart pointer" for reference to EDProducts and
//          their Provenances.
//
//  ValidHandle: A Handle that can not be invalid, and thus does not check
//               for validity upon dereferencing.
//
//  If the pointed-to EDProduct or Provenance is destroyed, use of the
//  Handle becomes undefined. There is no way to query the Handle to
//  discover if this has happened.
//
//  Handles can have:
//  -- Product and Provenance pointers both null;
//  -- Both pointers valid
//
//  ValidHandles must have Product and Provenance pointers valid.
//
//  To check validity, the art::Handle is implicitly convertible to
//  bool.  One can also use the Handle::isValid() function.
//  ValidHandles cannot be invalid, and so have no validity checks.
//
//  A data product provided by the input source may be removed from
//  memory by calling removeProduct(), allowing program memory to be
//  reclaimed when the product is no longer needed.
//
//  If failedToGet() returns true then the requested data is not available
//  If failedToGet() returns false but isValid() is also false then no
//  attempt to get data has occurred
//==========================================================================

#include "art/Framework/Principal/Group.h"
#include "art/Framework/Principal/Provenance.h"
#include "art/Persistency/Common/GroupQueryResult.h"
#include "canvas/Persistency/Common/detail/is_handle.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib_except/demangle.h"
#include "cetlib_except/exception.h"

#include <memory>
#include <typeinfo>

namespace art {

  template <typename T>
  class Handle;
  template <typename T>
  class ValidHandle;
  template <typename T>
  class PutHandle;

  template <class T>
  void swap(Handle<T>& a, Handle<T>& b);
  template <class T>
  void swap(ValidHandle<T>& a, ValidHandle<T>& b);
  template <class T>
  void convert_handle(GroupQueryResult const&, Handle<T>&);

  class EDProduct;
  template <typename T>
  class Wrapper;

  namespace detail {
    template <typename... T>
    void
    throw_if_invalid(std::string const& msg, T const&... t)
    {
      bool const all_valid = true && (... && t.isValid());
      if (!all_valid) {
        throw Exception{art::errors::NullPointerError} << msg << '\n';
      }
    }
  } // namespace detail

  template <class T>
  std::enable_if_t<detail::is_handle<T>::value, RangeSet const&>
  range_of_validity(T const& h);
  template <class T, class U>
  std::enable_if_t<detail::are_handles<T, U>::value, bool> same_ranges(
    T const& a,
    U const& b);
  template <class T, class U>
  std::enable_if_t<detail::are_handles<T, U>::value, bool> disjoint_ranges(
    T const& a,
    U const& b);
  template <class T, class U>
  std::enable_if_t<detail::are_handles<T, U>::value, bool> overlapping_ranges(
    T const& a,
    U const& b);

} // namespace art

template <typename T>
class art::Handle {
public:
  using element_type = T;
  class HandleTag {};

  ~Handle() = default;
  explicit constexpr Handle() =
    default; // Default-constructed handles are invalid.
  explicit Handle(GroupQueryResult const&);
  Handle(Handle const&) = default;
  Handle(Handle&&) = default;
  Handle& operator=(Handle const&) = default;
  Handle& operator=(Handle&&) = default;

  // pointer behaviors:
  T const& operator*() const;
  T const* operator->() const; // alias for product()
  T const* product() const;

  // inspectors:
  explicit operator bool() const noexcept;
  bool isValid() const noexcept;
  bool failedToGet()
    const; // was Handle used in a 'get' call whose data could not be found?
  Provenance const* provenance() const;
  ProductID id() const;
  std::shared_ptr<art::Exception const> whyFailed() const;
  EDProductGetter const* productGetter() const noexcept;

  // mutators:
  void swap(Handle<T>& other);
  void clear();
  bool removeProduct();

private:
  T const* prod_{nullptr};
  cet::exempt_ptr<Group> group_{nullptr};
  Provenance prov_{};
  std::shared_ptr<art::Exception const> whyFailed_{nullptr};
}; // Handle<>

// ----------------------------------------------------------------------
// c'tors:

template <class T>
art::Handle<T>::Handle(GroupQueryResult const& gqr)
  : group_{gqr.result()}, prov_{group_}, whyFailed_{gqr.whyFailed()}
{
  if (gqr.succeeded()) {
    auto const wrapperPtr = dynamic_cast<Wrapper<T> const*>(
      gqr.result()->uniqueProduct(TypeID{typeid(Wrapper<T>)}));
    if (wrapperPtr != nullptr) {
      prod_ = wrapperPtr->product();
    } else {
      Exception e{errors::LogicError};
      e << "Product retrieval via Handle<T> succeeded for product:\n"
        << prov_.productDescription()
        << "but an attempt to interpret it as an object of type '"
        << cet::demangle_symbol(typeid(T).name()) << "' failed.\n";
      whyFailed_ = std::make_shared<art::Exception const>(std::move(e));
    }
  }
}

// ----------------------------------------------------------------------
// pointer behaviors:

template <class T>
inline T const& art::Handle<T>::operator*() const
{
  return *product();
}

template <class T>
T const*
art::Handle<T>::product() const
{
  if (failedToGet()) {
    throw *whyFailed_;
  }
  if (prod_ == nullptr)
    throw Exception(art::errors::NullPointerError)
      << "Attempt to de-reference product that points to 'nullptr'.\n";
  return prod_;
}

template <class T>
inline T const* art::Handle<T>::operator->() const
{
  return product();
}

// ----------------------------------------------------------------------
// inspectors:

template <class T>
inline art::Handle<T>::operator bool() const noexcept
{
  return isValid();
}

template <class T>
bool
art::Handle<T>::isValid() const noexcept
{
  return prod_ && prov_.isValid();
}

template <class T>
bool
art::Handle<T>::failedToGet() const
{
  return whyFailed_.get();
}

template <class T>
inline art::Provenance const*
art::Handle<T>::provenance() const
{
  return &prov_;
}

template <class T>
inline art::ProductID
art::Handle<T>::id() const
{
  return prov_.isValid() ? prov_.productID() : ProductID{};
}

template <class T>
inline std::shared_ptr<art::Exception const>
art::Handle<T>::whyFailed() const
{
  return whyFailed_;
}

template <typename T>
inline art::EDProductGetter const*
art::Handle<T>::productGetter() const noexcept
{
  return group_.get();
}

// ----------------------------------------------------------------------
// mutators:

template <class T>
void
art::Handle<T>::swap(Handle<T>& other)
{
  std::swap(*this, other);
}

template <class T>
void
art::Handle<T>::clear()
{
  Handle<T> tmp;
  swap(tmp);
}

template <class T>
inline bool
art::Handle<T>::removeProduct()
{
  if (isValid() && !prov_.produced()) {
    assert(group_);
    group_->removeCachedProduct();
    clear();
    return true;
  }
  return false;
}

// ======================================================================
// Non-members:

// Convert from handle-to-EDProduct to handle-to-T
template <class T>
void
art::convert_handle(GroupQueryResult const& orig, Handle<T>& result)
{
  Handle<T> h{orig};
  result.swap(h);
}

// ======================================================================
template <typename T>
class art::ValidHandle {
public:
  using element_type = T;
  class HandleTag {};

  ~ValidHandle() = default;
  ValidHandle() = delete;
  explicit ValidHandle(T const* prod,
                       EDProductGetter const* productGetter,
                       Provenance prov);
  ValidHandle(ValidHandle const&) = default;
  ValidHandle(ValidHandle&&) = default;
  ValidHandle& operator=(ValidHandle const&) & = default;
  ValidHandle& operator=(ValidHandle&&) & = default;

  // pointer behaviors
  operator T const*() const; // conversion to T const*
  T const& operator*() const;
  T const* operator->() const; // alias for product()
  T const* product() const;

  // inspectors
  bool isValid() const;     // always true
  bool failedToGet() const; // always false
  Provenance const* provenance() const;
  ProductID id() const;
  std::shared_ptr<art::Exception const> whyFailed() const; // always null
  EDProductGetter const* productGetter() const noexcept;

  // mutators
  void swap(ValidHandle<T>& other);

  // No clear() function, because a ValidHandle does not have an invalid
  // state, and that is what the result of clear() would have to be.

private:
  T const* prod_;
  EDProductGetter const* productGetter_;
  Provenance prov_;
};

template <class T>
art::ValidHandle<T>::ValidHandle(T const* prod,
                                 EDProductGetter const* productGetter,
                                 Provenance prov)
  : prod_{prod}, productGetter_{productGetter}, prov_{prov}
{
  if (prod_ == nullptr) {
    throw Exception(art::errors::NullPointerError)
      << "Attempt to create ValidHandle with null pointer";
  }
}

template <class T>
inline art::ValidHandle<T>::operator T const*() const
{
  return prod_;
}

template <class T>
inline T const& art::ValidHandle<T>::operator*() const
{
  return *prod_;
}

template <class T>
inline T const* art::ValidHandle<T>::operator->() const
{
  return prod_;
}

template <class T>
inline T const*
art::ValidHandle<T>::product() const
{
  return prod_;
}

template <class T>
inline bool
art::ValidHandle<T>::isValid() const
{
  return true;
}

template <class T>
inline bool
art::ValidHandle<T>::failedToGet() const
{
  return false;
}

template <class T>
inline art::Provenance const*
art::ValidHandle<T>::provenance() const
{
  return &prov_;
}

template <class T>
inline art::ProductID
art::ValidHandle<T>::id() const
{
  return prov_.productID();
}

template <class T>
inline std::shared_ptr<art::Exception const>
art::ValidHandle<T>::whyFailed() const
{
  return std::shared_ptr<art::Exception const>();
}

template <class T>
inline void
art::ValidHandle<T>::swap(art::ValidHandle<T>& other)
{
  std::swap(*this, other);
}

template <typename T>
inline art::EDProductGetter const*
art::ValidHandle<T>::productGetter() const noexcept
{
  return productGetter_;
}

// ======================================================================
// Non-members:

template <class T>
std::enable_if_t<art::detail::is_handle<T>::value, art::RangeSet const&>
art::range_of_validity(T const& h)
{
  std::string const& errMsg =
    "Attempt to retrieve range set from invalid handle.";
  detail::throw_if_invalid(errMsg, h);
  return h.provenance()->rangeOfValidity();
}

template <class T, class U>
std::enable_if_t<art::detail::are_handles<T, U>::value, bool>
art::same_ranges(T const& a, U const& b)
{
  std::string const& errMsg =
    "Attempt to compare range sets where one or both handles are invalid.";
  detail::throw_if_invalid(errMsg, a, b);
  return same_ranges(range_of_validity(a), range_of_validity(b));
}

template <class T, class U>
std::enable_if_t<art::detail::are_handles<T, U>::value, bool>
art::disjoint_ranges(T const& a, U const& b)
{
  std::string const& errMsg =
    "Attempt to compare range sets where one or both handles are invalid.";
  detail::throw_if_invalid(errMsg, a, b);
  return disjoint_ranges(range_of_validity(a), range_of_validity(b));
}

template <class T, class U>
std::enable_if_t<art::detail::are_handles<T, U>::value, bool>
art::overlapping_ranges(T const& a, U const& b)
{
  std::string const& errMsg =
    "Attempt to compare range sets where one or both handles are invalid.";
  detail::throw_if_invalid(errMsg, a, b);
  return overlapping_ranges(range_of_validity(a), range_of_validity(b));
}

// ======================================================================
template <typename T>
class art::PutHandle {
public:
  using element_type = T;
  class HandleTag {};

  explicit PutHandle(T const* prod,
                     EDProductGetter const* productGetter,
                     ProductID id);

  // To be deprecated
  // ----------------
  // The following implicit conversion to ProductID is retained for
  // backwards compatibility--up through art 3.10, the supported
  // interface is (e.g.):
  //
  //   ProductID id = e.put(move(some_product));
  //
  // With art 3.11, the return type of 'put' will be a PutHandle<T>
  // object.  Until users have time to migrate to the new usage, the
  // PutHandle template will provide a conversion operator to
  // ProductID.
  operator ProductID() const;

  // pointer behaviors
  T const& operator*() const;
  T const* operator->() const; // alias for product()
  T const* product() const;

  // inspectors
  ProductID id() const;
  EDProductGetter const* productGetter() const noexcept;

private:
  T const* prod_;
  EDProductGetter const* productGetter_;
  ProductID id_;
};

template <class T>
inline art::PutHandle<T>::PutHandle(T const* prod,
                                    EDProductGetter const* productGetter,
                                    ProductID id)
  : prod_{prod}, productGetter_{productGetter}, id_{id}
{}

template <class T>
inline art::PutHandle<T>::operator ProductID() const
{
  return id();
}

template <class T>
inline T const& art::PutHandle<T>::operator*() const
{
  return *prod_;
}

template <class T>
inline T const* art::PutHandle<T>::operator->() const
{
  return prod_;
}

template <class T>
inline T const*
art::PutHandle<T>::product() const
{
  return prod_;
}

template <class T>
inline art::ProductID
art::PutHandle<T>::id() const
{
  return id_;
}

template <typename T>
inline art::EDProductGetter const*
art::PutHandle<T>::productGetter() const noexcept
{
  return productGetter_;
}

#endif /* art_Framework_Principal_Handle_h */

// Local Variables:
// mode: c++
// End:
