#ifndef art_Persistency_Common_EDProductGetter_h
#define art_Persistency_Common_EDProductGetter_h

//
// EDProductGetter
//
/*

 Description: Abstract base class used internally by the RefBase to obtain the EDProduct from the Event

 Usage:
    This is used internally by the art::Ref classes.
*/


#include "art/Persistency/Provenance/ProductID.h"

#include "boost/noncopyable.hpp"


namespace art {
   class EDProduct;

   class EDProductGetter
     : private boost::noncopyable
     {
   public:
      EDProductGetter();
      virtual ~EDProductGetter();

      ////virtual EDProduct const* getIt(ProductID const&) const = 0;
      virtual EDProduct const* getIt() const = 0;

      //Helper class to make the EDProductGetter accessible on at the proper times
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

private:
      /**This does not take ownership of the argument, so it is up to the caller to be
         sure that the object lifetime is greater than the time for which it is set*/
      static EDProductGetter const* set(EDProductGetter const*);

   };  // EDProductGetter

   EDProductGetter const*
   mustBeNonZero(EDProductGetter const* prodGetter, std::string refType, ProductID const& productID);

}  // art

// ======================================================================

#endif /* art_Persistency_Common_EDProductGetter_h */

// Local Variables:
// mode: c++
// End:
