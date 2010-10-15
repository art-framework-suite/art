#ifndef DataFormats_Common_HolderToVectorTrait_h
#define DataFormats_Common_HolderToVectorTrait_h
#include "art/Utilities/EDMException.h"
//#include <boost/static_assert.hpp>

namespace art {
  namespace reftobase {
    class RefVectorHolderBase;
    template <typename T> class BaseVectorHolder;

    template <typename T, typename REF>
    struct InvaidHolderToVector {
      static std::auto_ptr<BaseVectorHolder<T> > makeVectorHolder() {
	throw art::Exception(errors::InvalidReference)
	  << "InvaidHolderToVector: trying to use RefToBase built with "
	  << "an internal type. RefToBase should be built passing an "
	  << "object of type art::Ref<C>. This exception should never "
	  << "be thrown if a RefToBase was built from a RefProd<C>.";
      }
      static std::auto_ptr<RefVectorHolderBase> makeVectorBaseHolder() {
	throw art::Exception(errors::InvalidReference)
	  << "InvaidHolderToVector: trying to use RefToBase built with "
	  << "an internal type. RefToBase should be built passing an "
	  << "object of type art::Ref<C>. This exception should never "
	  << "be thrown if a RefToBase was built from a RefProd<C>.";
      }
    };

    template<typename T, typename REF>
    struct HolderToVectorTrait {
      //      BOOST_STATIC_ASSERT(sizeof(REF) == 0);
      typedef InvaidHolderToVector<T, REF> type;
    };

    template <typename REF>
    struct InvalidRefHolderToRefVector {
      static std::auto_ptr<RefVectorHolderBase> makeVectorHolder() {
	throw art::Exception(errors::InvalidReference)
	  << "InvaidRefHolderToRefVector: trying to use RefToBaseVector built with "
	  << "an internal type. RefToBase should be built passing an "
	  << "object of type art::RefVector<C>";
      }
      static std::auto_ptr<RefVectorHolderBase> makeVectorBaseHolder() {
	throw art::Exception(errors::InvalidReference)
	  << "InvaidRefHolderToRefVector: trying to use RefToBaseVector built with "
	  << "an internal type. RefToBase should be built passing an "
	  << "object of type art::RefVector<C>";
      }
    };

    template<typename REF>
    struct RefHolderToRefVectorTrait {
      //      BOOST_STATIC_ASSERT(sizeof(REF) == 0);
      typedef InvalidRefHolderToRefVector<REF> type;
    };

  }
}

#endif
