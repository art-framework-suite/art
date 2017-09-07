// vim: set sw=2 expandtab :
#define BOOST_TEST_MODULE ( Event_t )
#include "cetlib/quiet_unit_test.hpp"

#include "art/Framework/Core/ModuleType.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/Selector.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "art/Version/GetReleaseVersion.h"
#include "art/test/TestObjects/ToyProducts.h"
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
#include "canvas/Utilities/InputTag.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/ParameterSet.h"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <string>
#include <typeinfo>
#include <utility>
#include <vector>

using namespace art;
using namespace std;
using namespace string_literals;

using fhicl::ParameterSet;

namespace {

EventID
make_id()
{
  return EventID{2112, 47, 25};
}

Timestamp
make_timestamp()
{
  return Timestamp{1};
}

} // unnamed namespace

// This is a gross hack, to allow us to test the event
namespace art {

class EDProducer {

public:

  static
  void
  commitEvent(EventPrincipal& /*ep*/, Event& e)
  {
    set<TypeLabel> tlset{};
    e.commit(false, &tlset);
  }

};

} // namespace art

class MPRGlobalTestFixture {

public:

  MPRGlobalTestFixture();

public:

  template <typename T>
  void
  registerProduct(string const& tag, string const& label, string const& modtype, string const& process,
                  string const& instance = {});

public:

  unique_ptr<MasterProductRegistry>
  mpr_;

  unique_ptr<ModuleDescription>
  md_;

  map<string, ModuleDescription>
  tag_to_md_;

};

MPRGlobalTestFixture::
MPRGlobalTestFixture()
  : mpr_{new MasterProductRegistry{}}
  , md_{}
  , tag_to_md_{}
{
  registerProduct<arttest::IntProduct>("nolabel_tag", "modOne", "IntProducer", "EARLY");
  registerProduct<arttest::IntProduct>("int1_tag", "modMulti", "IntProducer", "EARLY", "int1");
  registerProduct<arttest::IntProduct>("int1_tag_late", "modMulti", "IntProducer", "LATE", "int1");
  registerProduct<arttest::IntProduct>("int2_tag", "modMulti", "IntProducer", "EARLY", "int2");
  registerProduct<arttest::IntProduct>("int3_tag", "modMulti", "IntProducer", "EARLY");
  // Fake up the production of a single IntProduct from an IntProducer module, run in the 'CURRENT' process.
  ParameterSet modPSet;
  string moduleLabel{"modMulti"};
  string moduleClassName{"IntProducer"};
  modPSet.put<string>("module_type", moduleClassName);
  modPSet.put<string>("module_label", moduleLabel);
  ParameterSet topPSet;
  string processName("CURRENT");
  topPSet.put<string>("process_name", processName);
  //FIXME: Shouldn't this be under "physics.producers"?
  topPSet.put(moduleLabel, modPSet);
  ProcessConfiguration processConfig(processName, topPSet.id(), getReleaseVersion());
  TypeID product_type(typeid(arttest::IntProduct));
  md_ = make_unique<ModuleDescription>(
    modPSet.id(), moduleClassName, moduleLabel, static_cast<int>(ModuleThreadingType::LEGACY), processConfig);
  string productInstanceName("int1");
  mpr_->addProduct(make_unique<BranchDescription>(InEvent, art::TypeLabel{product_type, productInstanceName}, *md_));
  mpr_->finalizeForProcessing();
  ProductMetaData::create_instance(*mpr_);
}

template <class T>
void
MPRGlobalTestFixture::
registerProduct(string const& tag, string const& label, string const& modtype, string const& process, string const& instance)
{
  ParameterSet modPSet;
  modPSet.put<string>("module_type", modtype);
  modPSet.put<string>("module_label", label);
  ParameterSet topPSet;
  topPSet.put<string>("process_name", process);
  //FIXME: Shouldn't this be under "physics.producers"?
  topPSet.put<ParameterSet>(label, modPSet);
  ProcessConfiguration pconf{process, topPSet.id(), getReleaseVersion()};
  ModuleDescription md{modPSet.id(), modtype, label, static_cast<int>(ModuleThreadingType::LEGACY), pconf};
  tag_to_md_[tag] = md;
  mpr_->addProduct(make_unique<BranchDescription>(InEvent, art::TypeLabel{TypeID{typeid(T)}, instance}, md));
}

