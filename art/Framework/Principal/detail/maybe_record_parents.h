#ifndef art_Framework_Principal_detail_maybe_record_parents_h
#define art_Framework_Principal_detail_maybe_record_parents_h

#include "art/Framework/Principal/DataViewImpl.h"
#include "art/Framework/Principal/fwd.h"
#include "cpp0x/type_traits"
#include "cpp0x/utility"

namespace art {
   namespace detail {

     template <typename T>
     struct has_donotrecordparents
     {
       static bool const value =
         std::is_base_of<art::DoNotRecordParents,T>::value;
     };

     struct RecordInParentless {
       void operator()(DataViewImpl::ProductPtrMap & used,
                       DataViewImpl::ProductPtrMap & /*ignored*/,
                       std::unique_ptr<EDProduct>&& wp,
                       BranchDescription const* desc) const {
         used.emplace(desc->branchID().id(), DataViewImpl::PMValue{std::move(wp), desc});
       }
     };  // RecordInParentless<>

     struct RecordInParentfull {
       void operator()(DataViewImpl::ProductPtrMap & used,
                       DataViewImpl::ProductPtrMap & /*ignored*/,
                       std::unique_ptr<EDProduct>&& wp,
                       BranchDescription const* desc) const {
         used.emplace(desc->branchID().id(), DataViewImpl::PMValue{std::move(wp), desc});
       }
     };  // RecordInParentfull<>

     template <typename T>
     void maybe_record_parents(DataViewImpl::ProductPtrMap & used,
                               DataViewImpl::ProductPtrMap & ignored,
                               std::unique_ptr<Wrapper<T>>&& wp,
                               BranchDescription const *desc) {
       typename std::conditional<
       has_donotrecordparents<T>::value,
         RecordInParentless,
         RecordInParentfull
         >::type parentage_recorder;
       parentage_recorder(used,
                          ignored,
                          std::move(wp),
                          desc);
   }

      // ----------------------------------------------------------------------


   }
}

#endif /* art_Framework_Principal_detail_maybe_record_parents_h */

// Local Variables:
// mode: c++
// End:
