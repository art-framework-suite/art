#include "art/Framework/Core/PtrRemapper.h"

art::RefCore
art::PtrRemapper::newRefCore_(ProductID const incomingProductID) const
{
  auto iter = prodTransMap_.find(incomingProductID);
  if (iter == cend(prodTransMap_)) {
    throw Exception(errors::LogicError)
      << "PtrRemapper: could not find old ProductID " << incomingProductID
      << " in translation table: already translated?\n";
  }
  return {iter->second, nullptr, event_->productGetter(iter->second)};
}
