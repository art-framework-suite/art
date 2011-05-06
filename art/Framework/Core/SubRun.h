#ifndef art_Framework_Core_SubRun_h
#define art_Framework_Core_SubRun_h

// ======================================================================
//
// SubRun: This is the primary interface for accessing per subRun
// EDProducts and inserting new derived per subRun EDProducts.
//
// For its usage, see "FWCore/Framework/interface/DataViewImpl.h"
//
// ======================================================================

#include "art/Framework/Core/DataViewImpl.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/detail/maybe_call_post_insert.h"
#include "art/Persistency/Provenance/RunID.h"
#include "art/Persistency/Provenance/SubRunAuxiliary.h"
#include "art/Persistency/Provenance/SubRunID.h"
#include "cpp0x/memory"
#include <utility>

namespace art {

  class SubRun : private DataViewImpl
  {
  public:
    SubRun(SubRunPrincipal& srp, const ModuleDescription& md);
    ~SubRun() {}

    typedef DataViewImpl Base;
    // AUX functions.
    SubRunNumber_t subRun() const {return aux_.subRun();}

    RunNumber_t run() const {return aux_.run();}

    SubRunID id() const {
      return aux_.id();
    }

    Timestamp const& beginTime() const {return aux_.beginTime();}
    Timestamp const& endTime() const {return aux_.endTime();}

    using Base::get;
    using Base::getByLabel;
    using Base::getMany;
    using Base::getManyByType;
    using Base::me;
    using Base::processHistory;

    Run const&
    getRun() const;

    ///Put a new product.
    template <typename PROD>
    void
    put(std::auto_ptr<PROD> product) {put<PROD>(product, std::string());}

    ///Put a new product with a 'product instance name'
    template <typename PROD>
    void
    put(std::auto_ptr<PROD> product, std::string const& productInstanceName);

    Provenance
    getProvenance(BranchID const& theID) const;

    void
    getAllProvenance(std::vector<Provenance const*> &provenances) const;

  private:
    SubRunPrincipal const&
    subRunPrincipal() const;

    SubRunPrincipal &
    subRunPrincipal();

    // commit_() is called to complete the transaction represented by
    // this DataViewImpl. The friendships required are gross, but any
    // alternative is not great either.  Putting it into the
    // public interface is asking for trouble
    friend class InputSource;
    friend class DecrepitRelicInputSourceImplementation;
    friend class EDFilter;
    friend class EDProducer;

    void commit_();

    SubRunAuxiliary const& aux_;
    std::shared_ptr<Run const> const run_;
  };

  template <typename PROD>
  void
  SubRun::put(std::auto_ptr<PROD> product, std::string const& productInstanceName)
  {
    if (product.get() == 0) {                // null pointer is illegal
      TypeID typeID(typeid(PROD));
      throw art::Exception(art::errors::NullPointerError)
        << "SubRun::put: A null auto_ptr was passed to 'put'.\n"
        << "The pointer is of type " << typeID << ".\n"
        << "The specified productInstanceName was '" << productInstanceName << "'.\n";
    }

    detail::maybe_call_post_insert(product.get());

    ConstBranchDescription const& desc =
      getBranchDescription(TypeID(*product), productInstanceName);

    Wrapper<PROD> *wp(new Wrapper<PROD>(product));

    putProducts().push_back(std::make_pair(wp, &desc));

    // product.release(); // The object has been copied into the Wrapper.
    // The old copy must be deleted, so we cannot release ownership.
  }

}  // art

// ======================================================================

#endif /* art_Framework_Core_SubRun_h */

// Local Variables:
// mode: c++
// End:
