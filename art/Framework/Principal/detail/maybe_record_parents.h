#ifndef art_Framework_Principal_detail_maybe_record_parents_h
#define art_Framework_Principal_detail_maybe_record_parents_h

#include "art/Framework/Principal/DataViewImpl.h"
#include "art/Framework/Principal/fwd.h"

#include <type_traits>
#include <utility>

namespace art {
   namespace detail {

     template <typename T>
     struct has_donotrecordparents : std::is_base_of<art::DoNotRecordParents, T>
     {};

     struct RecordInParentless {
       auto operator()(DataViewImpl::TypeLabelMap& used,
                       DataViewImpl::TypeLabelMap& /*ignored*/,
                       TypeLabel&& typeLabel,
                       std::unique_ptr<EDProduct>&& wp,
                       BranchDescription const& bd) const {
         return used.emplace(std::move(typeLabel), DataViewImpl::PMValue{std::move(wp), bd, RangeSet::invalid()});
       }
     };

     using RecordInParentfull = RecordInParentless; // Not currently different than above.

     template <typename T>
     auto maybe_record_parents(DataViewImpl::TypeLabelMap& used,
                               DataViewImpl::TypeLabelMap& ignored,
                               TypeLabel&& typeLabel,
                               std::unique_ptr<Wrapper<T>>&& wp,
                               BranchDescription const& bd) {
       std::conditional_t<has_donotrecordparents<T>::value,
                          RecordInParentless,
                          RecordInParentfull> parentage_recorder;

       return parentage_recorder(used,
                                 ignored,
                                 std::move(typeLabel),
                                 std::move(wp),
                                 bd);
     }

   }
}

#endif /* art_Framework_Principal_detail_maybe_record_parents_h */

// Local Variables:
// mode: c++
// End:
