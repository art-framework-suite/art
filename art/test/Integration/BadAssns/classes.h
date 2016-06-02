#include "canvas/Persistency/Common/Assns.h"
#include "canvas/Persistency/Common/Wrapper.h"
#include "art/test/TestObjects/AssnTestData.h"
#include "art/test/TestObjects/ToyProducts.h"

template class art::Wrapper<art::Assns<arttest::StringProduct,arttest::DummyProduct>>;
template class art::Wrapper<art::Assns<arttest::DummyProduct,arttest::StringProduct>>;

// Local Variables:
// mode: c++
// End:
