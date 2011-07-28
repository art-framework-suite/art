#ifndef art_Persistency_Provenance_ProductMetaData_h
#define art_Persistency_Provenance_ProductMetaData_h
////////////////////////////////////////////////////////////////////////
// ProductMetaData
//
// Singleton-like facade to provide const access to the
// MasterProductRegistry.
////////////////////////////////////////////////////////////////////////

#include "art/Persistency/Provenance/ProductList.h"

// We only need to include this because of a nested typedef.
#include "art/Persistency/Provenance/MasterProductRegistry.h"

#include "art/Persistency/Provenance/BranchType.h"

#include "cetlib/exempt_ptr.h"
#include "boost/noncopyable.hpp"
#include <ostream>
#include <vector>

class MPRGlobalTestFixture; // Used for testing

namespace art
{
  class BranchDescription;
  class MasterProductRegistry;
  class ProductMetaData;
  class Schedule;

  std::ostream& operator<< (std::ostream&, ProductMetaData const&);
}

class art::ProductMetaData : boost::noncopyable
{
 public:

   typedef MasterProductRegistry::ProcessLookup ProcessLookup;
   typedef MasterProductRegistry::TypeLookup    TypeLookup;

   // Give access to the only instance; throws if it is not yet made.
   static ProductMetaData const& instance();

   // Accessors: this is the facade presented by ProductMetaData
   // for the MasterProductRegistry.
   ProductList const& productList() const;

   // This should be changed to take a vector by reference, and to fill
   // that vector.
   std::vector<BranchDescription const*> allBranchDescriptions() const;

   // Print all the BranchDescriptions we contain.
   void printBranchDescriptions(std::ostream&) const;

   // Obtain lookup map to find a group by type of product.
   TypeLookup const& productLookup() const;

   // Obtain lookup map to find a group by type of element in a product
   // which is a collection.
   TypeLookup const& elementLookup() const;

   // Return true if any product is produced in this process for
   // the given branch type.
   bool productProduced(BranchType which) const;

  friend class Schedule;
  friend class ::MPRGlobalTestFixture; // Used for testing.

private:
  // Only the create_instance() member will create an instance.
  ProductMetaData(MasterProductRegistry const&);

  // Only friends are permitted to create the instance.
  static void create_instance(MasterProductRegistry const& );

  // Member data.
  static ProductMetaData const* me;
  cet::exempt_ptr<MasterProductRegistry const> mpr_;
};

inline
std::ostream&
art::operator<< (std::ostream& os, art::ProductMetaData const& pmd)
{
  pmd.printBranchDescriptions(os);
  return os;
}

#endif /* art_Persistency_Provenance_ProductMetaData_h */

// Local Variables:
// mode: c++
// End:
