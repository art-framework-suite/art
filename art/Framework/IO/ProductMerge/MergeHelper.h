#ifndef art_Framework_IO_ProductMerge_MergeHelper_h
#define art_Framework_IO_ProductMerge_MergeHelper_h

#include "art/Framework/Core/ProducerBase.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/IO/ProductMerge/MergeOp.h"
#include "art/Persistency/Common/EDProduct.h"
#include "art/Utilities/Exception.h"
#include "art/Utilities/TypeID.h"
#include "cpp0x/functional"
#include "cpp0x/memory"

#include "boost/function.hpp"

#include <string>
#include <vector>

namespace art {
  class MergeHelper;
  class PtrRemapper;
}

class art::MergeHelper {
public:
  explicit MergeHelper(ProducerBase & producesProvider);

  // Returns the object upon which produces() can be called: may replace
  // by actually implementing forwarding templates in the helper.
  ProducerBase &producesProvider() const;

  //////////////////////////////////////////////////////////////////////
  // declareMergeOp templates.
  //
  // These function templates may be used by writers of product-merging
  // modules to declare a product mix operation. Such an operation may
  // be specified by providing:
  //
  // 1, an InputTag specifying which secondary products should be
  // merged;
  //
  // 2. an optional instance label for the merged product; and
  //
  // 3. a merge function. This merge function should have the following
  //    signature:
  //
  //       void mergefunc (std::vector<PROD const *> const &,
  //                       PROD &,
  //                       PtrRemapper const &remap),
  //
  //    This function may be a free function, a function object (in
  //    which case operator() should be the member function with the
  //    correct signature) or a member function of any class provided it
  //    has the same return type and arguments. If it is a member
  //    function it may be provided bound to the object upon which it is
  //    to be called by the user (in which case it is treated as a free
  //    function by the registration method) or by specifying the member
  //    function followed by the object to which it should be bound (in
  //    which case the bind will be done for the user). In this latter
  //    case the template argument specifying the product type need
  //    *not* be specified usually as it may be deduced from the
  //    signature of the provided function. If one specifies an overload
  //    set however (eg in the case where a class has several merge()
  //    member functions, each one with a different merge function) then
  //    the template argument must be specified in order to constrain
  //    the overload set to a single function. For free functions,
  //    function objects and pre-bound member functions the product type
  //    template argument must be specified as it cannot be deduced,
  //////////////////////////////////////////////////////////////////////

  // Provide an InputTag and free function or function object.
  template <typename PROD, typename FUNC>
  void declareMergeOp(InputTag const &inputTag,
                      FUNC mergeFunc);

  //  Provide an InputTag, instance label and free function or function
  //  object,
  template <typename PROD, typename FUNC>
  void declareMergeOp(InputTag const &inputTag,
                      std::string const &outputInstanceLabel,
                      FUNC mergeFunc);

  // Provide an InputTag, member function with the correct signature and
  // object to which the member function should be bound.
  template <typename PROD, typename T>
  void declareMergeOp(InputTag const &inputTag,
                      void (T::*mergefunc) (std::vector<PROD const *> const &,
                                            PROD &,
                                            PtrRemapper const &remap),
                      T *t);

  // Provide an InputTag, instance label, member function with the
  // correct signature and object to which the member function should be
  // bound.
  template <typename PROD, typename T>
  void declareMergeOp(InputTag const &inputTag,
                      std::string const &outputInstanceLabel,
                      void (T::*mergefunc) (std::vector<PROD const *> const &,
                                            PROD &,
                                            PtrRemapper const &remap),
                      T *t);

  //////////////////////////////////////////////////////////////////////
  // Merge module writers should not need anything below this point.
  //////////////////////////////////////////////////////////////////////
  void mergeAndPut(size_t nSecondaries, Event &e, PtrRemapper const &remap);

private:
  typedef std::vector<std::shared_ptr<MergeOpBase> > MergeOpList;
  typedef MergeOpList::const_iterator MergeOpIter;

  ProducerBase &producesProvider_;
  MergeOpList mergeOps_;

};

template <typename PROD, typename FUNC>
void
art::MergeHelper::
declareMergeOp(InputTag const &inputTag,
               FUNC mergeFunc) {
  producesProvider().produces<PROD>();
  mergeOps_.push_back(std::shared_ptr<MergeOpBase>
                      (new MergeOp<PROD>
                       (inputTag,
                        std::string(),
                        mergeFunc)));
}

template <typename PROD, typename FUNC>
void
art::MergeHelper::
declareMergeOp(InputTag const &inputTag,
               std::string const &outputInstanceLabel,
               FUNC mergeFunc) {
  producesProvider().produces<PROD>(outputInstanceLabel);
  mergeOps_.push_back(std::shared_ptr<MergeOpBase>
                      (new MergeOp<PROD>
                       (inputTag,
                        outputInstanceLabel,
                        mergeFunc)));
}

template <typename PROD, typename T>
void
art::MergeHelper::
declareMergeOp(InputTag const &inputTag,
               void (T::*mergeFunc) (std::vector<PROD const *> const &,
                                     PROD &,
                                     PtrRemapper const &remap),
               T *t) {
  producesProvider().produces<PROD>();
  mergeOps_.push_back(std::shared_ptr<MergeOpBase>
                      (new MergeOp<PROD>
                       (inputTag,
                        std::string(),
                        std::bind(mergeFunc, t, _1, _2, _3))));
}

template <typename PROD, typename T>
void
art::MergeHelper::
declareMergeOp(InputTag const &inputTag,
               std::string const &outputInstanceLabel,
               void (T::*mergeFunc) (std::vector<PROD const *> const &,
                                     PROD &,
                                     PtrRemapper const &remap),
               T *t) {
  producesProvider().produces<PROD>(outputInstanceLabel);
  mergeOps_.push_back(std::shared_ptr<MergeOpBase>
                      (new MergeOp<PROD>
                       (inputTag,
                        outputInstanceLabel,
                        std::bind(mergeFunc, t, _1, _2, _3))));
}

#endif /* art_Framework_IO_ProductMerge_MergeHelper_h */

// Local Variables:
// mode: c++
// End:
