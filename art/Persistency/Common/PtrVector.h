#ifndef DataFormats_Common_PtrVector_h
#define DataFormats_Common_PtrVector_h

// ======================================================================
//
// PtrVector: a container which returns edm::Ptr<>'s referring to items
//            in one container in the edm::Event
//
// ======================================================================


// Framework support:
#include "art/Persistency/Common/Ptr.h"
#include "art/Persistency/Common/PtrVectorBase.h"

// Boost library support:
#include "boost/iterator.hpp"
#include "boost/static_assert.hpp"
#include "boost/type_traits/is_base_of.hpp"

// C++ support:
#include <iterator>
#include <vector>

// Contents:
namespace edm {
  template< typename > class PtrProxy;
  template< typename > class PtrVector;
  template< typename > class PtrVectorItr;

  template< typename T >
    void  swap( PtrVector<T> &, PtrVector<T> & );
 }


// ======================================================================


template< typename T >
  class edm::PtrProxy
{
private:
  Ptr<T> ptr_;

public:
  // --- Construction/destruction:

  PtrProxy( Ptr<T> const & p ) : ptr_(p) { }

  // --- Observers:

  Ptr<T> const &  operator *  () const { return ptr_; }
  Ptr<T> const *  operator -> () const { return & ptr_; }

};  // PtrProxy<>


// ======================================================================


template< typename T >
  class edm::PtrVectorItr
: public std::iterator< std::random_access_iterator_tag, Ptr<T> >
{
private:
  typedef  std::iterator< std::random_access_iterator_tag, Ptr<T> >
           base_t;

  std::vector<void const *>::const_iterator  iter_;
  PtrVector<T> const *                       base_;

public:
  typedef  PtrVectorItr<T>                   iterator;
  typedef  typename base_t::difference_type  difference_type;

  // --- Construction/destruction:

    PtrVectorItr( std::vector<void const *>::const_iterator const & it
                , PtrVector<T> const *                              b )
  : base_t( )
  , iter_ ( it )
  , base_ ( b )
 { }

  // --- Observers:

  Ptr<T>
    operator * () const
  { return base_->fromItr(iter_); }
  PtrProxy<T>
    operator-> () const
  { return PtrProxy<T>( operator *() ); }

  // --- Operators:

  iterator &
    operator ++ ()
  { ++iter_; return *this; }
  iterator &
    operator -- ()
  { --iter_; return *this; }

  iterator &
    operator += ( difference_type n )
  { iter_ += n; return *this; }
  iterator &
    operator -= ( difference_type n )
  { iter_ -= n; return *this; }

  iterator
    operator ++ (int)
  { iterator it(*this); ++iter_; return it; }
  iterator
    operator -- (int)
  { iterator it(*this); --iter_; return it; }

  iterator
    operator + ( difference_type n ) const
  { iterator it(*this); return it += n; }

  iterator
    operator - ( difference_type n ) const
  { iterator it(*this); return it -= n; }

  difference_type
    operator - ( iterator const & other ) const
  { return iter_ - other.iter_; }

  bool
    operator == ( iterator const & other ) const
  { return iter_ == other.iter_; }
  bool
    operator != ( iterator const & other ) const
  { return iter_ != other.iter_; }
  bool
    operator <  ( iterator const & other ) const
  { return iter_ < other.iter_; }
  bool
    operator >  ( iterator const & other ) const
  { return iter_ > other.iter_; }
  bool
    operator <= ( iterator const & other ) const
  { return iter_ <= other.iter_; }
  bool
    operator >= ( iterator const & other ) const
  { return iter_ >= other.iter_; }

};  // PtrVectorItr<>


// ======================================================================


template< typename T >
class edm::PtrVector
: public PtrVectorBase
{
  friend class PtrVectorItr<T>;

public:
  typedef  PtrVectorItr<T>  const_iterator;
  typedef  Ptr<T>           value_type;

  // --- Construction/destruction:

    PtrVector()
  : PtrVectorBase( )
  { }

  template< typename U >
    PtrVector( PtrVector<U> const & other )
  : PtrVectorBase( other )
  { BOOST_STATIC_ASSERT(( boost::is_base_of<T,U>::value )); }

  // --- Observers:

  Ptr<T>
    operator [] ( unsigned long const idx ) const
  { return makePtr<Ptr<T> >(idx); }

  const_iterator
    begin() const
  { return const_iterator(void_begin(), this); }

  const_iterator
    end() const
  { return const_iterator(void_end(), this); }

  // --- Mutators:

  void
    push_back( Ptr<T> const & p )
  {
    push_back_base( p.refCore()
                  , p.key()
                  , p.hasCache() ? p.operator->()
                                 : static_cast<void const *>(0)
                  );
  }

  template< typename U >
  void
    push_back( Ptr<U> const & p )
  {
    // ensure that types are compatible
    BOOST_STATIC_ASSERT(( boost::is_base_of<T,U>::value ));
    push_back_base( p.refCore()
                  , p.key()
                  , p.hasCache() ? p.operator->()
                                 : static_cast<void const *>(0)
                  );
  }

  void
    swap( PtrVector & other )
  { PtrVectorBase::swap(other); }

private:
  // --- Helpers:

  std::type_info const &
    typeInfo() const
  { return typeid(T); }

  Ptr<T>
    fromItr( std::vector<void const *>::const_iterator const & it ) const
  { return makePtr< Ptr<T> >(it); }

};  // PtrVector<T>


template< typename T >
  inline void
  edm::swap( PtrVector<T> & lhs, PtrVector<T> & rhs )
{ lhs.swap(rhs); }


// ======================================================================

#endif  // DataFormats_Common_PtrVector_h
