#ifndef art_Framework_Core_MergeHelper_h
#define art_Framework_Core_MergeHelper_h

#include "art/Framework/Core/ProducerBase.h"

#include "boost/function.hpp"
#include "boost/any.hpp"

#include <string>
#include <vector>

namespace art {
  class MergeHelper;
}

class art::MergeHelper {
public:
  explicit MergeHelper(ProducerBase & producesProvider);
  ProducerBase &producesProvider() const;

  template <typename InType, typename OutType, typename FUNC>
  void declareMergeOp(InputTag const &inputTag,
                      FUNC const &mergeFunc);

  template <typename InType, typename OutType, typename FUNC>
  void declareMergeOp(InputTag const &inputTag,
                      std::string const &outputInstanceName,
                      FUNC const &mergeFunc);
private:
  struct MergeOperationInfo {
    MergeOperationInfo(TypeID const &inputType,
                       InputTag const &inputTag,
                       std::string const &outputInstanceName,
                       boost::any const &mergeFunc);
    TypeID inputType;
    InputTag inputTag;
    std::string outputInstanceName;
    boost::any mergeFunc;
  };

  typedef std::vector<MergeOperationInfo> MergeOpList;

  ProducerBase &producesProvider_;
  MergOpList mergeOps_;

};

inline
art::MergeHelper::MergeHelper(ProducerBase &producesProvider)
  :
  producesProvider_(producesProvider),
  mergeOps_()
{}

inline ProducerBase &
art::MergeHelper::producesProvider() {
  return producesProvider_;
}

template <typename InType, typename OutType, typename FUNC>
void
declareMergeOp(InputTag const &inputTag,
               FUNC const &mergeFunc) {
  producesProvider().produces<OutType>();
  mergeOps_.push_back(MergeOperationInfo(TypeID(typeid(inType)),
                                         inputTag,
                                         std::string(),
                                         boost::function<void (std::vector<InType> const &,
                                                               OutType &)>(mergeFunc)));
}

template <typename InType, typename OutType, typename FUNC>
void
declareMergeOp(InputTag const &inputTag,
               std::string const &outputInstanceName,
               FUNC const &mergeFunc) {
  producesProvider().produces<OutType>(outputInstanceName);
  mergeOps_.push_back(MergeOperationInfo(TypeID(typeid(inType)),
                                         inputTag,
                                         outputInstanceName,
                                         boost::function<void (std::vector<InType> const &,
                                                               OutType &)>(mergeFunc)));
}

art::MergeHelper::
MergeOperationInfo::MergeOperationInfo(TypeID const &inputType,
                                       InputTag const &inputTag,
                                       std::string const &outputInstanceName,
                                       boost::any const &mergeFunc)
  :
  inputType(inputType),
  inputTag(inputTag),
  outputInstanceName(outputInstanceName),
  mergeFunc(mergeFunc)
{}

#endif /* art_Framework_Core_MergeHelper_h */

// Local Variables:
// mode: c++
// End:
