#include "art/Persistency/Common/RefVector.h"
#include "art/Persistency/Common/CopyPolicy.h"
#include "art/Persistency/Common/EDProduct.h"
#include "art/Persistency/Common/HLTGlobalStatus.h"
#include "art/Persistency/Common/HLTPathStatus.h"
#include "art/Persistency/Common/OwnVector.h"
#include "art/Persistency/Provenance/ProductID.h"
#include "art/Persistency/Common/RangeMap.h"
#include "art/Persistency/Common/RefBase.h"
#include "art/Persistency/Common/RefToBaseVector.h"
#include "art/Persistency/Common/RNGsnapshot.h"
#include "art/Persistency/Common/VectorHolder.h"
#include "art/Persistency/Common/RefItem.h"
#include "art/Persistency/Common/RefVectorBase.h"
#include "art/Persistency/Common/TriggerResults.h"
#include "art/Persistency/Common/Wrapper.h"
#include "art/Persistency/Common/FillView.h"
#include "art/Persistency/Common/DataFrame.h"
#include "art/Persistency/Common/DataFrameContainer.h"
#include "art/Persistency/Common/DetSetVectorNew.h"
#include "art/Persistency/Common/ConstPtrCache.h"
#include "art/Persistency/Common/BoolCache.h"
#include "art/Persistency/Common/PtrVectorBase.h"
#include "art/Persistency/Common/ValueMap.h"

#include <vector>

namespace {
  struct dictionary {
    edm::Wrapper<edm::DataFrameContainer> dummywdfc;
    edm::Wrapper<edm::HLTPathStatus> dummyx16;
    edm::Wrapper<std::vector<edm::HLTPathStatus> > dummyx17;
    edm::Wrapper<edm::HLTGlobalStatus> dummyx18;
    edm::Wrapper<edm::TriggerResults> dummyx19;

    edm::Wrapper<edm::RefVector<std::vector<int> > > dummyx20;
    edm::Wrapper<edm::RefToBaseVector<int> > dummyx21;
    edm::reftobase::RefVectorHolderBase * dummyx21_0;
    edm::reftobase::IndirectVectorHolder<int> dummyx21_1;
    edm::reftobase::VectorHolder<int, edm::RefVector<std::vector<int> > > dummyx21_2;

    edm::RefItem<unsigned int> dummyRefItem1;
    edm::RefItem<unsigned long> dummyRefItem1_1;
    edm::RefItem<int> dummyRefItem3;
    edm::RefItem<std::pair<unsigned int, unsigned int> > dummyRefItem2;
    edm::RefItem<std::pair<unsigned int, unsigned long> > dummyRefItem2a;
    edm::RefBase<std::vector<unsigned int>::size_type> dummRefBase1;
    edm::RefBase<std::pair<unsigned int, unsigned int> > dummRefBase2;
    edm::RefBase<std::pair<unsigned int, unsigned long> > dummRefBase2a;
    edm::RefBase<int> dummyRefBase3;
    edm::RefBase<unsigned int> dummyRefBase3_1;
    edm::RefBase<unsigned long> dummyRefBase3_2;
    edm::RefVectorBase<std::vector<unsigned int>::size_type> dummyRefVectorBase;
    edm::RefVectorBase<int> dummyRefVectorBase2;
    edm::RefVectorBase<unsigned int> dummyRefVectorBase2_1;
    edm::RefVectorBase<unsigned long> dummyRefVectorBase2_2;
    edm::RefVectorBase<std::pair<unsigned int, unsigned int> > dummyRefVectorBase3;

    edm::RangeMap<int, std::vector<float>, edm::CopyPolicy<float> > dummyRangeMap1;

    std::vector<edmNew::dstvdetails::DetSetVectorTrans::Item>  dummyDSTVItemVector;

    std::pair<edm::ProductID, unsigned int> ppui1;
    edm::Wrapper<edm::ValueMap<int> > wvm1;
    edm::Wrapper<edm::ValueMap<unsigned int> > wvm2;
    edm::Wrapper<edm::ValueMap<bool> > wvm3;
    edm::Wrapper<edm::ValueMap<float> > wvm4;
    edm::Wrapper<edm::ValueMap<double> > wvm5;

    edm::RNGsnapshot                             dummyRNGsnap;
    std::vector<edm::RNGsnapshot>                dummyVectorRNGsnap;
    edm::Wrapper<std::vector<edm::RNGsnapshot> > dummyWrapperVectorRNGsnap;

  };
}
