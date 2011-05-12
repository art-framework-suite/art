#ifndef art_Framework_Core_detail_maybe_record_parents_h
#define art_Framework_Core_detail_maybe_record_parents_h

#include "art/Framework/Core/DataViewImpl.h"
#include "art/Framework/Core/FCPfwd.h"
#include "cpp0x/type_traits"
#include <utility>

namespace art {
   namespace detail {
      template <typename T>
         struct has_donotrecordparents
         {
            static bool const value =
               std::is_base_of<art::DoNotRecordParents,T>::value;
         };

      template <typename T>
         struct RecordInParentless {
            void operator()(DataViewImpl::ProductPtrVec &used,
                            DataViewImpl::ProductPtrVec &ignored,
                            Wrapper<T>* wp,
                            ConstBranchDescription const* desc) const {
               used.push_back(std::make_pair(wp, desc));
            }
         };  // RecordInParentless<>

      template <typename T>
         struct RecordInParentfull {
            void operator()(DataViewImpl::ProductPtrVec &used,
                            DataViewImpl::ProductPtrVec &ignored,
                            Wrapper<T>* wp,
                            ConstBranchDescription const* desc) const {
               used.push_back(std::make_pair(wp, desc));
            }
         };  // RecordInParentfull<>

      template <typename T>
         void maybe_record_parents(DataViewImpl::ProductPtrVec &used,
                                   DataViewImpl::ProductPtrVec &ignored,
                                   Wrapper<T> *wp,
                                   ConstBranchDescription const *desc) {
         typename std::conditional<
            has_donotrecordparents<T>::value,
            RecordInParentless<T>,
            RecordInParentfull<T>
            >::type parentage_recorder;
      parentage_recorder(used,
                         ignored,
                         wp,
                         desc);
   }

      // ----------------------------------------------------------------------


   }
}

#endif /* art_Framework_Core_detail_maybe_record_parents_h */

// Local Variables:
// mode: c++
// End:
