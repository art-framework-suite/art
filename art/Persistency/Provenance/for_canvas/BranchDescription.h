#ifndef art_Persistency_Provenance_BranchDescription_h
#define art_Persistency_Provenance_BranchDescription_h

/*----------------------------------------------------------------------

BranchDescription: The full description of a Branch.  Equivalently,
                   the event-independent description of an EDProduct.
This description also applies to every product instance on the branch.

----------------------------------------------------------------------*/

#include "canvas/Persistency/Provenance/BranchID.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ProcessConfigurationID.h"
#include "canvas/Persistency/Provenance/ProvenanceFwd.h"
#include "canvas/Persistency/Provenance/Transient.h"
#include "canvas/Persistency/Provenance/TypeLabel.h"
#include "fhiclcpp/ParameterSetID.h"

#include <iosfwd>
#include <set>
#include <string>

// ======================================================================

namespace art {

  class BranchDescription;

  std::ostream& operator << (std::ostream& os, BranchDescription const& p);

  bool operator <  (BranchDescription const& a, BranchDescription const& b);
  bool operator == (BranchDescription const& a, BranchDescription const& b);

  bool combinable(BranchDescription const& a, BranchDescription const& b);

  namespace detail {
    class BranchDescriptionStreamer;
    void setBranchDescriptionStreamer();
  }

  // Declared below due to use of nested type:
//   std::string match(BranchDescription const& a,
//                     BranchDescription const& b,
//                     std::string const& fileName,
//                     BranchDescription::MatchMode m);

}

// ----------------------------------------------------------------------

class art::BranchDescription {
public:
  static int const invalidSplitLevel = -1;
  static int const invalidBasketSize = 0;
  static int const invalidCompression = -1;
  enum MatchMode { Strict = 0,
                   Permissive };

  BranchDescription();

  BranchDescription(TypeLabel const &tl,
                    ModuleDescription const& modDesc);

  // use compiler-generated copy c'tor, copy assignment, and d'tor

  void write(std::ostream& os) const;

  std::string const& moduleLabel() const {return moduleLabel_;}
  std::string const& processName() const {return processName_;}
  BranchID const& branchID() const {return branchID_;}
  std::string const& producedClassName() const {return producedClassName_;}
  std::string const& friendlyClassName() const {return friendlyClassName_;}
  std::string const& productInstanceName() const {return productInstanceName_;}

  bool const & produced() const {return guts().produced_;}
  bool const & transient() const {return guts().transient_;}

  int const & splitLevel() const {return guts().splitLevel_;}
  int const & basketSize() const {return guts().basketSize_;}
  int const & compression() const {return guts().compression_;}
  std::set<fhicl::ParameterSetID> const& psetIDs() const {return psetIDs_;}
  std::string const &branchName() const {return guts().branchName_;}
  BranchType const &branchType() const {return branchType_;}
  std::string const &wrappedName() const {return guts().wrappedName_;}

  void merge(BranchDescription const& other);
  void swap(BranchDescription &other);

  friend bool combinable(BranchDescription const &, BranchDescription const &);
  friend std::string match(BranchDescription const &,
                           BranchDescription const &,
                           std::string const &,
                           BranchDescription::MatchMode);
  friend bool operator<(BranchDescription const &, BranchDescription const &);
  friend bool operator==(BranchDescription const &, BranchDescription const &);

  struct Transients {
    Transients();

    // The branch name, which is currently derivable fron the other attributes.
    std::string branchName_;

    // The wrapped class name, which is currently derivable fron the other attributes.
    std::string wrappedName_;

    // Was this branch produced in this process
    // rather than in a previous process
    bool produced_;

    // Is the class of the branch marked as transient
    // in the data dictionary
    bool transient_;

    // The split level of the branch, as marked
    // in the data dictionary.
    int splitLevel_;

    // The basket size of the branch, as marked
    // in the data dictionary.
    int basketSize_;

    // The compression of the branch, as marked
    // in the data dictionary.
    int compression_;
  };

private:
  friend class detail::BranchDescriptionStreamer;

  bool transientsFluffed_() const {return !guts().branchName_.empty(); }
  void initBranchID_();
  void fluffTransients_() const;

  fhicl::ParameterSetID const& psetID() const;
  bool isPsetIDUnique() const {return psetIDs().size() == 1;}
  std::set<ProcessConfigurationID> const& processConfigurationIDs() const {return processConfigurationIDs_;}

  Transients &guts() {return transients_.get(); }
  Transients const &guts() const {return transients_.get(); }

  void throwIfInvalid_() const;

  // What tree is the branch in?
  BranchType branchType_;

  // A human friendly string that uniquely identifies the EDProducer
  // and becomes part of the identity of a product that it produces
  std::string moduleLabel_;

  // the physical process that this program was part of (e.g. production)
  std::string processName_;

  // An ID uniquely identifying the branch
  BranchID branchID_;

  // the full name of the type of product this is
  std::string producedClassName_;

  // a readable name of the type of product this is
  std::string friendlyClassName_;

  // a user-supplied name to distinguish multiple products of the same type
  // that are produced by the same producer
  std::string productInstanceName_;

  // ID's of parameter set of the creators of products
  // on this branch
  std::set<fhicl::ParameterSetID> psetIDs_;

  // ID's of process configurations for products
  // on this branch
  std::set<ProcessConfigurationID> processConfigurationIDs_;

  mutable Transient<Transients> transients_;
};  // BranchDescription

namespace art {
  std::string match(BranchDescription const& a,
                    BranchDescription const& b,
                    std::string const& fileName,
                    BranchDescription::MatchMode m);
}

#endif /* art_Persistency_Provenance_BranchDescription_h */

// Local Variables:
// mode: c++
// End:
