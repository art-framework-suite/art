// vim: set sw=2 expandtab :
#define BOOST_TEST_MODULE (Event_t)

// =====================================================================
// Event_t tests the art::Event transactional object.  It does this by
// creating products that originate from a "source", and by producing
// one product in the "current" process.
//
// A large amount of boilerplate is required to construct an
// art::Event object.  Required items include:
//
//   - A well-formed ProcessHistory
//   - Properly-constructed product-lookup tables
//
// =====================================================================

#include "boost/test/unit_test.hpp"

#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/Selector.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Persistency/Provenance/ModuleContext.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/ModuleType.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "art/Version/GetReleaseVersion.h"
#include "art/test/TestObjects/ToyProducts.h"
#include "canvas/Persistency/Common/WrappedTypeID.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/EventAuxiliary.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/ProcessHistory.h"
#include "canvas/Persistency/Provenance/ProductTables.h"
#include "canvas/Persistency/Provenance/RunAuxiliary.h"
#include "canvas/Persistency/Provenance/SubRunAuxiliary.h"
#include "canvas/Persistency/Provenance/Timestamp.h"
#include "canvas/Persistency/Provenance/TypeLabel.h"
#include "canvas/Utilities/InputTag.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/ParameterSet.h"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <typeinfo>
#include <utility>
#include <vector>

using namespace art;
using namespace std;
using namespace string_literals;

using fhicl::ParameterSet;
using product_t = arttest::IntProduct;

namespace {

  art::EventID
  make_id()
  {
    return art::EventID{2112, 47, 25};
  }

  constexpr art::Timestamp timestamp{1};

  std::string
  module_class_name()
  {
    return "IntProducer";
  }

  auto
  product_with_value(int const v)
  {
    return std::make_unique<arttest::IntProduct>(v);
  }

  ModuleLabelSelector const modMultiSelector{"modMulti"};
} // namespace

namespace art {

  std::ostream&
  boost_test_print_type(std::ostream& os, Timestamp const timestamp)
  {
    return os << timestamp.value();
  }

  // This gross hack allows us to test the event
  namespace detail {
    class Producer {
    public:
      static void
      commit(Event& e)
      {
        e.commitProducts();
      }
    };
  }

} // namespace art

class ProductTablesFixture {
public:
  ProductTablesFixture();

  ProcessHistory processHistory() const;
  ModuleDescription currentModuleDescription_{};
  // The moduleConfigurations_ data member is a map that with the
  // following semantics:
  //   process_name <-> (module_name <-> module_configuration)
  std::map<std::string, std::map<std::string, ParameterSet>>
    moduleConfigurations_{};
  std::map<std::string, ModuleDescription> moduleDescriptions_{};

  // The descriptions_ data member is used only to create the product
  // lookup tables.
  ProductDescriptions descriptions_{};
  ProductTables producedProducts_{ProductTables::invalid()};
  ProductTable presentProducts_{};
  std::map<std::string, TypeLabelLookup_t> earlyLookup_{};
  TypeLabelLookup_t currentLookup_{};

private:
  // The 'Present' non-type parameter refers to if the product is
  // present from the source (true) or if the product is produced
  // (false).
  template <typename T, bool Present>
  ModuleDescription const& registerProduct(
    std::string const& tag,
    std::string const& moduleLabel,
    std::string const& processName,
    std::string const& productInstanceName = {});
};

ProductTablesFixture::ProductTablesFixture()
{
  constexpr bool presentFromSource{true};
  constexpr bool produced{false};

  // Register products for "EARLY" process
  registerProduct<product_t, presentFromSource>(
    "nolabel_tag", "modOne", "EARLY");
  registerProduct<product_t, presentFromSource>(
    "int1_tag", "modMulti", "EARLY", "int1");
  registerProduct<product_t, presentFromSource>(
    "int2_tag", "modMulti", "EARLY", "int2");
  registerProduct<product_t, presentFromSource>(
    "int3_tag", "modMulti", "EARLY");

  // Register products for "LATE" process
  registerProduct<product_t, presentFromSource>(
    "int1_tag_late", "modMulti", "LATE", "int1");

  // Fill the lookups for "source-like" products
  {
    presentProducts_ = ProductTables{descriptions_}.get(InEvent);
    descriptions_.clear();
  }

  // Register single IntProduct for the "CURRENT" process
  currentModuleDescription_ = registerProduct<product_t, produced>(
    "current_tag", "modMulti", "CURRENT", "int1");

  // Create the lookup that we will use for the current-process module
  {
    producedProducts_ = ProductTables{descriptions_};
    descriptions_.clear();
  }
}

ProcessHistory
ProductTablesFixture::processHistory() const
{
  // Construct process history for event.  This takes several lines of
  // code but other than the process names none of it is used or
  // interesting.
  ProcessHistory processHistory;
  for (auto const& [process_name, module_configs] : moduleConfigurations_) {
    // Skip current process since it is not yet part of the history.
    if (process_name == "CURRENT") {
      continue;
    }

    // Construct the ParameterSet for each process
    ParameterSet processParameterSet;
    processParameterSet.put("process_name", process_name);
    for (auto const& [key, value] : module_configs) {
      processParameterSet.put(key, value);
    }

    ProcessConfiguration const processConfiguration{
      process_name, processParameterSet.id(), getReleaseVersion()};
    processHistory.push_back(processConfiguration);
  }

  auto const processHistoryID = processHistory.id();
  ProcessHistoryRegistry::emplace(processHistoryID, processHistory);
  return processHistory;
}

template <class T, bool Present>
ModuleDescription const&
ProductTablesFixture::registerProduct(std::string const& tag,
                                      std::string const& moduleLabel,
                                      std::string const& processName,
                                      std::string const& productInstanceName)
{
  ParameterSet moduleParams;
  moduleParams.put("module_type", module_class_name());
  moduleParams.put("module_label", moduleLabel);

  moduleConfigurations_[processName][moduleLabel] = moduleParams;

  ParameterSet processParams;
  processParams.put("process_name", processName);
  processParams.put(moduleLabel, moduleParams);

  ProcessConfiguration const process{
    processName, processParams.id(), getReleaseVersion()};

  ModuleDescription const localModuleDescription{moduleParams.id(),
                                                 module_class_name(),
                                                 moduleLabel,
                                                 ModuleThreadingType::legacy,
                                                 process,
                                                 Present};

  TypeID const product_type{typeid(T)};

  moduleDescriptions_[tag] = localModuleDescription;
  TypeLabel typeLabel{
    product_type, productInstanceName, SupportsView<T>::value, false};
  if (Present) {
    typeLabel = TypeLabel{
      product_type, productInstanceName, SupportsView<T>::value, moduleLabel};
  }
  BranchDescription pd{
    InEvent, typeLabel, moduleLabel, moduleParams.id(), process};
  if (Present) {
    earlyLookup_[tag].emplace(typeLabel, pd);
  } else {
    currentLookup_.emplace(typeLabel, pd);
  }
  descriptions_.push_back(pd);
  return moduleDescriptions_[tag];
}

struct EventTestFixture {
public:
  EventTestFixture();

  template <class T>
  ProductID addSourceProduct(std::unique_ptr<T>&& product,
                             std::string const& tag,
                             std::string const& instanceName = {});

  ProductTablesFixture const& ptf();
  ProcessHistory processHistory_;
  ProcessConfiguration const& processConfiguration_;
  ModuleContext currentModuleContext_;
  RunPrincipal runPrincipal_;
  std::unique_ptr<SubRunPrincipal> subRunPrincipal_;
  std::unique_ptr<EventPrincipal> principal_;
  Event currentEvent_;
};

namespace {
  auto
  make_run_principal(ProcessConfiguration const& pc)
  {
    RunAuxiliary const runAux{make_id().run(), timestamp, timestamp};
    return RunPrincipal{runAux, pc, nullptr};
  }

  auto
  make_subrun_principal(RunPrincipal const& rp, ProcessConfiguration const& pc)
  {
    SubRunAuxiliary const subRunAux{rp.run(), 1u, timestamp, timestamp};
    auto srp = std::make_unique<SubRunPrincipal>(subRunAux, pc, nullptr);
    srp->setRunPrincipal(&rp);
    return srp;
  }

  auto
  make_event_principal(SubRunPrincipal const* srp,
                       ProcessConfiguration const& pc,
                       ProductTablesFixture const& ptf,
                       ProcessHistoryID const& phid)
  {
    // When any new product is added to the event principal at
    // commitProducts, the "CURRENT" process will go into the
    // ProcessHistory because the process name comes from the
    // currentModuleDescription stored in the principal.  On the other
    // hand, when addSourceProduct is called, another event is created
    // with a fake moduleDescription containing the old process name
    // and that is used to create the group in the principal used to
    // look up the object.
    EventAuxiliary eventAux{make_id(), timestamp, true};
    eventAux.setProcessHistoryID(phid);
    auto ep =
      std::make_unique<EventPrincipal>(eventAux, pc, &ptf.presentProducts_);
    ep->setSubRunPrincipal(srp);
    ep->createGroupsForProducedProducts(ptf.producedProducts_);
    ep->enableLookupOfProducedProducts();
    return ep;
  }
}

EventTestFixture::EventTestFixture()
  : processHistory_{ptf().processHistory()}
  , processConfiguration_{ptf()
                            .currentModuleDescription_.processConfiguration()}
  , currentModuleContext_{ptf().currentModuleDescription_}
  , runPrincipal_{make_run_principal(processConfiguration_)}
  , subRunPrincipal_{make_subrun_principal(runPrincipal_,
                                           processConfiguration_)}
  , principal_{make_event_principal(subRunPrincipal_.get(),
                                    processConfiguration_,
                                    ptf(),
                                    processHistory_.id())}
  , currentEvent_{principal_->makeEvent(currentModuleContext_)}
{}

// Add the given product, of type T, to the current event, as if it
// came from the module/source specified by the given tag.
template <class T>
ProductID
EventTestFixture::addSourceProduct(std::unique_ptr<T>&& product,
                                   std::string const& tag,
                                   std::string const& instanceName)
{
  auto description = ptf().moduleDescriptions_.find(tag);
  if (description == ptf().moduleDescriptions_.end())
    throw art::Exception(errors::LogicError)
      << "Failed to find a module description for tag: " << tag << '\n';

  auto lookup = ptf().earlyLookup_.find(tag);
  if (lookup == ptf().earlyLookup_.end())
    throw art::Exception(errors::LogicError)
      << "Failed to find a product lookup for tag: " << tag << '\n';

  ModuleContext const tempMC{description->second};
  auto temporaryEvent = principal_->makeEvent(tempMC);
  ProductID const id{temporaryEvent.put(std::move(product), instanceName)};

  detail::Producer::commit(temporaryEvent);
  return id;
}

ProductTablesFixture const&
EventTestFixture::ptf()
{
  static ProductTablesFixture const ptf_s;
  return ptf_s;
}

BOOST_FIXTURE_TEST_SUITE(Event_t, EventTestFixture)

BOOST_AUTO_TEST_CASE(badProductID)
{
  auto const pid = ProductID::invalid();
  InputTag tag{};
  if (auto pd = currentEvent_.getProductDescription(pid)) {
    tag = pd->inputTag();
  }
  BOOST_TEST(tag.empty());
}

BOOST_AUTO_TEST_CASE(emptyEvent)
{
  BOOST_TEST(currentEvent_.id() == make_id());
  BOOST_TEST(currentEvent_.time() == timestamp);
}

BOOST_AUTO_TEST_CASE(getBySelectorFromEmpty)
{
  ModuleLabelSelector const byModuleLabel{"mod1"};
  Handle<int> nonesuch;
  BOOST_TEST(not nonesuch);
  BOOST_TEST(!nonesuch.isValid());
  BOOST_TEST(!currentEvent_.get(byModuleLabel, nonesuch));
  BOOST_TEST(!nonesuch.isValid());
  BOOST_TEST(nonesuch.failedToGet());
  BOOST_CHECK_THROW(*nonesuch, cet::exception);
}

BOOST_AUTO_TEST_CASE(dereferenceDfltHandle)
{
  Handle<int> nonesuch;
  BOOST_CHECK_THROW(*nonesuch, cet::exception);
  BOOST_CHECK_THROW(nonesuch.product(), cet::exception);
}

BOOST_AUTO_TEST_CASE(putAnIntProduct)
{
  currentEvent_.put(product_with_value(3), "int1");
  detail::Producer::commit(currentEvent_);
}

BOOST_AUTO_TEST_CASE(putAndGetAnIntProduct)
{
  currentEvent_.put(product_with_value(4), "int1");
  detail::Producer::commit(currentEvent_);

  ProcessNameSelector const should_match{"CURRENT"};
  ProcessNameSelector const should_not_match{"NONESUCH"};
  // The ProcessNameSelector does not understand the "current_process"
  // string.  See notes in the Selector header file.
  ProcessNameSelector const should_also_not_match{"current_process"};
  Handle<arttest::IntProduct> h;
  BOOST_TEST_REQUIRE(currentEvent_.get(should_match, h));
  BOOST_TEST(h);
  BOOST_TEST(h.isValid());
  h.clear();
  BOOST_CHECK_THROW(*h, cet::exception);
  BOOST_TEST_REQUIRE(!currentEvent_.get(should_not_match, h));
  BOOST_TEST(!h.isValid());
  BOOST_CHECK_THROW(*h, cet::exception);
  BOOST_TEST_REQUIRE(!currentEvent_.get(should_also_not_match, h));
  BOOST_TEST(!h.isValid());
  BOOST_CHECK_THROW(*h, cet::exception);
}

