#include "art/Framework/Core/ProductMetaData.h"
#include "art/Utilities/Exception.h"

cet::exempt_ptr<art::MasterProductRegistry const> art::ProductMetaData::mpr_;

art::MasterProductRegistry const &
art::ProductMetaData::get() {
  if (mpr_) {
    return *mpr_;
  } else {
    throw Exception(errors::LogicError)
      << "ProductMetaData::get(): attempt to access MasterProductRegistry before\n"
      << "it has been made available. Note that access is *not* available from module"
      << "\nconstructors.\n";
  }
}
