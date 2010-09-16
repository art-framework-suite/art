#ifndef DataFormats_Common_EDProductGetter_h
#define DataFormats_Common_EDProductGetter_h

//
// Package:     EDProduct
// Class  :     EDProductGetter
//
/**\class EDProductGetter EDProductGetter.h DataFormats/Common/interface/EDProductGetter.h

 Description: Abstract base class used internally by the RefBase to obtain the EDProduct from the Event

 Usage:
    This is used internally by the edm::Ref classes.

*/


#include "art/Persistency/Provenance/ProductID.h"

#include "boost/utility.hpp"


namespace edm {
   class EDProduct;

   class EDProductGetter
     : private boost::noncopyable {
   public:
      EDProductGetter();
      virtual ~EDProductGetter();

      // ---------- const member functions ---------------------
      virtual EDProduct const* getIt(ProductID const&) const = 0;

      // ---------- static member functions --------------------
      static EDProductGetter const* instance();

      ///Helper class to make the EDProductGetter accessible on at the proper times
      class Operate {
       public:
         Operate(EDProductGetter const* iGet) : previous_(EDProductGetter::set(iGet)) {
         }
         ~Operate() {
            EDProductGetter::set(previous_);
         }
       private:
         EDProductGetter const* previous_;
      };

      friend class Operate;

      ProductID oldToNewProductID(ProductID const& oldProductID) const {
        if (oldProductID.oldID() == 0) return oldProductID;
        return oldToNewProductID_(oldProductID);
      }
private:
      virtual ProductID oldToNewProductID_(ProductID const& oldProductID) const;
      /**This does not take ownership of the argument, so it is up to the caller to be
         sure that the object lifetime is greater than the time for which it is set*/
      static EDProductGetter const* set(EDProductGetter const*);

   };

   EDProductGetter const*
   mustBeNonZero(EDProductGetter const* prodGetter, std::string refType, ProductID const& productID);

}  // namespace edm

#endif  // DataFormats_Common_EDProductGetter_h