BOOST_AUTO_TEST_CASE(getByProductID)
{
  using handle_t = Handle<product_t>;

  ProductID wanted;
  {
    auto const id1 =
      addSourceProduct(product_with_value(1), "int1_tag", "int1");
    BOOST_TEST(id1 != ProductID{});
    wanted = id1;

    auto const id2 =
      addSourceProduct(product_with_value(2), "int2_tag", "int2");
    BOOST_TEST(id2 != ProductID{});
    BOOST_TEST(id2 != id1);
  }
  Handle<arttest::IntProduct> h;
  currentEvent_.get(wanted, h);
  BOOST_TEST(h.isValid());
  BOOST_TEST(h.id() == wanted);
  BOOST_TEST(h->value == 1);

  ProductID const notpresent{};
  BOOST_TEST_REQUIRE(!currentEvent_.get(notpresent, h));
  BOOST_TEST(!h.isValid());
  BOOST_TEST(h.failedToGet());
  BOOST_CHECK_THROW(*h, cet::exception);
}

BOOST_AUTO_TEST_CASE(transaction)
{
  // Put a product into an Event, and make sure that if we don't
  // commitProducts, there is no product in the EventPrincipal
  // afterwards.
  BOOST_TEST(principal_->size() == 6u);
  currentEvent_.put(product_with_value(3), "int1");
  BOOST_TEST(principal_->size() == 6u);
}

BOOST_AUTO_TEST_CASE(getProductTokens)
{
  addSourceProduct(product_with_value(1), "int1_tag", "int1");
  addSourceProduct(product_with_value(2), "int2_tag", "int2");
  addSourceProduct(product_with_value(3), "int3_tag");
  addSourceProduct(product_with_value(4), "nolabel_tag");

  auto const tags = currentEvent_.getInputTags<product_t>();
  auto const tokens = currentEvent_.getProductTokens<product_t>();
  BOOST_TEST(size(tags) == size(tokens));

  // Verify that the same products are retrieved whether tags or
  // tokens are used.
  for (std::size_t i{}; i < tags.size(); ++i) {
    auto h1 = currentEvent_.getHandle<product_t>(tags[i]);
    auto h2 = currentEvent_.getHandle(tokens[i]);
    BOOST_TEST(static_cast<bool>(h1) == static_cast<bool>(h2));
  }
}

BOOST_AUTO_TEST_CASE(getByInstanceName)
{
  using handle_t = Handle<product_t>;
  using handle_vec = std::vector<handle_t>;

  addSourceProduct(product_with_value(1), "int1_tag", "int1");
  addSourceProduct(product_with_value(2), "int2_tag", "int2");
  addSourceProduct(product_with_value(3), "int3_tag");
  addSourceProduct(product_with_value(4), "nolabel_tag");

  Selector const sel{ProductInstanceNameSelector{"int2"} &&
                     ModuleLabelSelector{"modMulti"}};
  auto h = currentEvent_.getHandle<product_t>(sel);
  BOOST_TEST_REQUIRE(h);
  BOOST_TEST(h->value == 2);

  Selector const sel2{ProductInstanceNameSelector{"int2"} ||
                      ProductInstanceNameSelector{"int1"}};
  auto handles = currentEvent_.getMany<product_t>(sel2);
  BOOST_TEST(handles.size() == std::size_t{2});

  std::string const instance;
  Selector sel1{ProductInstanceNameSelector{instance}};
  sel1 = Selector{sel1 && ModuleLabelSelector{"modMulti"}};

  BOOST_TEST_REQUIRE(currentEvent_.get(sel1, h));
  BOOST_TEST(h->value == 3);
  handles.clear();

  // There should be five registered products with the 'modMulti'
  // module label.
  auto tags = currentEvent_.getInputTags<product_t>(modMultiSelector);
  BOOST_TEST(tags.size() == 5u);
  // Now remove the unavailable products
  auto new_end =
    std::remove_if(begin(tags), end(tags), [this](auto const& tag) {
      return !currentEvent_.getHandle<product_t>(tag);
    });
  tags.erase(new_end, end(tags));

  // Only three of the registered products are available.
  handles = currentEvent_.getMany<product_t>(modMultiSelector);
  BOOST_TEST(handles.size() == 3u);

  // Make sure the resolved products agree with those specified in the
  // 'tags' variable above.  The products are resolved in the same
  // order as the input-tag list.
  for (std::size_t i{}; i < 3u; ++i) {
    BOOST_TEST(handles[i].provenance()->inputTag() == tags[i]);
  }

  handles = currentEvent_.getMany<product_t>(ModuleLabelSelector{"nomatch"});
  BOOST_TEST_REQUIRE(handles.empty());

  auto const nomatches = currentEvent_.getMany<int>(modMultiSelector);
  BOOST_TEST_REQUIRE(nomatches.empty());
}

