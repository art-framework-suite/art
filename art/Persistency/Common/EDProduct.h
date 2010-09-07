#ifndef DataFormats_Common_EDProduct_h
#define DataFormats_Common_EDProduct_h

// ======================================================================
//
// EDProduct: The base class of each type that will be inserted into the
//            Event.
//
// ======================================================================


// Framework support:
#include "art/Persistency/Common/EDProductfwd.h"

// C++ support:
#include <vector>

// Contents:
namespace edm {
  class EDProduct;
}


// ======================================================================


class edm::EDProduct
{
public:
  EDProduct();
  virtual ~EDProduct();

  bool
    isPresent() const
  { return isPresent_(); }

  // We use vector<void *> so as to keep the type information out
  // of the EDProduct class.
  void
    fillView( ProductID const &           id
            , std::vector<void const *> & view
            , helper_vector_ptr &         helpers ) const;

  void
    setPtr( std::type_info const & iToType
          , unsigned long          iIndex
          , void const * &         oPtr ) const;

  void
    fillPtrVector( std::type_info const &             iToType
                 , std::vector<unsigned long> const & iIndicies
                 , std::vector<void const *> &        oPtr ) const;

#ifndef __REFLEX__

  bool
    isMergeable() const
  { return isMergeable_(); }

  bool
    mergeProduct( EDProduct const * newProduct )
  { return mergeProduct_(newProduct); }

  bool
    hasIsProductEqual() const
  { return hasIsProductEqual_(); }

  bool
    isProductEqual( EDProduct const * newProduct ) const
  { return isProductEqual_(newProduct); }

#endif  // __REFLEX__

private:
  // These will never be called.
  // For technical ROOT related reasons, we cannot declare it = 0.
  virtual bool
    isPresent_() const
  { return true; }

#ifndef __REFLEX__

  virtual bool
    isMergeable_() const
  { return true; }

  virtual bool
    mergeProduct_( EDProduct const * newProduct )
  { return true; }

  virtual bool
    hasIsProductEqual_() const
  { return true; }

  virtual bool
    isProductEqual_( EDProduct const * newProduct ) const
  { return true; }

#endif  // __REFLEX__

  virtual void
    do_setPtr( std::type_info const & iToType
             , unsigned long          iIndex
             , void const * &         oPtr ) const = 0;

  virtual void
    do_fillPtrVector( std::type_info const &             iToType
                    , std::vector<unsigned long> const & iIndicies
                    , std::vector<void const *> &        oPtr ) const = 0;

};  // EDProduct


// ======================================================================

#endif  // DataFormats_Common_EDProduct_h
