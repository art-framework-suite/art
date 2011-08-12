#ifndef art_Persistency_Common_EDProduct_h
#define art_Persistency_Common_EDProduct_h

// ======================================================================
//
// EDProduct: The base class of each type that will be inserted into the
//            Event.
//
// ======================================================================

#include "art/Persistency/Common/fwd.h"
#include <string>
#include <vector>

namespace art {
  class EDProduct;
}

// ======================================================================

class art::EDProduct
{
public:
  EDProduct();
  virtual ~EDProduct();

  bool
    isPresent() const
  { return isPresent_(); }

  // We use vector<void *> so as to keep the type information out
  // of the EDProduct class.
  virtual void
    fillView( std::vector<void const *> & view ) const
  { /* should be called only polymorphically */ }

  void
    setPtr(std::type_info const &toType,
          unsigned long index,
          void const * &ptr) const;

  void
    getElementAddresses(std::type_info const &toType,
                       std::vector<unsigned long> const &indices,
                       std::vector<void const *> &ptr) const;

  virtual std::string
    productSize() const
  { return "-"; }

protected:

   virtual void do_setPtr(std::type_info const &toType,
                          unsigned long index,
                          void const * &ptr) const = 0;

   virtual void
   do_getElementAddresses(std::type_info const &toType,
                          std::vector<unsigned long> const &indices,
                          std::vector<void const *> &ptr) const = 0;
private:
  // These will never be called.
  // For technical ROOT related reasons, we cannot declare it = 0.
  virtual bool
    isPresent_() const
  { return true; }

};  // EDProduct

// ======================================================================

#endif /* art_Persistency_Common_EDProduct_h */

// Local Variables:
// mode: c++
// End:
