#ifndef art_Framework_Core_detail_maybe_call_post_insert_h
#define art_Framework_Core_detail_maybe_call_post_insert_h

#include "cpp0x/type_traits"

// has_postinsert is a metafunction of one argument, the type T.  As
// with many metafunctions, it is implemented as a class with a data
// member 'value', which contains the value 'returned' by the
// metafunction.
//
// has_postinsert<T>::value is 'true' if T has the post_insert member
// function (with the right signature), and 'false' if T has no such
// member function.

namespace art {
   namespace detail {
      typedef char (& no_tag )[1]; // type indicating FALSE
      typedef char (& yes_tag)[2]; // type indicating TRUE

      // Definitions forthe following struct and function templates are
      // not needed; we only require the declarations.
      template <typename T, void (T::*)()>  struct postinsert_function;
      template <typename T> no_tag  has_postinsert_helper(...);
      template <typename T> yes_tag has_postinsert_helper(postinsert_function<T, &T::post_insert> * p);

      template<typename T>
         struct has_postinsert
         {
            static bool const value =
               sizeof(has_postinsert_helper<T>(0)) == sizeof(yes_tag) &&
               !std::is_base_of<art::DoNotSortUponInsertion, T>::value;
         };

      template <typename T>
         struct DoPostInsert
         {
            void operator()(T* p) const { p->post_insert(); }
         };

      template <typename T>
         struct DoNotPostInsert
         {
            void operator()(T*) const { }
         };

      template <typename T>
         void maybe_call_post_insert(T* p) {
         // The following will call post_insert if T has such a function,
         // and do nothing if T has no such function.
         typename std::conditional<
            has_postinsert<T>::value,
            DoPostInsert<T>,
            DoNotPostInsert<T>
            >::type maybe_inserter;
         maybe_inserter(p);

      }
   }
}
#endif /* art_Framework_Core_detail_maybe_call_post_insert_h */

// Local Variables:
// mode: c++
// End:
