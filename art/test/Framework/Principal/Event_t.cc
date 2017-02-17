#define BOOST_TEST_MODULE ( Event_t )
#include "cetlib/quiet_unit_test.hpp"

#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/Selector.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Persistency/Provenance/BranchIDListRegistry.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "art/Version/GetReleaseVersion.h"
#include "canvas/Persistency/Common/Wrapper.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/EventAuxiliary.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/History.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/ProcessHistory.h"
#include "canvas/Persistency/Provenance/RunAuxiliary.h"
#include "canvas/Persistency/Provenance/SubRunAuxiliary.h"
#include "canvas/Persistency/Provenance/Timestamp.h"
#include "canvas/Persistency/Provenance/TypeLabel.h"
#include "canvas/Utilities/GetPassID.h"
#include "canvas/Utilities/InputTag.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/ParameterSet.h"
#include "art/test/TestObjects/ToyProducts.h"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <string>
#include <typeinfo>
#include <vector>

using namespace art;
using fhicl::ParameterSet;

namespace {
  art::EventID   make_id() { return art::EventID{2112, 47, 25}; }
  art::Timestamp make_timestamp() { return art::Timestamp{1}; }
}

// This is a gross hack, to allow us to test the event
namespace art {
  class EDProducer {
  public:
    static void commitEvent(EventPrincipal& ep, Event& e, ProducedMap const& expectedProducts)
    {
      e.commit_(ep, false, expectedProducts);
    }
  };
}

class MPRGlobalTestFixture {
public:
  MPRGlobalTestFixture();

  template <typename T>
  void registerProduct(std::string const& tag,
                       std::string const& moduleLabel,
                       std::string const& moduleClassName,
                       std::string const& processName,
                       std::string const& productInstanceName = std::string());

  std::unique_ptr<MasterProductRegistry> availableProducts_;
  std::unique_ptr<ModuleDescription> currentModuleDescription_;
  using modCache_t = std::map<std::string, ModuleDescription>;
  using iterator_t = modCache_t::iterator;

  modCache_t moduleDescriptions_;
};

MPRGlobalTestFixture::MPRGlobalTestFixture()
  :
  availableProducts_(new MasterProductRegistry()),
  currentModuleDescription_(new ModuleDescription),
  moduleDescriptions_()
{
  using prod_t = arttest::IntProduct;

  registerProduct<prod_t>("nolabel_tag",   "modOne",   "IntProducer",   "EARLY");
  registerProduct<prod_t>("int1_tag",      "modMulti", "IntProducer",   "EARLY", "int1");
  registerProduct<prod_t>("int1_tag_late", "modMulti", "IntProducer",   "LATE",  "int1");
  registerProduct<prod_t>("int2_tag",      "modMulti", "IntProducer",   "EARLY", "int2");
  registerProduct<prod_t>("int3_tag",      "modMulti", "IntProducer",   "EARLY");

  // Fake up the production of a single IntProduct from an IntProducer
  // module, run in the 'CURRENT' process.
  ParameterSet moduleParams;
  std::string moduleLabel("modMulti");
  std::string moduleClassName("IntProducer");
  moduleParams.put<std::string>("module_type", moduleClassName);
  moduleParams.put<std::string>("module_label", moduleLabel);

  ParameterSet processParams;
  std::string processName("CURRENT");
  processParams.put<std::string>("process_name", processName);
  processParams.put(moduleLabel, moduleParams);

  ProcessConfiguration process;
  process.processName_    = processName;
  process.releaseVersion_ = getReleaseVersion();
  process.passID_         = getPassID();
  process.parameterSetID_ = processParams.id();

  TypeID product_type(typeid(prod_t));

  currentModuleDescription_ = std::make_unique<ModuleDescription>(moduleParams.id(),
                                                                  moduleClassName,
                                                                  moduleLabel,
                                                                  process);

  std::string productInstanceName("int1");

  availableProducts_->addProduct(std::make_unique<BranchDescription>
                                 (art::TypeLabel(InEvent,
                                                 product_type,
                                                 productInstanceName),
                                  *currentModuleDescription_));

  // Freeze the product registry before we make the Event.
  availableProducts_->setFrozen();
  ProductMetaData::create_instance(*availableProducts_);
  BranchIDListRegistry::updateFromProductRegistry(*availableProducts_);
}

template <class T>
void
MPRGlobalTestFixture::
registerProduct(std::string const& tag,
                std::string const& moduleLabel,
                std::string const& moduleClassName,
                std::string const& processName,
                std::string const& productInstanceName)
{
  ParameterSet moduleParams;
  moduleParams.put<std::string>("module_type", moduleClassName);
  moduleParams.put<std::string>("module_label", moduleLabel);

  ParameterSet processParams;
  processParams.put<std::string>("process_name", processName);
  processParams.put<ParameterSet>(moduleLabel, moduleParams);

  ProcessConfiguration process;
  process.processName_    = processName;
  process.releaseVersion_ = getReleaseVersion();
  process.passID_         = getPassID();
  process.parameterSetID_ = processParams.id();

  ModuleDescription localModuleDescription(moduleParams.id(),
                                           moduleClassName,
                                           moduleLabel,
                                           process);

  TypeID product_type(typeid(T));

  moduleDescriptions_[tag] = localModuleDescription;
  availableProducts_->addProduct(std::make_unique<BranchDescription>
                                 (art::TypeLabel(InEvent,
                                                 product_type,
                                                 productInstanceName),
                                  localModuleDescription));
}

struct EventTestFixture {
  EventTestFixture();

  template <class T>
  ProductID addProduct(std::unique_ptr<T> && product,
                       std::string const& tag,
                       std::string const& productLabel = std::string());

  MPRGlobalTestFixture &gf();
  using modCache_t = MPRGlobalTestFixture::modCache_t;
  using iterator_t = MPRGlobalTestFixture::iterator_t;


  std::unique_ptr<EventPrincipal> principal_;
  std::unique_ptr<Event> currentEvent_;
};

EventTestFixture::EventTestFixture()
  :
  principal_(),
  currentEvent_()
{
  // First build a fake process history, that says there
  // were previous processes named "EARLY" and "LATE".
  // This takes several lines of code but other than
  // the process names none of it is used or interesting.
  ParameterSet moduleParamsEarly;
  std::string moduleLabelEarly("currentModule");
  std::string moduleClassNameEarly("IntProducer");
  moduleParamsEarly.put<std::string>("module_type", moduleClassNameEarly);
  moduleParamsEarly.put<std::string>("module_label", moduleLabelEarly);

  ParameterSet processParamsEarly;
  std::string processNameEarly("EARLY");
  processParamsEarly.put<std::string>("process_name", processNameEarly);
  processParamsEarly.put(moduleLabelEarly, moduleParamsEarly);

  ProcessConfiguration processEarly;
  processEarly.processName_    = "EARLY";
  processEarly.releaseVersion_ = getReleaseVersion();
  processEarly.passID_         = getPassID();
  processEarly.parameterSetID_ = processParamsEarly.id();

  ParameterSet moduleParamsLate;
  std::string moduleLabelLate("currentModule");
  std::string moduleClassNameLate("IntProducer");
  moduleParamsLate.put<std::string>("module_type", moduleClassNameLate);
  moduleParamsLate.put<std::string>("module_label", moduleLabelLate);

  ParameterSet processParamsLate;
  std::string processNameLate("LATE");
  processParamsLate.put<std::string>("process_name", processNameLate);
  processParamsLate.put(moduleLabelLate, moduleParamsLate);

  ProcessConfiguration processLate;
  processLate.processName_    = "LATE";
  processLate.releaseVersion_ = getReleaseVersion();
  processLate.passID_         = getPassID();
  processLate.parameterSetID_ = processParamsLate.id();

  ProcessHistory* processHistory = new ProcessHistory;
  ProcessHistory& ph = *processHistory;
  processHistory->push_back(processEarly);
  processHistory->push_back(processLate);

  ProcessHistoryRegistry::emplace(ph.id(), ph);

  ProcessHistoryID processHistoryID = ph.id();

  // Finally done with making a fake process history

  // This is all a hack to make this test work, but here is some
  // detail as to what is going on.  (this does not happen in real
  // data processing).
  // When any new product is added to the event principal at
  // commit, the "CURRENT" process will go into the ProcessHistory
  // even if we have faked it that the new product is associated
  // with a previous process, because the process name comes from
  // the currentModuleDescription stored in the principal.  On the
  // other hand, when addProduct is called another event is created
  // with a fake moduleDescription containing the old process name
  // and that is used to create the group in the principal used to
  // look up the object.

  Timestamp time = make_timestamp();
  EventID id = make_id();
  ProcessConfiguration const& pc = gf().currentModuleDescription_->processConfiguration();
  RunAuxiliary runAux(id.run(), time, time);
  auto rp = std::make_unique<RunPrincipal>(runAux, pc);
  SubRunAuxiliary subRunAux {rp->run(), 1u, time, time};
  auto srp = std::make_unique<SubRunPrincipal>(subRunAux, pc);
  srp->setRunPrincipal(rp.get());
  EventAuxiliary eventAux(id, time, true);
  auto history = std::make_shared<History>();
  const_cast<ProcessHistoryID &>(history->processHistoryID()) = processHistoryID;
  principal_ = std::make_unique<EventPrincipal>(eventAux, pc, history);

  principal_->setSubRunPrincipal(srp.get());
  currentEvent_ = std::make_unique<Event>(*principal_, *gf().currentModuleDescription_);

  delete processHistory;
}

// Add the given product, of type T, to the current event,
// as if it came from the module specified by the given tag.
template <class T>
ProductID
EventTestFixture::
addProduct(std::unique_ptr<T> && product,
           std::string const& tag,
           std::string const& productLabel)
{
  iterator_t description = gf().moduleDescriptions_.find(tag);
  if (description == gf().moduleDescriptions_.end())
    throw art::Exception(errors::LogicError)
      << "Failed to find a module description for tag: "
      << tag << '\n';

  Event temporaryEvent(*principal_, description->second);
  ProductID id = temporaryEvent.put(std::move(product), productLabel);
  EDProducer::commitEvent(*principal_, temporaryEvent, ProducedMap{});
  return id;
}

MPRGlobalTestFixture &
EventTestFixture::gf() {
  static MPRGlobalTestFixture gf_s;
  return gf_s;
}

BOOST_FIXTURE_TEST_SUITE(Event_t, EventTestFixture)

BOOST_AUTO_TEST_CASE(emptyEvent)
{
  BOOST_REQUIRE(currentEvent_.get());
  BOOST_REQUIRE_EQUAL(currentEvent_->id(), make_id());
  BOOST_REQUIRE(currentEvent_->time() == make_timestamp());
  BOOST_REQUIRE_EQUAL(currentEvent_->size(), 0u);
}

BOOST_AUTO_TEST_CASE(getBySelectorFromEmpty)
{
  ModuleLabelSelector byModuleLabel("mod1");
  Handle<int> nonesuch;
  BOOST_REQUIRE(!nonesuch.isValid());
  BOOST_REQUIRE(!currentEvent_->get(byModuleLabel, nonesuch));
  BOOST_REQUIRE(!nonesuch.isValid());
  BOOST_REQUIRE(nonesuch.failedToGet());
  BOOST_REQUIRE_THROW(*nonesuch,
                      cet::exception);
}

BOOST_AUTO_TEST_CASE(dereferenceDfltHandle)
{
  Handle<int> nonesuch;
  BOOST_REQUIRE_THROW(*nonesuch, cet::exception);
  BOOST_REQUIRE_THROW( nonesuch.product(), cet::exception);
}

BOOST_AUTO_TEST_CASE(putAnIntProduct)
{
  auto three = std::make_unique<arttest::IntProduct>(3);
  currentEvent_->put(std::move(three), "int1");
  BOOST_REQUIRE_EQUAL(currentEvent_->size(), 1u);
  EDProducer::commitEvent(*principal_, *currentEvent_, ProducedMap{});
  BOOST_REQUIRE_EQUAL(currentEvent_->size(), 1u);
}

BOOST_AUTO_TEST_CASE(putAndGetAnIntProduct)
{
  auto four = std::make_unique<arttest::IntProduct>(4);
  currentEvent_->put(std::move(four), "int1");
  EDProducer::commitEvent(*principal_, *currentEvent_, ProducedMap{});

  ProcessNameSelector should_match("CURRENT");
  ProcessNameSelector should_not_match("NONESUCH");
  Handle<arttest::IntProduct> h;
  currentEvent_->get(should_match, h);
  BOOST_REQUIRE(h.isValid());
  h.clear();
  BOOST_REQUIRE_THROW(*h, cet::exception);
  BOOST_REQUIRE(!currentEvent_->get(should_not_match, h));
  BOOST_REQUIRE(!h.isValid());
  BOOST_REQUIRE_THROW(*h, cet::exception);
}

BOOST_AUTO_TEST_CASE(getByProductID)
{

  using product_t = arttest::IntProduct;
  using handle_t  = Handle<product_t>;

  ProductID wanted;

  {
    auto one = std::make_unique<product_t>(1);
    ProductID id1 = addProduct(std::move(one), "int1_tag", "int1");
    BOOST_REQUIRE(id1 != ProductID());
    wanted = id1;

    auto two = std::make_unique<product_t>(2);
    ProductID id2 = addProduct(std::move(two), "int2_tag", "int2");
    BOOST_REQUIRE(id2 != ProductID());
    BOOST_REQUIRE(id2 != id1);

    EDProducer::commitEvent(*principal_, *currentEvent_, ProducedMap{});
    BOOST_REQUIRE_EQUAL(currentEvent_->size(), 2u);
  }

  handle_t h;
  currentEvent_->get(wanted, h);
  BOOST_REQUIRE(h.isValid());
  BOOST_REQUIRE_EQUAL(h.id(), wanted);
  BOOST_REQUIRE_EQUAL(h->value, 1);

  ProductID invalid;
  BOOST_REQUIRE_THROW(currentEvent_->get(invalid, h), cet::exception);
  BOOST_REQUIRE(!h.isValid());
  ProductID notpresent(0, std::numeric_limits<unsigned short>::max());
  BOOST_REQUIRE(!currentEvent_->get(notpresent, h));
  BOOST_REQUIRE(!h.isValid());
  BOOST_REQUIRE(h.failedToGet());
  BOOST_REQUIRE_THROW(*h, cet::exception);
}

BOOST_AUTO_TEST_CASE(transaction)
{
  // Put a product into an Event, and make sure that if we don't
  // commit, there is no product in the EventPrincipal afterwards.
  BOOST_REQUIRE_EQUAL(principal_->size(), 0u);
  {
    auto three = std::make_unique<arttest::IntProduct>(3);
    currentEvent_->put(std::move(three), "int1");
    BOOST_REQUIRE_EQUAL(principal_->size(), 0u);
    BOOST_REQUIRE_EQUAL(currentEvent_->size(), 1u);
    // DO NOT COMMIT!
  }

  // The Event has been destroyed without a commit -- we should not
  // have any products in the EventPrincipal.
  BOOST_REQUIRE_EQUAL(principal_->size(), 0u);
}

BOOST_AUTO_TEST_CASE(getByInstanceName)
{
  using product_t  = arttest::IntProduct;
  using handle_t   = Handle<product_t>;
  using handle_vec = std::vector<handle_t>;

  auto one   = std::make_unique<product_t>(1);
  auto two   = std::make_unique<product_t>(2);
  auto three = std::make_unique<product_t>(3);
  auto four  = std::make_unique<product_t>(4);
  addProduct(std::move(one),   "int1_tag", "int1");
  addProduct(std::move(two),   "int2_tag", "int2");
  addProduct(std::move(three), "int3_tag");
  addProduct(std::move(four),  "nolabel_tag");

  BOOST_REQUIRE_EQUAL(currentEvent_->size(), 4u);

  Selector sel(ProductInstanceNameSelector("int2") &&
               ModuleLabelSelector("modMulti"));
  handle_t h;
  BOOST_REQUIRE(currentEvent_->get(sel, h));
  BOOST_REQUIRE_EQUAL(h->value, 2);

  Selector sel2(ProductInstanceNameSelector("int2") ||
                ProductInstanceNameSelector("int1"));
  handle_vec handles;
  currentEvent_->getMany(sel2, handles);
  BOOST_REQUIRE_EQUAL(handles.size(), std::size_t(2));

  std::string instance;
  Selector sel1(ProductInstanceNameSelector(instance) &&
                ModuleLabelSelector("modMulti"));

  BOOST_REQUIRE(currentEvent_->get(sel1, h));
  BOOST_REQUIRE_EQUAL(h->value, 3);

  handles.clear();
  currentEvent_->getMany(ModuleLabelSelector("modMulti"), handles);
  BOOST_REQUIRE_EQUAL(handles.size(), 3u);
  handles.clear();
  currentEvent_->getMany(ModuleLabelSelector("nomatch"), handles);
  BOOST_REQUIRE(handles.empty());
  std::vector<Handle<int> > nomatches;
  currentEvent_->getMany(ModuleLabelSelector("modMulti"), nomatches);
  BOOST_REQUIRE(nomatches.empty());
}

BOOST_AUTO_TEST_CASE(getBySelector)
{
  using product_t  = arttest::IntProduct;
  using handle_t   = Handle<product_t>;
  using handle_vec = std::vector<handle_t>;

  auto one   = std::make_unique<product_t>(1);
  auto two   = std::make_unique<product_t>(2);
  auto three = std::make_unique<product_t>(3);
  auto four  = std::make_unique<product_t>(4);
  addProduct(std::move(one),   "int1_tag", "int1");
  addProduct(std::move(two),   "int2_tag", "int2");
  addProduct(std::move(three), "int3_tag");
  addProduct(std::move(four),  "nolabel_tag");

  //  std::unique_ptr<std::vector<arttest::Thing> > ap_vthing(new std::vector<arttest::Thing>);
  //  addProduct(ap_vthing, "thing", "");

  auto oneHundred = std::make_unique<product_t>(100);
  addProduct(std::move(oneHundred), "int1_tag_late", "int1");

  auto twoHundred = std::make_unique<product_t>(200);
  currentEvent_->put(std::move(twoHundred), "int1");
  EDProducer::commitEvent(*principal_, *currentEvent_, ProducedMap{});

  BOOST_REQUIRE_EQUAL(currentEvent_->size(), 6u);

  Selector sel(ProductInstanceNameSelector("int2") &&
               ModuleLabelSelector("modMulti") &&
               ProcessNameSelector("EARLY"));
  handle_t h;
  BOOST_REQUIRE(currentEvent_->get(sel, h));
  BOOST_REQUIRE_EQUAL(h->value, 2);

  Selector sel1(ProductInstanceNameSelector("nomatch") &&
                ModuleLabelSelector("modMulti") &&
                ProcessNameSelector("EARLY"));
  BOOST_REQUIRE(!currentEvent_->get(sel1, h));
  BOOST_REQUIRE(!h.isValid());
  BOOST_REQUIRE_THROW(*h, cet::exception);

  Selector sel2(ProductInstanceNameSelector("int2") &&
                ModuleLabelSelector("nomatch") &&
                ProcessNameSelector("EARLY"));
  BOOST_REQUIRE(!currentEvent_->get(sel2, h));
  BOOST_REQUIRE(!h.isValid());
  BOOST_REQUIRE_THROW(*h, cet::exception);

  Selector sel3(ProductInstanceNameSelector("int2") &&
                ModuleLabelSelector("modMulti") &&
                ProcessNameSelector("nomatch"));
  BOOST_REQUIRE(!currentEvent_->get(sel3, h));
  BOOST_REQUIRE(!h.isValid());
  BOOST_REQUIRE_THROW(*h, cet::exception);

  Selector sel4(ModuleLabelSelector("modMulti") &&
                ProcessNameSelector("EARLY"));
  //multiple selections throw
  BOOST_REQUIRE_THROW(currentEvent_->get(sel4, h), cet::exception);

  Selector sel5(ModuleLabelSelector("modMulti") &&
                ProcessNameSelector("LATE"));
  currentEvent_->get(sel5, h);
  BOOST_REQUIRE_EQUAL(h->value, 100);

  Selector sel6(ModuleLabelSelector("modMulti") &&
                ProcessNameSelector("CURRENT"));
  currentEvent_->get(sel6, h);
  BOOST_REQUIRE_EQUAL(h->value, 200);

  Selector sel7(ModuleLabelSelector("modMulti"));
  currentEvent_->get(sel7, h);
  BOOST_REQUIRE_EQUAL(h->value, 200);

  handle_vec handles;
  currentEvent_->getMany(ModuleLabelSelector("modMulti"), handles);
  BOOST_REQUIRE_EQUAL(handles.size(), 5u);
  int sum = 0;
  for (int k = 0; k < 5; ++k) sum += handles[k]->value;
  BOOST_REQUIRE_EQUAL(sum, 306);
}

