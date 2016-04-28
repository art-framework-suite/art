#ifndef art_Framework_Principal_Run_h
#define art_Framework_Principal_Run_h

// ======================================================================
//
// Run - This is the primary interface for accessing per run EDProducts
//       and inserting new derived products.
//
// For its usage, see "art/Framework/Principal/DataViewImpl.h"
// ======================================================================

#include "art/Framework/Principal/DataViewImpl.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/ProductTokens.h"
#include "canvas/Persistency/Provenance/RunAuxiliary.h"
#include "canvas/Persistency/Provenance/RunID.h"
#include "canvas/Utilities/TypeID.h"

#include <memory>
#include <utility>

class art::Run : private art::DataViewImpl {
public:

  Run(RunPrincipal& rp, const ModuleDescription& md);
  ~Run() = default;

  using Base = DataViewImpl;

  // AUX functions.
  RunID const& id() const {return aux_.id();}
  RunNumber_t run() const {return aux_.run();}
  Timestamp const& beginTime() const {return aux_.beginTime();}
  Timestamp const& endTime() const {return aux_.endTime();}

  RangeSet fullRunRangeSet() const { return RangeSet::forRun(id()); }
  RangeSet const& seenRangeSet() const { return seenRanges_; }

  using Base::get;
  using Base::getByLabel;
  using Base::getMany;
  using Base::getManyByType;
  using Base::removeCachedProduct;
  using Base::me;
  using Base::processHistory;

  template <typename PROD>
  art::ValidHandle<PROD> getValidHandle(InputTag const& tag) const
  {
    art::Handle<PROD> h;
    getByLabel(tag, h);
    return art::ValidHandle<PROD>(&(*h), *h.provenance());
  }

  ///Put a new product.
  template <typename PROD>
  void
  put(std::unique_ptr<PROD> && product,
      detail::RangedFragmentToken<Level::Run> const& token)
  {
    put<PROD>(std::move(product), std::string(), token);
  }

  ///Put a new product.
  template <typename PROD>
  void
  put(std::unique_ptr<PROD> && product,
      std::string const& productInstanceName,
      detail::RangedFragmentToken<Level::Run> const& token)
  {
    static_assert(detail::CanBeAggregated<PROD>::value,
                  "\n\n"
                  "art error: A Run product put with the token 'RunFragment'\n"
                  "           must be able to be aggregated. Please add the appropriate\n"
                  "              void aggregate(T const&)\n"
                  "           function to your class, or contact artists@fnal.gov.\n");
    put_<PROD>(std::move(product), productInstanceName, token.rs);
  }


  template <typename PROD>
  void
  put(std::unique_ptr<PROD> && product, detail::FullToken<Level::Run> const token)
  {
    put<PROD>(std::move(product), std::string(), token);
  }

  template <typename PROD>
  void
  put(std::unique_ptr<PROD> && product, detail::FragmentToken<Level::Run> const token)
  {
    put<PROD>(std::move(product), std::string(), token);
  }

  template <typename PROD>
  void
  put(std::unique_ptr<PROD> && product,
      std::string const& productInstanceName,
      detail::FullToken<Level::Run>)
  {
    put_<PROD>(std::move(product), productInstanceName, fullRunRangeSet());
  }

  template <typename PROD>
  void
  put(std::unique_ptr<PROD> && product,
      std::string const& productInstanceName,
      detail::FragmentToken<Level::Run>)
  {
    static_assert(detail::CanBeAggregated<PROD>::value,
                  "\n\n"
                  "art error: A Run product put with the token 'RunFragment'\n"
                  "           must be able to be aggregated. Please add the appropriate\n"
                  "              void aggregate(T const&)\n"
                  "           function to your class, or contact artists@fnal.gov.\n");
    put_<PROD>(std::move(product), productInstanceName, seenRangeSet());
  }

  // Return true if this Run has been subjected to a process with
  // the given processName, and false otherwise.
  // If true is returned, then ps is filled with the ParameterSets
  // (possibly more than one) used to configure the identified
  // process(es). Equivalent ParameterSets are compressed out of the
  // result.
  bool
  getProcessParameterSet(std::string const& processName,
                         std::vector<fhicl::ParameterSet>& ps) const;

private:
  // commit_() is called to complete the transaction represented by
  // this DataViewImpl. The friendships required are gross, but any
  // alternative is not great either.  Putting it into the
  // public interface is asking for trouble
  friend class InputSource;
  friend class DecrepitRelicInputSourceImplementation;
  friend class EDFilter;
  friend class EDProducer;

  void commit_();

  ///Put a new product with a 'product instance name' and a 'range set'
  template <typename PROD>
  void
  put_(std::unique_ptr<PROD> && product,
       std::string const& productInstanceName,
       RangeSet const& rs);

  RunAuxiliary const& aux_;
  RangeSet seenRanges_;
};

template <typename PROD>
void
art::Run::put_(std::unique_ptr<PROD> && product,
               std::string const& productInstanceName,
               RangeSet const& rs)
{
  if (product.get() == nullptr) {
    throw art::Exception(art::errors::NullPointerError)
      << "Run::put: A null unique_ptr was passed to 'put'.\n"
      << "The pointer is of type " << TypeID(typeid(PROD)) << ".\n"
      << "The specified productInstanceName was '" << productInstanceName << "'.\n";
  }

  auto const& bd = getBranchDescription(TypeID(*product), productInstanceName);
  auto        wp = std::make_unique<Wrapper<PROD>>(std::move(product));

  auto result = putProducts().emplace(bd.branchID(), PMValue{std::move(wp), bd, rs});
  if (!result.second) {
    throw art::Exception(art::errors::InsertFailure)
      << "Run::put: Attempt to put multiple products with the\n"
      << "          following description onto the Run.\n"
      << "          Products must be unique per Run.\n"
      << "=================================\n"
      << bd
      << "=================================\n";
  }

}

#endif /* art_Framework_Principal_Run_h */

// Local Variables:
// mode: c++
// End:
