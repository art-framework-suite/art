#ifndef DataFormats_Common_FillView_h
#define DataFormats_Common_FillView_h

/*----------------------------------------------------------------------

Several fillView function templates, to provide View support for
Standard Library containers.



----------------------------------------------------------------------*/

#include <string>
#include <vector>
#include <list>
#include <deque>
#include <set>
#include "art/Persistency/Common/RefTraits.h"
#include "art/Persistency/Common/RefVectorTraits.h"
#include "art/Persistency/Common/GetProduct.h"

namespace edm {
  template<typename C, typename T, typename F> class RefVector;
  template<typename C, typename T, typename F> class Ref;
  namespace detail {

    // Function template reallyFillView<C> can fill views for
    // standard-library collections of appropriate types (vector, list,
    // deque, set).

    template<typename C, typename T, typename F>
    struct FillViewRefTypeTrait {
      typedef Ref<C, T, F> type;
    };

    template<typename C, typename T, typename F, typename T1, typename F1>
    struct FillViewRefTypeTrait<RefVector<C, T, F>, T1, F1> {
      typedef typename refhelper::RefVectorTrait<C, T, F>::ref_type type;
    };
  }
}

#include "art/Persistency/Common/EDProductfwd.h"
#include "art/Persistency/Common/RefHolder_.h"
#include "art/Persistency/Provenance/ProductID.h"
#include "art/Persistency/Common/RefVectorHolderBase.h"

namespace edm {
  namespace detail {

    template <class COLLECTION>
    void
    reallyFillView(COLLECTION const& coll,
		   ProductID const& id,
		   std::vector<void const*>& ptrs,
		   helper_vector& helpers)
    {
      typedef COLLECTION                            product_type;
      typedef typename GetProduct<product_type>::element_type     element_type;
      typedef typename product_type::const_iterator iter;
      typedef typename product_type::size_type      size_type;
      typedef typename FillViewRefTypeTrait<product_type,
	typename refhelper::ValueTrait<product_type>::value,
	typename refhelper::FindTrait<product_type,
	typename refhelper::ValueTrait<product_type>::value>::value>::type ref_type;
      typedef reftobase::RefHolder<ref_type>        holder_type;

      size_type key = 0;
      for (iter i = coll.begin(), e = coll.end(); i!=e; ++i, ++key) {
	element_type const* address = GetProduct<product_type>::address(i);
	ptrs.push_back(address);
	ref_type ref(id, address, key, GetProduct<product_type>::product(coll) );
	holder_type h(ref);
	helpers.push_back(&h);
      }
    }
  }

  template <class T, class A>
  void
  fillView(std::vector<T,A> const& obj,
	   ProductID const& id,
	   std::vector<void const*>& ptrs,
	   helper_vector& helpers)
  {
    detail::reallyFillView(obj, id, ptrs, helpers);
  }

  template <class T, class A>
  void
  fillView(std::list<T,A> const& obj,
	   ProductID const& id,
	   std::vector<void const*>& ptrs,
	   helper_vector& helpers)
  {
    detail::reallyFillView(obj, id, ptrs, helpers);
  }

  template <class T, class A>
  void
  fillView(std::deque<T,A> const& obj,
	   ProductID const& id,
	   std::vector<void const*>& ptrs,
	   helper_vector& helpers)
  {
    detail::reallyFillView(obj, id, ptrs, helpers);
  }

  template <class T, class A, class Comp>
  void
  fillView(std::set<T,A,Comp> const& obj,
	   ProductID const& id,
	   std::vector<void const*>& ptrs,
	   helper_vector& helpers)
  {
    detail::reallyFillView(obj, id, ptrs, helpers);
  }

}

#include "art/Persistency/Common/RefHolder.h"

/*
#include "art/Persistency/Common/Ref.h"
#include "art/Persistency/Common/RefVector.h"
#include "art/Persistency/Common/RefToBase.h"
*/

#endif