struct EventTestFixture {

public:

  EventTestFixture();

public:

  MPRGlobalTestFixture&
  gf();

  template <class T>
  ProductID
  addProduct(unique_ptr<T>&& product, string const& tag, string const& productLabel = string());

public:

  unique_ptr<EventPrincipal>
  principal_;

  unique_ptr<Event>
  currentEvent_;

};

EventTestFixture::
EventTestFixture()
  : principal_{}
  , currentEvent_{}
{
  // First build a fake process history, that says there
  // were previous processes named "EARLY" and "LATE".
  // This takes several lines of code but other than
  // the process names none of it is used or interesting.
  ParameterSet moduleParamsEarly;
  string moduleLabelEarly("currentModule");
  string moduleClassNameEarly("IntProducer");
  moduleParamsEarly.put<string>("module_type", moduleClassNameEarly);
  moduleParamsEarly.put<string>("module_label", moduleLabelEarly);
  ParameterSet processParamsEarly;
  string processNameEarly("EARLY");
  processParamsEarly.put<string>("process_name", processNameEarly);
  processParamsEarly.put(moduleLabelEarly, moduleParamsEarly);
  ProcessConfiguration processEarly("EARLY", processParamsEarly.id(), getReleaseVersion());
  ParameterSet moduleParamsLate;
  string moduleLabelLate("currentModule");
  string moduleClassNameLate("IntProducer");
  moduleParamsLate.put<string>("module_type", moduleClassNameLate);
  moduleParamsLate.put<string>("module_label", moduleLabelLate);
  ParameterSet processParamsLate;
  string processNameLate("LATE");
  processParamsLate.put<string>("process_name", processNameLate);
  processParamsLate.put(moduleLabelLate, moduleParamsLate);
  ProcessConfiguration processLate("LATE", processParamsLate.id(), getReleaseVersion());
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
  ProcessConfiguration const& pc = gf().md_->processConfiguration();
  RunAuxiliary runAux{id.run(), time, time};
  auto rp = make_unique<RunPrincipal>(runAux, pc);
  SubRunAuxiliary subRunAux{rp->run(), 1u, time, time};
  auto srp = make_unique<SubRunPrincipal>(subRunAux, pc);
  srp->setRunPrincipal(rp.get());
  EventAuxiliary eventAux{id, time, true};
  auto history = make_unique<History>();
  const_cast<ProcessHistoryID&>(history->processHistoryID()) = processHistoryID;
  principal_ = make_unique<EventPrincipal>(eventAux, pc, move(history));
  principal_->setSubRunPrincipal(srp.get());
  currentEvent_ = make_unique<Event>(*principal_, *gf().md_);
  delete processHistory;
}

// Add the given product, of type T, to the current event,
// as if it came from the module specified by the given tag.
template <class T>
ProductID
EventTestFixture::
addProduct(unique_ptr<T>&& product, string const& tag, string const& label)
{
  auto description = gf().tag_to_md_.find(tag);
  if (description == gf().tag_to_md_.end()) {
    throw art::Exception(errors::LogicError)
        << "Failed to find a module description for tag: "
        << tag << '\n';
  }
  Event temporaryEvent(*principal_, description->second);
  ProductID id = temporaryEvent.put(move(product), label);
  EDProducer::commitEvent(*principal_, temporaryEvent);
  return id;
}

MPRGlobalTestFixture&
EventTestFixture::
gf()
{
  static MPRGlobalTestFixture gf_s;
  return gf_s;
}

BOOST_FIXTURE_TEST_SUITE(Event_t, EventTestFixture)

BOOST_AUTO_TEST_CASE(emptyEvent)
{
  BOOST_REQUIRE(currentEvent_.get());
  BOOST_REQUIRE_EQUAL(currentEvent_->id(), make_id());
  BOOST_REQUIRE(currentEvent_->time() == make_timestamp());
  BOOST_REQUIRE_EQUAL(currentEvent_->size(), 6u);
}

BOOST_AUTO_TEST_CASE(getBySelectorFromEmpty)
{
  ModuleLabelSelector byModuleLabel("mod1");
  Handle<int> nonesuch;
  BOOST_REQUIRE(!nonesuch.isValid());
  BOOST_REQUIRE(!currentEvent_->get(byModuleLabel, nonesuch));
  BOOST_REQUIRE(!nonesuch.isValid());
  BOOST_REQUIRE(nonesuch.failedToGet());
  BOOST_REQUIRE_THROW(*nonesuch, cet::exception);
}

BOOST_AUTO_TEST_CASE(dereferenceDfltHandle)
{
  Handle<int> nonesuch;
  BOOST_REQUIRE_THROW(*nonesuch, cet::exception);
  BOOST_REQUIRE_THROW(nonesuch.product(), cet::exception);
}

BOOST_AUTO_TEST_CASE(putAnIntProduct)
{
  auto three = make_unique<arttest::IntProduct>(3);
  currentEvent_->put(move(three), "int1");
  BOOST_REQUIRE_EQUAL(currentEvent_->size(), 7u);
  EDProducer::commitEvent(*principal_, *currentEvent_);
  BOOST_REQUIRE_EQUAL(currentEvent_->size(), 6u);
}

BOOST_AUTO_TEST_CASE(putAndGetAnIntProduct)
{
  auto four = make_unique<arttest::IntProduct>(4);
  currentEvent_->put(move(four), "int1");
  EDProducer::commitEvent(*principal_, *currentEvent_);
  ProcessNameSelector should_match{"CURRENT"};
  ProcessNameSelector should_not_match{"NONESUCH"};
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
  ProductID wanted;
  {
    auto one = make_unique<arttest::IntProduct>(1);
    ProductID id1 = addProduct(move(one), "int1_tag", "int1");
    BOOST_REQUIRE(id1 != ProductID());
    wanted = id1;
    auto two = make_unique<arttest::IntProduct>(2);
    ProductID id2 = addProduct(move(two), "int2_tag", "int2");
    BOOST_REQUIRE(id2 != ProductID());
    BOOST_REQUIRE(id2 != id1);
    EDProducer::commitEvent(*principal_, *currentEvent_);
    BOOST_REQUIRE_EQUAL(currentEvent_->size(), 6u);
  }
  Handle<arttest::IntProduct> h;
  currentEvent_->get(wanted, h);
  BOOST_REQUIRE(h.isValid());
  BOOST_REQUIRE_EQUAL(h.id(), wanted);
  BOOST_REQUIRE_EQUAL(h->value, 1);
  ProductID notpresent{};  // Need to make distinction about invalid vs. not present?
  BOOST_REQUIRE(!currentEvent_->get(notpresent, h));
  BOOST_REQUIRE(!h.isValid());
  BOOST_REQUIRE(h.failedToGet());
  BOOST_REQUIRE_THROW(*h, cet::exception);
}

BOOST_AUTO_TEST_CASE(transaction)
{
  // Put a product into an Event, and make sure that if we don't
  // commit, there is no product in the EventPrincipal afterwards.
  BOOST_REQUIRE_EQUAL(principal_->size(), 6u);
  {
    auto three = make_unique<arttest::IntProduct>(3);
    currentEvent_->put(move(three), "int1");
    BOOST_REQUIRE_EQUAL(principal_->size(), 6u);
    BOOST_REQUIRE_EQUAL(currentEvent_->size(), 7u);
    // DO NOT COMMIT!
  }
  // The Event has been destroyed without a commit -- we should not
  // have any products in the EventPrincipal.
  BOOST_REQUIRE_EQUAL(principal_->size(), 6u);
}

BOOST_AUTO_TEST_CASE(getByInstanceName)
{
  using product_t  = arttest::IntProduct;
  using handle_t   = Handle<product_t>;
  using handle_vec = vector<handle_t>;
  auto one   = make_unique<product_t>(1);
  auto two   = make_unique<product_t>(2);
  auto three = make_unique<product_t>(3);
  auto four  = make_unique<product_t>(4);
  addProduct(move(one),   "int1_tag", "int1");
  addProduct(move(two),   "int2_tag", "int2");
  addProduct(move(three), "int3_tag");
  addProduct(move(four),  "nolabel_tag");
  BOOST_REQUIRE_EQUAL(currentEvent_->size(), 6u);
  Selector sel(ProductInstanceNameSelector("int2") &&
               ModuleLabelSelector("modMulti"));
  handle_t h;
  BOOST_REQUIRE(currentEvent_->get(sel, h));
  BOOST_REQUIRE_EQUAL(h->value, 2);
  Selector sel2(ProductInstanceNameSelector("int2") ||
                ProductInstanceNameSelector("int1"));
  handle_vec handles;
  currentEvent_->getMany(sel2, handles);
  BOOST_REQUIRE_EQUAL(handles.size(), size_t(2));
  string instance;
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
  vector<Handle<int>> nomatches;
  currentEvent_->getMany(ModuleLabelSelector("modMulti"), nomatches);
  BOOST_REQUIRE(nomatches.empty());
}

BOOST_AUTO_TEST_CASE(getBySelector)
{
  using product_t  = arttest::IntProduct;
  using handle_t   = Handle<product_t>;
  using handle_vec = vector<handle_t>;
  auto one   = make_unique<product_t>(1);
  auto two   = make_unique<product_t>(2);
  auto three = make_unique<product_t>(3);
  auto four  = make_unique<product_t>(4);
  addProduct(move(one),   "int1_tag", "int1");
  addProduct(move(two),   "int2_tag", "int2");
  addProduct(move(three), "int3_tag");
  addProduct(move(four),  "nolabel_tag");
  //  unique_ptr<vector<arttest::Thing> > ap_vthing(new vector<arttest::Thing>);
  //  addProduct(ap_vthing, "thing", "");
  auto oneHundred = make_unique<product_t>(100);
  addProduct(move(oneHundred), "int1_tag_late", "int1");
  auto twoHundred = make_unique<product_t>(200);
  currentEvent_->put(move(twoHundred), "int1");
  EDProducer::commitEvent(*principal_, *currentEvent_);
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
  for (int k = 0; k < 5; ++k) {
    sum += handles[k]->value;
  }
  BOOST_REQUIRE_EQUAL(sum, 306);
}

BOOST_AUTO_TEST_CASE(getByLabel)
{
  using product_t  = arttest::IntProduct;
  using handle_t   = Handle<product_t>;
  using handle_vec = vector<handle_t>;
  auto one   = make_unique<product_t>(1);
  auto two   = make_unique<product_t>(2);
  auto three = make_unique<product_t>(3);
  auto four  = make_unique<product_t>(4);
  addProduct(move(one),   "int1_tag", "int1");
  addProduct(move(two),   "int2_tag", "int2");
  addProduct(move(three), "int3_tag");
  addProduct(move(four),  "nolabel_tag");
  auto oneHundred = make_unique<product_t>(100);
  addProduct(move(oneHundred), "int1_tag_late", "int1");
  auto twoHundred = make_unique<product_t>(200);
  currentEvent_->put(move(twoHundred), "int1");
  EDProducer::commitEvent(*principal_, *currentEvent_);
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
  using handle_vec = vector<handle_t>;
  auto one   = make_unique<product_t>(1);
  auto two   = make_unique<product_t>(2);
  auto three = make_unique<product_t>(3);
  auto four  = make_unique<product_t>(4);
  addProduct(move(one),   "int1_tag", "int1");
  addProduct(move(two),   "int2_tag", "int2");
  addProduct(move(three), "int3_tag");
  addProduct(move(four),  "nolabel_tag");
  auto oneHundred = make_unique<product_t>(100);
  addProduct(move(oneHundred), "int1_tag_late", "int1");
  auto twoHundred = make_unique<product_t>(200);
  currentEvent_->put(move(twoHundred), "int1");
  EDProducer::commitEvent(*principal_, *currentEvent_);
  BOOST_REQUIRE_EQUAL(currentEvent_->size(), 6u);
  handle_vec handles;
  currentEvent_->getManyByType(handles);
  BOOST_REQUIRE_EQUAL(handles.size(), 6u);
  int sum = 0;
  for (int k = 0; k < 6; ++k) {
    sum += handles[k]->value;
  }
  BOOST_REQUIRE_EQUAL(sum, 310);
}

BOOST_AUTO_TEST_CASE(printHistory)
{
  ProcessHistory const& history = currentEvent_->processHistory();
  ofstream out("history.log");
  cet::copy_all(history, ostream_iterator<ProcessHistory::const_iterator::value_type>(out, "\n"));
}

BOOST_AUTO_TEST_SUITE_END()
