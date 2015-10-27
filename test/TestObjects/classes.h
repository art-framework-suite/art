#include "art/Persistency/Common/Assns.h"
#include "art/Persistency/Common/Ptr.h"
#include "art/Persistency/Common/PtrVector.h"
#include "art/Persistency/Common/Wrapper.h"

#include "test/TestObjects/AssnTestData.h"
#include "test/TestObjects/MockCluster.h"
#include "test/TestObjects/ProductWithPtrs.h"
#include "test/TestObjects/ToyProducts.h"

#include "test/TestObjects/TH1Data.h"

#include <vector>

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
template class std::vector<art::Ptr<arttest::SimpleDerived> >;
template class art::PtrVector<arttest::SimpleDerived>;
template class art::Wrapper<art::PtrVector<arttest::SimpleDerived> >;
template class std::vector<art::Ptr<arttest::Simple> >;
template class art::Wrapper<art::PtrVector<arttest::Simple> >;
template class art::Wrapper<arttest::MockClusterList>;
template class art::Wrapper<arttest::ProductWithPtrs>;
template class std::vector<art::Ptr<double> >;
template class art::PtrVector<double>;
template class art::Wrapper<std::vector<art::Ptr<double> > >;
template class art::Wrapper<art::PtrVector<double> >;
template class art::Ptr<double>;
template class art::Wrapper<arttest::VSimpleProduct>;
template class std::vector<arttest::Hit>;
template class std::vector<arttest::Track>;
template class art::Wrapper<std::vector<arttest::Hit> >;
template class art::Wrapper<std::vector<arttest::Track> >;
template class std::pair<art::Ptr<arttest::Hit>, art::Ptr<arttest::Track> >;
template class std::vector<std::pair<art::Ptr<arttest::Hit>, art::Ptr<arttest::Track> > >;
template class art::Assns<arttest::Hit, arttest::Track>;
template class art::Wrapper<art::Assns<arttest::Hit, arttest::Track> >;
template class art::Assns<arttest::Track, arttest::Hit>;
template class art::Wrapper<art::Assns<arttest::Track, arttest::Hit> >;