BOOST_AUTO_TEST_CASE(getBySelector)
{
  using handle_t = Handle<product_t>;
  using handle_vec = std::vector<handle_t>;

  addSourceProduct(product_with_value(1), "int1_tag", "int1");
  addSourceProduct(product_with_value(2), "int2_tag", "int2");
  addSourceProduct(product_with_value(3), "int3_tag");
  addSourceProduct(product_with_value(4), "nolabel_tag");
  addSourceProduct(product_with_value(100), "int1_tag_late", "int1");

  currentEvent_.put(product_with_value(200), "int1");
  detail::Producer::commit(currentEvent_);

  Selector const sel{ProductInstanceNameSelector{"int2"} && modMultiSelector &&
                     ProcessNameSelector{"EARLY"}};
  handle_t h;
  BOOST_TEST_REQUIRE(currentEvent_.get(sel, h));
  BOOST_TEST(h->value == 2);

  Selector const sel1{ProductInstanceNameSelector{"nomatch"} &&
                      modMultiSelector && ProcessNameSelector{"EARLY"}};
  BOOST_TEST_REQUIRE(!currentEvent_.get(sel1, h));
  BOOST_TEST_REQUIRE(!h.isValid());
  BOOST_REQUIRE_THROW(*h, cet::exception);

  Selector const sel2{ProductInstanceNameSelector{"int2"} &&
                      ModuleLabelSelector{"nomatch"} &&
                      ProcessNameSelector{"EARLY"}};
  BOOST_TEST_REQUIRE(!currentEvent_.get(sel2, h));
  BOOST_TEST_REQUIRE(!h.isValid());
  BOOST_REQUIRE_THROW(*h, cet::exception);

  Selector const sel3{ProductInstanceNameSelector{"int2"} && modMultiSelector &&
                      ProcessNameSelector{"nomatch"}};
  BOOST_TEST_REQUIRE(!currentEvent_.get(sel3, h));
  BOOST_TEST_REQUIRE(!h.isValid());
  BOOST_REQUIRE_THROW(*h, cet::exception);

  Selector const sel4{modMultiSelector && ProcessNameSelector{"EARLY"}};
  // multiple selections throw
  BOOST_REQUIRE_THROW(currentEvent_.get(sel4, h), cet::exception);

  Selector const sel5{modMultiSelector && ProcessNameSelector{"LATE"}};
  BOOST_TEST_REQUIRE(currentEvent_.get(sel5, h));
  BOOST_TEST(h->value == 100);

  Selector const sel6{modMultiSelector && ProcessNameSelector{"CURRENT"}};
  BOOST_TEST_REQUIRE(currentEvent_.get(sel6, h));
  BOOST_TEST(h->value == 200);

  Selector const sel7{modMultiSelector};
  BOOST_TEST_REQUIRE(currentEvent_.get(sel7, h));
  BOOST_TEST(h->value == 200);

  auto handles = currentEvent_.getMany<product_t>(modMultiSelector);
  BOOST_TEST(size(handles) == 5u);
  int sum = 0;
  for (int k = 0; k < 5; ++k) {
    sum += handles[k]->value;
  }
  BOOST_TEST(sum == 306);
}