BOOST_AUTO_TEST_CASE(getByLabel)
{
  using product_t  = arttest::IntProduct;
  using handle_t   = Handle<product_t>;
  using handle_vec = std::vector<handle_t>;

  auto one   = std::make_unique<product_t>(1);
  auto two   = std::make_unique<product_t>(2);
  auto three = std::make_unique<product_t>(3);
  auto four  = std::make_unique<product_t>(4);
  addProduct(std::move(one),   "int1_tag", "int1");
  addProduct(std::move(two),   "int2_tag", "int2");
  addProduct(std::move(three), "int3_tag");
  addProduct(std::move(four),  "nolabel_tag");

  auto oneHundred = std::make_unique<product_t>(100);
  addProduct(std::move(oneHundred), "int1_tag_late", "int1");

  auto twoHundred = std::make_unique<product_t>(200);
  currentEvent_->put(std::move(twoHundred), "int1");
  EDProducer::commitEvent(*principal_, *currentEvent_, ProducedMap{});

  BOOST_REQUIRE_EQUAL(currentEvent_->size(), 6u);

  handle_t h;
  BOOST_REQUIRE(currentEvent_->getByLabel("modMulti", h));
  BOOST_REQUIRE_EQUAL(h->value, 3);

  BOOST_REQUIRE(currentEvent_->getByLabel("modMulti", "int1", h));
  BOOST_REQUIRE_EQUAL(h->value, 200);

  BOOST_REQUIRE(!currentEvent_->getByLabel("modMulti", "nomatch", h));
  BOOST_REQUIRE(!h.isValid());
  BOOST_REQUIRE_THROW(*h, cet::exception);

  InputTag inputTag("modMulti", "int1");
  BOOST_REQUIRE(currentEvent_->getByLabel(inputTag, h));
  BOOST_REQUIRE_EQUAL(h->value, 200);

  GroupQueryResult bh =
    principal_->getByLabel(TypeID(typeid(arttest::IntProduct)), "modMulti", "int1", "LATE");
  convert_handle(bh, h);
  BOOST_REQUIRE_EQUAL(h->value, 100);
  GroupQueryResult bh2(principal_->getByLabel(TypeID(typeid(arttest::IntProduct)), "modMulti", "int1", "nomatch"));
  BOOST_REQUIRE(!bh2.succeeded());
}

BOOST_AUTO_TEST_CASE(getManyByType)
{
  using product_t  = arttest::IntProduct;
  using handle_t   = Handle<product_t>;
  using handle_vec = std::vector<handle_t>;

  auto one   = std::make_unique<product_t>(1);
  auto two   = std::make_unique<product_t>(2);
  auto three = std::make_unique<product_t>(3);
  auto four  = std::make_unique<product_t>(4);
  addProduct(std::move(one),   "int1_tag", "int1");
  addProduct(std::move(two),   "int2_tag", "int2");
  addProduct(std::move(three), "int3_tag");
  addProduct(std::move(four),  "nolabel_tag");

  auto oneHundred = std::make_unique<product_t>(100);
  addProduct(std::move(oneHundred), "int1_tag_late", "int1");

  auto twoHundred = std::make_unique<product_t>(200);
  currentEvent_->put(std::move(twoHundred), "int1");
  EDProducer::commitEvent(*principal_, *currentEvent_, ProducedMap{});

  BOOST_REQUIRE_EQUAL(currentEvent_->size(), 6u);

  handle_vec handles;
  currentEvent_->getManyByType(handles);
  BOOST_REQUIRE_EQUAL(handles.size(), 6u);
  int sum = 0;
  for (int k = 0; k < 6; ++k) sum += handles[k]->value;
  BOOST_REQUIRE_EQUAL(sum, 310);
}

BOOST_AUTO_TEST_CASE(printHistory)
{
  ProcessHistory const& history = currentEvent_->processHistory();
  std::ofstream out("history.log");

  cet::copy_all(history, std::ostream_iterator<ProcessHistory::const_iterator::value_type>(out, "\n"));
}

BOOST_AUTO_TEST_SUITE_END()
