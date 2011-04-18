#ifndef art_Framework_Core_MergeHelper_h
#define art_Framework_Core_MergeHelper_h

#include "art/Framework/Core/ProducerBase.h"
#include "art/Framework/Core/Event.h"
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

  //////////////////////////////////////////////////////////////////////
  // Machinery required for retrieving and invoking stored merge
  // operations. Only required for MergeFilter.
  class MergeOpBase {
  public:

    virtual
    InputTag const &inputTag() const = 0;

    virtual
    std::string const &outputInstanceLabel() const = 0;

    virtual
    void
    mergeAndPut(Event &e,
                std::vector<EDProduct const *> const &inProducts,
                PtrRemapper const &remap) const = 0;
  };

  typedef std::vector<std::shared_ptr<MergeOpBase> > MergeOpList;
  typedef MergeOpList::const_iterator MergeOpIter;

  MergeOpIter mergeOpsBegin() const;
  MergeOpIter mergeOpsEnd() const;
  //////////////////////////////////////////////////////////////////////

private:
  template <typename PROD>
  class MergeOp : public MergeOpBase {
  public:
    template <typename F>
    MergeOp(InputTag const &inputTag,
            std::string const &outputInstanceLabel,
            F mergeFunc);

    virtual
    InputTag const &inputTag() const;

    virtual
    std::string const &outputInstanceLabel() const;

    virtual
    void
    mergeAndPut(Event &e,
                std::vector<EDProduct const *> const &inProducts,
                PtrRemapper const &remap) const;

  private:
    InputTag inputTag_;
    std::string outputInstanceLabel_;
    boost::function<void (std::vector<PROD const *> const &,
                          PROD &,
                          PtrRemapper const &remap)> mergeFunc_;
  };

  ProducerBase &producesProvider_;
  MergeOpList mergeOps_;

};

inline
art::MergeHelper::MergeHelper(ProducerBase &producesProvider)
  :
  producesProvider_(producesProvider),
  mergeOps_()
{}

inline art::ProducerBase &
art::MergeHelper::producesProvider() const {
  return producesProvider_;
}

inline
art::MergeHelper::MergeOpIter
art::MergeHelper::mergeOpsBegin() const {
  return mergeOps_.begin();
}

inline
art::MergeHelper::MergeOpIter
art::MergeHelper::mergeOpsEnd() const {
  return mergeOps_.end();
}

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

template <typename PROD>
template <typename F>
art::MergeHelper::
MergeOp<PROD>::MergeOp(InputTag const &inputTag,
                       std::string const &outputInstanceLabel,
                       F mergeFunc)
  :
  inputTag_(inputTag),
  outputInstanceLabel_(outputInstanceLabel),
  mergeFunc_(mergeFunc)
{}

template <typename PROD>
art::InputTag const &
art::MergeHelper::MergeOp<PROD>::
inputTag() const {
  return inputTag_;
}

template <typename PROD>
std::string const &
art::MergeHelper::MergeOp<PROD>::
outputInstanceLabel() const {
  return outputInstanceLabel_;
}

template <typename PROD>
void
art::MergeHelper::MergeOp<PROD>::
mergeAndPut(Event &e,
            std::vector<EDProduct const *> const &inProducts,
            PtrRemapper const &remap) const {
  std::auto_ptr<PROD> rProd(new PROD);
  std::vector<PROD const *> inConverted;
  inConverted.reserve(inProducts.size());
  try {
    for (std::vector<EDProduct const *>::const_iterator
           i = inProducts.begin(),
           endIter = inProducts.end();
         i != endIter;
         ++i) {
      inConverted.push_back(dynamic_cast<Wrapper<PROD> const &>(**i).product());
      if (!inConverted.back()) {
        throw Exception(errors::ProductNotFound)
          << "While processing products of type "
          << TypeID(*rProd).friendlyClassName()
          << " for merging: a secondary event was missing a product.\n";
      }
    }
  }
  catch (std::bad_cast const &) {
    throw Exception(errors::DataCorruption)
      << "Unable to obtain correctly-typed product from wrapper.\n";
  }
  mergeFunc_(inConverted, *rProd, remap);
  if (outputInstanceLabel_.empty()) {
    e.put(rProd);
  } else {
    e.put(rProd, outputInstanceLabel_);
  }
}
#endif /* art_Framework_Core_MergeHelper_h */

// Local Variables:
// mode: c++
// End:
