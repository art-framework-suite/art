#ifndef art_Framework_Core_MergeHelper_h
#define art_Framework_Core_MergeHelper_h

#include "art/Framework/Core/ProducerBase.h"
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
}

class art::MergeHelper {
public:
  explicit MergeHelper(ProducerBase & producesProvider);
  ProducerBase &producesProvider() const;

  template <typename PROD, typename FUNC>
  void declareMergeOp(InputTag const &inputTag,
                      FUNC mergeFunc);

  template <typename PROD, typename FUNC>
  void declareMergeOp(InputTag const &inputTag,
                      std::string const &outputInstanceName,
                      FUNC mergeFunc);

  template <typename PROD, typename T>
  void declareMergeOp(InputTag const &inputTag,
                      void (T::*mergefunc) (std::vector<PROD const *> const &,
                                            PROD &),
                      T *t);

  template <typename PROD, typename T>
  void declareMergeOp(InputTag const &inputTag,
                      std::string const &outputInstanceName,
                      void (T::*mergefunc) (std::vector<PROD const *> const &,
                                            PROD &),
                      T *t);

private:
  class MergeOpBase {
  public:
    virtual
    std::auto_ptr<EDProduct>
    operator()(std::vector<EDProduct const *> const &inProducts) = 0;
  };

  template <typename PROD>
  class MergeOp : public MergeOpBase {
  public:
    template <typename F>
    MergeOp(InputTag const &inputTag,
            std::string const &outputInstanceName,
            F mergeFunc);

    virtual
    std::auto_ptr<EDProduct>
    operator()(std::vector<EDProduct const *> const &inProducts);
    
  private:
    InputTag inputTag_;
    std::string outputInstanceName_;
    boost::function<void (std::vector<PROD const *> const &, PROD &)> mergeFunc_;
  };

  typedef std::vector<std::shared_ptr<MergeOpBase> > MergeOpList;

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
               std::string const &outputInstanceName,
               FUNC mergeFunc) {
  producesProvider().produces<PROD>(outputInstanceName);
  mergeOps_.push_back(std::shared_ptr<MergeOpBase>
                      (new MergeOp<PROD>
                       (inputTag,
                        outputInstanceName,
                        mergeFunc)));
}

template <typename PROD, typename T>
void
art::MergeHelper::
declareMergeOp(InputTag const &inputTag,
               void (T::*mergeFunc) (std::vector<PROD const *> const &,
                                     PROD &),
               T *t) {
  producesProvider().produces<PROD>();
  mergeOps_.push_back(std::shared_ptr<MergeOpBase>
                      (new MergeOp<PROD>
                       (inputTag,
                        std::string(),
                        std::bind(mergeFunc, t, _1, _2))));
}

template <typename PROD, typename T>
void
art::MergeHelper::
declareMergeOp(InputTag const &inputTag,
               std::string const &outputInstanceName,
               void (T::*mergeFunc) (std::vector<PROD const *> const &,
                                     PROD &),
               T *t) {
  producesProvider().produces<PROD>(outputInstanceName);
  mergeOps_.push_back(std::shared_ptr<MergeOpBase>
                      (new MergeOp<PROD>
                       (inputTag,
                        outputInstanceName,
                        std::bind(mergeFunc, t, _1, _2))));
}

template <typename PROD>
template <typename F>
art::MergeHelper::
MergeOp<PROD>::MergeOp(InputTag const &inputTag,
                       std::string const &outputInstanceName,
                       F mergeFunc)
  :
  inputTag_(inputTag),
  outputInstanceName_(outputInstanceName),
  mergeFunc_(mergeFunc)
{}

template <typename PROD>
std::auto_ptr<art::EDProduct>
art::MergeHelper::MergeOp<PROD>::
operator()(std::vector<EDProduct const *> const &inProducts) {
  std::auto_ptr<PROD> rProd(new PROD);
  std::vector<PROD const *> inConverted;
  inConverted.reserve(inProducts.size());
  try {
    for (std::vector<EDProduct const *>::const_iterator
           i = inProducts.begin(),
           e = inProducts.end();
         i != e;
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
  catch (std::bad_cast const &e) {
    throw Exception(errors::DataCorruption)
      << "Unable to obtain correctly-typed product from wrapper.\n";
  }
  mergeFunc_(inConverted, *rProd);
  return std::auto_ptr<EDProduct>(new Wrapper<PROD>(rProd));
}
#endif /* art_Framework_Core_MergeHelper_h */

// Local Variables:
// mode: c++
// End:
