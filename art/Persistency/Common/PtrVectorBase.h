#ifndef DataFormats_Common_PtrVectorBase_h
#define DataFormats_Common_PtrVectorBase_h

// ======================================================================
//
// PtrVectorBase: provide PtrVector<T> behavior that's independent of T
//
// ======================================================================

#include "art/Persistency/Common/RefCore.h"
#include <vector>
#include <typeinfo>
#include <utility>

namespace art {
  class PtrVectorBase;
}

// ======================================================================

class art::PtrVectorBase
{
public:
  typedef  unsigned long  key_type;
  typedef  key_type       size_type;

  // --- Construction/destruction:

  virtual
    ~PtrVectorBase();

  // --- Observers:

  bool       isNonnull()      const { return core_.isNonnull(); }
  bool       isNull()         const { return ! isNonnull(); }
  bool       operator ! ()    const { return isNull(); }

  bool       hasCache()       const { return !cachedItems_.empty(); }

  bool       isAvailable()    const { return core_.isAvailable(); }
  bool       isTransient()    const { return core_.isTransient(); }
  ProductID  id()             const { return core_.id(); }
  EDProductGetter const *
             productGetter()  const { return core_.productGetter(); }

  bool       empty()          const { return indicies_.empty(); }
  size_type  size()           const { return indicies_.size(); }
  size_type  capacity()       const { return indicies_.capacity(); }

  bool       operator == ( PtrVectorBase const & ) const;

  // --- Mutators ---

  void
    clear()
  { core_ = RefCore(), indicies_.clear(), cachedItems_.clear(); }

  void
    reserve( size_type n )
  { indicies_.reserve(n), cachedItems_.reserve(n); }

  void
    setProductGetter( EDProductGetter * g ) const
  { core_.setProductGetter(g); }

protected:
  PtrVectorBase();

  // --- Observers:

  std::vector<void const *>::const_iterator
    void_begin() const
  { return getProduct_(), cachedItems_.begin(); }

  std::vector<void const *>::const_iterator
    void_end() const
  { return getProduct_(), cachedItems_.end(); }

  template< typename TPtr >
    TPtr
    makePtr( unsigned long idx ) const
  {
    typedef  typename TPtr::value_type const *  v_ptr;

    if( isTransient() )   return TPtr( reinterpret_cast<v_ptr>(cachedItems_[idx])
                                     , indicies_[idx] );
    else if( hasCache() ) return TPtr( this->id()
                                     , reinterpret_cast<v_ptr>(cachedItems_[idx])
                                     , indicies_[idx] );
    else                  return TPtr( this->id()
                                     , indicies_[idx]
                                     , productGetter() );
  }

  template< typename TPtr >
    TPtr
    makePtr( std::vector<void const *>::const_iterator const it ) const
  {
    typedef  typename TPtr::value_type const *  v_ptr;

    if( isTransient() )   return TPtr( reinterpret_cast<v_ptr>(*it)
                                     , indicies_[it - cachedItems_.begin()] );
    else if( hasCache() ) return TPtr( this->id()
                                     , reinterpret_cast<v_ptr>(*it)
                                     , indicies_[it - cachedItems_.begin()] );
    else                  return TPtr( this->id()
                                     , indicies_[it - cachedItems_.begin()]
                                     , productGetter() );
  }

  // --- Mutators:

  void push_back_base( RefCore const &
                     , key_type
                     , void const * );
  void swap( PtrVectorBase & );

  void
    swap( key_type k1, key_type k2 )
  {
    std::swap( indicies_[k1], indicies_[k2] );
    std::swap( cachedItems_[k1], cachedItems_[k2] );
  }

private:
  RefCore                            core_;
  std::vector<key_type>              indicies_;
  mutable std::vector<void const *>  cachedItems_;  //! transient

  // --- Helpers:

  void
    getProduct_() const;
  virtual std::type_info const &
    typeInfo() const = 0;

};  // PtrVectorBase

// ======================================================================

#endif