BOOST_AUTO_TEST_CASE(getByLabel)
{
  using handle_t = Handle<product_t>;
  using handle_vec = std::vector<handle_t>;

  addSourceProduct(product_with_value(1), "int1_tag", "int1");
  addSourceProduct(product_with_value(2), "int2_tag", "int2");
  addSourceProduct(product_with_value(3), "int3_tag");
  addSourceProduct(product_with_value(4), "nolabel_tag");
  addSourceProduct(product_with_value(100), "int1_tag_late", "int1");

  currentEvent_.put(product_with_value(200), "int1");

  detail::Producer::commit(currentEvent_);

  handle_t h;
  BOOST_TEST_REQUIRE(currentEvent_.getByLabel("modMulti", h));
  BOOST_TEST(h->value == 3);
  BOOST_TEST_REQUIRE(currentEvent_.getByLabel("modMulti", "int1", h));
  BOOST_TEST(h->value == 200);
  BOOST_TEST_REQUIRE(
    currentEvent_.getByLabel("modMulti", "int1", "CURRENT", h));
  BOOST_TEST(h->value == 200);
  BOOST_TEST_REQUIRE(
    currentEvent_.getByLabel("modMulti", "int1", "current_process", h));
  BOOST_TEST(h->value == 200);
  BOOST_TEST(!currentEvent_.getByLabel("modMulti", "nomatch", h));
  BOOST_TEST(!h.isValid());
  BOOST_CHECK_THROW(*h, cet::exception);

  InputTag const inputTag{"modMulti", "int1"};
  BOOST_TEST_REQUIRE(currentEvent_.getByLabel(inputTag, h));
  BOOST_TEST(h->value == 200);

  GroupQueryResult bh{principal_->getByLabel(currentModuleContext_,
                                             WrappedTypeID::make<product_t>(),
                                             "modMulti",
                                             "int1",
                                             ProcessTag{"LATE", "CURRENT"})};
  h = handle_t{bh};
  BOOST_TEST(h->value == 100);

  GroupQueryResult bh2{
    principal_->getByLabel(currentModuleContext_,
                           WrappedTypeID::make<product_t>(),
                           "modMulti",
                           "int1",
                           ProcessTag{"nomatch", "CURRENT"})};
  BOOST_TEST(!bh2.succeeded());
}

BOOST_AUTO_TEST_CASE(getByLabelSpecialProcessNames)
{
  addSourceProduct(product_with_value(2), "int2_tag", "int2");
  currentEvent_.put(product_with_value(200), "int1");

  // Try to retrieve something that does not exist in the current process.
  InputTag const badEarlyTag{"modMulti", "int2", "current_process"};
  BOOST_REQUIRE_THROW(currentEvent_.getValidHandle<product_t>(badEarlyTag),
                      cet::exception);

  // Verify that it can be read using the 'input_source' process name
  Handle<product_t> h;
  InputTag const goodEarlyTag{"modMulti", "int2", "input_source"};
  BOOST_TEST_REQUIRE(currentEvent_.getByLabel(goodEarlyTag, h));
  BOOST_TEST(h->value == 2);

  // Verify that "int1" cannot be looked up using the "input_source"
  // process name.
  InputTag const badCurrentTag{"modMulti", "int1", "input_source"};
  BOOST_CHECK_THROW(currentEvent_.getValidHandle<product_t>(badCurrentTag),
                    cet::exception);
}

BOOST_AUTO_TEST_CASE(getManyByType)
{
  addSourceProduct(product_with_value(1), "int1_tag", "int1");
  addSourceProduct(product_with_value(2), "int2_tag", "int2");
  addSourceProduct(product_with_value(3), "int3_tag");
  addSourceProduct(product_with_value(4), "nolabel_tag");

  addSourceProduct(product_with_value(100), "int1_tag_late", "int1");

  currentEvent_.put(product_with_value(200), "int1");
  detail::Producer::commit(currentEvent_);

  // Verify that the returned tags match thos provided through the
  // handles below.
  auto const tags = currentEvent_.getInputTags<product_t>();
  BOOST_TEST(tags.size() == 6u);

  auto const handles = currentEvent_.getMany<product_t>();
  BOOST_TEST(handles.size() == 6u);
  int sum = 0;
  for (int k = 0; k < 6; ++k) {
    auto const& h = handles[k];
    BOOST_TEST(tags[k] == h.provenance()->productDescription().inputTag());
    sum += h->value;
  }
  BOOST_TEST(sum == 310);
}

BOOST_AUTO_TEST_CASE(printHistory)
{
  auto const& history = currentEvent_.processHistory();
  std::ofstream out{"history.log"};

  cet::copy_all(
    history,
    std::ostream_iterator<ProcessHistory::const_iterator::value_type>(out,
                                                                      "\n"));
}

BOOST_AUTO_TEST_SUITE_END()
