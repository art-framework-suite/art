#include "art/Framework/Core/InputSourceMacros.h"
#include "art/Framework/IO/Input/PoolSource.h"
#include "art/Framework/IO/Sources/VectorInputSourceMacros.h"

using art::PoolSource;
DEFINE_FWK_INPUT_SOURCE(PoolSource);
DEFINE_FWK_VECTOR_INPUT_SOURCE(PoolSource);
