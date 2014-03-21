#include "art/Persistency/Common/Ptr.h"
#include "art/Persistency/Common/PtrVector.h"
#include "art/Persistency/Common/Wrapper.h"

#include "test/TestObjects/AssnTestData.h"
#include "test/TestObjects/MockCluster.h"
#include "test/TestObjects/ProductWithPtrs.h"
#include "test/TestObjects/ToyProducts.h"

#include "test/TestObjects/TH1Data.h"

template class art::Wrapper<arttest::TH1Data>;
template class art::Wrapper<arttest::DummyProduct>;
template class art::Wrapper<arttest::IntProduct>;
template class art::Wrapper<arttest::CompressedIntProduct>;
template class art::Wrapper<arttest::Int16_tProduct>;
template class art::Wrapper<arttest::DoubleProduct>;
template class art::Wrapper<arttest::StringProduct>;
template class art::Wrapper<arttest::Prodigal>;
template class std::vector<arttest::SimpleDerived>;
template class art::Wrapper<std::vector<arttest::SimpleDerived> >;
template class art::Ptr<arttest::SimpleDerived>;
template class art::PtrVector<arttest::SimpleDerived>;
template class art::Wrapper<art::PtrVector<arttest::SimpleDerived> >;
template class art::Wrapper<art::PtrVector<arttest::Simple> >;
template class art::Wrapper<arttest::MockClusterList>;
template class art::Wrapper<arttest::ProductWithPtrs>;
template class std::vector<art::Ptr<double> >;
template class art::PtrVector<double>;
template class art::Wrapper<std::vector<art::Ptr<double> > >;
template class art::Wrapper<art::PtrVector<double> >;
template class art::Ptr<double>;
template class art::Wrapper<arttest::VSimpleProduct>;
