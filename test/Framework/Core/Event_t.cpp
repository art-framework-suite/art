
/*----------------------------------------------------------------------

Test program for art::Event.

----------------------------------------------------------------------*/
#include "test/CppUnit_testdriver.icpp"
#include <cppunit/extensions/HelperMacros.h>

#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/EventPrincipal.h"
#include "art/Framework/Core/RootDictionaryManager.h"
#include "art/Framework/Core/RunPrincipal.h"
#include "art/Framework/Core/Selector.h"
#include "art/Framework/Core/SubRunPrincipal.h"
#include "art/Persistency/Common/Handle.h"
#include "art/Persistency/Common/OrphanHandle.h"
#include "art/Persistency/Common/Wrapper.h"
#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/BranchIDListHelper.h"
#include "art/Persistency/Provenance/BranchIDListHelper.h"
#include "art/Persistency/Provenance/EventAuxiliary.h"
#include "art/Persistency/Provenance/EventID.h"
#include "art/Persistency/Provenance/History.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/ProcessHistory.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "art/Persistency/Provenance/ProductRegistry.h"
#include "art/Persistency/Provenance/RunAuxiliary.h"
#include "art/Persistency/Provenance/SubRunAuxiliary.h"
#include "art/Persistency/Provenance/Timestamp.h"
#include "art/Utilities/GetPassID.h"
#include "art/Utilities/GlobalIdentifier.h"
#include "art/Utilities/InputTag.h"
#include "art/Version/GetReleaseVersion.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/ParameterSet.h"
// #include "test/TestObjects/Thing.h"
#include "test/TestObjects/ToyProducts.h"
// #include "art/Utilities/EDMException.h"

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

// This is a gross hack, to allow us to test the event
namespace art
{
  class EDProducer
  {
  public:
    static void commitEvent(Event& e) { e.commit_(); }

  };
}


class testEvent: public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(testEvent);
  CPPUNIT_TEST(emptyEvent);
  CPPUNIT_TEST(getBySelectorFromEmpty);
  CPPUNIT_TEST(putAnIntProduct);
  CPPUNIT_TEST(putAndGetAnIntProduct);
  CPPUNIT_TEST(getByProductID);
  CPPUNIT_TEST(transaction);
  CPPUNIT_TEST(getByInstanceName);
  CPPUNIT_TEST(getBySelector);
  CPPUNIT_TEST(getByLabel);
  CPPUNIT_TEST(getByType);
  CPPUNIT_TEST(printHistory);
  CPPUNIT_TEST_SUITE_END();

 public:
  testEvent();
  ~testEvent();
  void setUp();
  void tearDown();
  void emptyEvent();
  void getBySelectorFromEmpty();
  void putAnIntProduct();
  void putAndGetAnIntProduct();
  void getByProductID();
  void transaction();
  void getByInstanceName();
  void getBySelector();
  void getByLabel();
  void getByType();
  void printHistory();

 private:

  template <class T>
  void registerProduct(std::string const& tag,
		       std::string const& moduleLabel,
		       std::string const& moduleClassName,
		       std::string const& processName,
		       std::string const& productInstanceName);

  template <class T>
  void registerProduct(std::string const& tag,
		       std::string const& moduleLabel,
		       std::string const& moduleClassName,
		       std::string const& processName)
  {
    std::string productInstanceName;
    registerProduct<T>(tag, moduleLabel, moduleClassName, processName,
		       productInstanceName);
  }

  template <class T>
  ProductID addProduct(std::auto_ptr<T> product,
		       std::string const& tag,
		       std::string const& productLabel = std::string());

  ProductRegistry*   availableProducts_;
  EventPrincipal*    principal_;
  Event*             currentEvent_;
  ModuleDescription* currentModuleDescription_;
  typedef std::map<std::string, ModuleDescription> modCache_t;
  typedef modCache_t::iterator iterator_t;

  modCache_t moduleDescriptions_;
  art::RootDictionaryManager rdm_;
};

///registration of the test so that the runner can find it
CPPUNIT_TEST_SUITE_REGISTRATION(testEvent);

namespace
{
  template <class T> void kill_and_clear(T*& p) { delete p; p = 0; }
}

EventID   make_id() { return EventID(2112, 47, 25); }
Timestamp make_timestamp() { return Timestamp(1); }

template <class T>
void
testEvent::registerProduct(std::string const& tag,
			   std::string const& moduleLabel,
 			   std::string const& moduleClassName,
 			   std::string const& processName,
			   std::string const& productInstanceName)
{
  if (!availableProducts_)
    availableProducts_ = new ProductRegistry();

  ParameterSet moduleParams;
  moduleParams.template put<std::string>("module_type", moduleClassName);
  moduleParams.template put<std::string>("module_label", moduleLabel);

  ParameterSet processParams;
  processParams.template put<std::string>("process_name", processName);
  processParams.template put<ParameterSet>(moduleLabel, moduleParams);

  ProcessConfiguration process;
  process.processName_    = processName;
  process.releaseVersion_ = getReleaseVersion();
  process.passID_         = getPassID();
  process.parameterSetID_ = processParams.id();

  ModuleDescription localModuleDescription;
  localModuleDescription.parameterSetID_       = moduleParams.id();
  localModuleDescription.moduleName_           = moduleClassName;
  localModuleDescription.moduleLabel_          = moduleLabel;
  localModuleDescription.processConfiguration_ = process;

  TypeID product_type(typeid(T));

  BranchDescription branch(InEvent,
			   moduleLabel,
			   processName,
			   product_type.userClassName(),
			   product_type.friendlyClassName(),
			   productInstanceName,
			   localModuleDescription
			  );

  moduleDescriptions_[tag] = localModuleDescription;
  availableProducts_->addProduct(branch);
}

// Add the given product, of type T, to the current event,
// as if it came from the module specified by the given tag.
template <class T>
ProductID
testEvent::addProduct(std::auto_ptr<T> product,
		      std::string const& tag,
		      std::string const& productLabel)
{
  iterator_t description = moduleDescriptions_.find(tag);
  if (description == moduleDescriptions_.end())
    throw art::Exception(errors::LogicError)
      << "Failed to find a module description for tag: "
      << tag << '\n';

  Event temporaryEvent(*principal_, description->second);
  OrphanHandle<T> h = temporaryEvent.put(product, productLabel);
  ProductID id = h.id();
  EDProducer::commitEvent(temporaryEvent);
  return id;
}

testEvent::testEvent() :
  availableProducts_(new ProductRegistry()),
  principal_(0),
  currentEvent_(0),
  currentModuleDescription_(0),
  moduleDescriptions_()
{
  BranchIDListHelper::clearRegistries();

  typedef arttest::IntProduct prod_t;
  //  typedef std::vector<arttest::Thing> vec_t;

  registerProduct<prod_t>("nolabel_tag",   "modOne",   "IntProducer",   "EARLY");
  registerProduct<prod_t>("int1_tag",      "modMulti", "IntProducer",   "EARLY", "int1");
  registerProduct<prod_t>("int1_tag_late", "modMulti", "IntProducer",   "LATE",  "int1");
  registerProduct<prod_t>("int2_tag",      "modMulti", "IntProducer",   "EARLY", "int2");
  registerProduct<prod_t>("int3_tag",      "modMulti", "IntProducer",   "EARLY");
  //  registerProduct<vec_t>("thing",          "modthing", "ThingProducer", "LATE");
  //  registerProduct<vec_t>("thing2",         "modthing", "ThingProducer", "LATE",  "inst2");

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

  currentModuleDescription_ = new ModuleDescription();
  currentModuleDescription_->parameterSetID_       = moduleParams.id();
  currentModuleDescription_->moduleName_           = moduleClassName;
  currentModuleDescription_->moduleLabel_          = moduleLabel;
  currentModuleDescription_->processConfiguration_ = process;

  std::string productInstanceName("int1");

  BranchDescription branch(InEvent,
			   moduleLabel,
			   processName,
			   product_type.userClassName(),
			   product_type.friendlyClassName(),
			   productInstanceName,
			   *currentModuleDescription_
			  );

  availableProducts_->addProduct(branch);

  // Freeze the product registry before we make the Event.
  availableProducts_->setFrozen();
  BranchIDListHelper::updateRegistries(*availableProducts_);
}

testEvent::~testEvent()
{
  delete principal_;
  delete currentEvent_;
  delete currentModuleDescription_;
}

void testEvent::setUp()
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

  ProcessHistoryRegistry::put(ph);

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

  boost::shared_ptr<ProductRegistry const> preg(availableProducts_);
  std::string uuid = createGlobalIdentifier();
  Timestamp time = make_timestamp();
  EventID id = make_id();
  ProcessConfiguration const& pc = currentModuleDescription_->processConfiguration();
  RunAuxiliary runAux(id.run(), time, time);
  boost::shared_ptr<RunPrincipal> rp(new RunPrincipal(runAux, preg, pc));
  SubRunAuxiliary subRunAux(rp->run(), 1, time, time);
  boost::shared_ptr<SubRunPrincipal>srp(new SubRunPrincipal(subRunAux, preg, pc));
  srp->setRunPrincipal(rp);
  EventAuxiliary eventAux(id, uuid, time, true);
  boost::shared_ptr<History> history(new History);
  const_cast<ProcessHistoryID &>(history->processHistoryID()) = processHistoryID;
  principal_  = new EventPrincipal(eventAux,
				   preg,
                                   pc,
                                   history);

  principal_->setSubRunPrincipal(srp);
  currentEvent_ = new Event(*principal_, *currentModuleDescription_);

  delete processHistory;
}

void
testEvent::tearDown()
{
  kill_and_clear(currentEvent_);
  kill_and_clear(principal_);
}

void testEvent::emptyEvent()

{
  CPPUNIT_ASSERT(currentEvent_);
  CPPUNIT_ASSERT(currentEvent_->id() == make_id());
  CPPUNIT_ASSERT(currentEvent_->time() == make_timestamp());
  CPPUNIT_ASSERT(currentEvent_->size() == 0);
}

void testEvent::getBySelectorFromEmpty()
{
  ModuleLabelSelector byModuleLabel("mod1");
  Handle<int> nonesuch;
  CPPUNIT_ASSERT(!nonesuch.isValid());
  CPPUNIT_ASSERT(!currentEvent_->get(byModuleLabel, nonesuch));
  CPPUNIT_ASSERT(!nonesuch.isValid());
  CPPUNIT_ASSERT(nonesuch.failedToGet());
  CPPUNIT_ASSERT_THROW(*nonesuch,
		       cet::exception);
}

void testEvent::putAnIntProduct()
{
  std::auto_ptr<arttest::IntProduct> three(new arttest::IntProduct(3));
  currentEvent_->put(three, "int1");
  CPPUNIT_ASSERT(currentEvent_->size() == 1);
  EDProducer::commitEvent(*currentEvent_);
  CPPUNIT_ASSERT(currentEvent_->size() == 1);
}

void testEvent::putAndGetAnIntProduct()
{
  std::auto_ptr<arttest::IntProduct> four(new arttest::IntProduct(4));
  currentEvent_->put(four, "int1");
  EDProducer::commitEvent(*currentEvent_);

  ProcessNameSelector should_match("CURRENT");
  ProcessNameSelector should_not_match("NONESUCH");
  Handle<arttest::IntProduct> h;
  currentEvent_->get(should_match, h);
  CPPUNIT_ASSERT(h.isValid());
  CPPUNIT_ASSERT(!currentEvent_->get(should_not_match, h));
  CPPUNIT_ASSERT(!h.isValid());
  CPPUNIT_ASSERT_THROW(*h, cet::exception);
}

void testEvent::getByProductID()
{

  typedef arttest::IntProduct product_t;
  typedef std::auto_ptr<product_t> ap_t;
  typedef Handle<product_t>  handle_t;

  ProductID wanted;

  {
    ap_t one(new product_t(1));
    ProductID id1 = addProduct(one, "int1_tag", "int1");
    CPPUNIT_ASSERT(id1 != ProductID());
    wanted = id1;

    ap_t two(new product_t(2));
    ProductID id2 = addProduct(two, "int2_tag", "int2");
    CPPUNIT_ASSERT(id2 != ProductID());
    CPPUNIT_ASSERT(id2 != id1);

    EDProducer::commitEvent(*currentEvent_);
    CPPUNIT_ASSERT(currentEvent_->size() == 2);
  }

  handle_t h;
  currentEvent_->get(wanted, h);
  CPPUNIT_ASSERT(h.isValid());
  CPPUNIT_ASSERT(h.id() == wanted);
  CPPUNIT_ASSERT(h->value == 1);

  ProductID invalid;
  CPPUNIT_ASSERT_THROW(currentEvent_->get(invalid, h), cet::exception);
  CPPUNIT_ASSERT(!h.isValid());
  ProductID notpresent(0, std::numeric_limits<unsigned short>::max());
  CPPUNIT_ASSERT(!currentEvent_->get(notpresent, h));
  CPPUNIT_ASSERT(!h.isValid());
  CPPUNIT_ASSERT(h.failedToGet());
  CPPUNIT_ASSERT_THROW(*h, cet::exception);
}

void testEvent::transaction()
{
  // Put a product into an Event, and make sure that if we don't
  // commit, there is no product in the EventPrincipal afterwards.
  CPPUNIT_ASSERT(principal_->size() == 0);
  {
    typedef arttest::IntProduct product_t;
    typedef std::auto_ptr<product_t> ap_t;

    ap_t three(new product_t(3));
    currentEvent_->put(three, "int1");
    CPPUNIT_ASSERT(principal_->size() == 0);
    CPPUNIT_ASSERT(currentEvent_->size() == 1);
    // DO NOT COMMIT!
  }

  // The Event has been destroyed without a commit -- we should not
  // have any products in the EventPrincipal.
  CPPUNIT_ASSERT(principal_->size() == 0);
}

void testEvent::getByInstanceName()
{
  typedef arttest::IntProduct product_t;
  typedef std::auto_ptr<product_t> ap_t;
  typedef Handle<product_t> handle_t;
  typedef std::vector<handle_t> handle_vec;

  ap_t one(new product_t(1));
  ap_t two(new product_t(2));
  ap_t three(new product_t(3));
  ap_t four(new product_t(4));
  addProduct(one,   "int1_tag", "int1");
  addProduct(two,   "int2_tag", "int2");
  addProduct(three, "int3_tag");
  addProduct(four,  "nolabel_tag");

  CPPUNIT_ASSERT(currentEvent_->size() == 4);

  Selector sel(ProductInstanceNameSelector("int2") &&
	       ModuleLabelSelector("modMulti"));;
  handle_t h;
  CPPUNIT_ASSERT(currentEvent_->get(sel, h));
  CPPUNIT_ASSERT(h->value == 2);

  std::string instance;
  Selector sel1(ProductInstanceNameSelector(instance) &&
	       ModuleLabelSelector("modMulti"));;

  CPPUNIT_ASSERT(currentEvent_->get(sel1, h));
  CPPUNIT_ASSERT(h->value == 3);

  handle_vec handles;
  currentEvent_->getMany(ModuleLabelSelector("modMulti"), handles);
  CPPUNIT_ASSERT(handles.size() == 3);
  handles.clear();
  currentEvent_->getMany(ModuleLabelSelector("nomatch"), handles);
  CPPUNIT_ASSERT(handles.empty());
  std::vector<Handle<int> > nomatches;
  currentEvent_->getMany(ModuleLabelSelector("modMulti"), nomatches);
  CPPUNIT_ASSERT(nomatches.empty());
}

void testEvent::getBySelector()
{
  typedef arttest::IntProduct product_t;
  typedef std::auto_ptr<product_t> ap_t;
  typedef Handle<product_t> handle_t;
  typedef std::vector<handle_t> handle_vec;

  ap_t one(new product_t(1));
  ap_t two(new product_t(2));
  ap_t three(new product_t(3));
  ap_t four(new product_t(4));
  addProduct(one,   "int1_tag", "int1");
  addProduct(two,   "int2_tag", "int2");
  addProduct(three, "int3_tag");
  addProduct(four,  "nolabel_tag");

  //  std::auto_ptr<std::vector<arttest::Thing> > ap_vthing(new std::vector<arttest::Thing>);
  //  addProduct(ap_vthing, "thing", "");

  ap_t oneHundred(new product_t(100));
  addProduct(oneHundred, "int1_tag_late", "int1");

  std::auto_ptr<arttest::IntProduct> twoHundred(new arttest::IntProduct(200));
  currentEvent_->put(twoHundred, "int1");
  EDProducer::commitEvent(*currentEvent_);

  CPPUNIT_ASSERT(currentEvent_->size() == 6);

  Selector sel(ProductInstanceNameSelector("int2") &&
	       ModuleLabelSelector("modMulti") &&
               ProcessNameSelector("EARLY"));;
  handle_t h;
  CPPUNIT_ASSERT(currentEvent_->get(sel, h));
  CPPUNIT_ASSERT(h->value == 2);

  Selector sel1(ProductInstanceNameSelector("nomatch") &&
	        ModuleLabelSelector("modMulti") &&
                ProcessNameSelector("EARLY"));
  CPPUNIT_ASSERT(!currentEvent_->get(sel1, h));
  CPPUNIT_ASSERT(!h.isValid());
  CPPUNIT_ASSERT_THROW(*h, cet::exception);

  Selector sel2(ProductInstanceNameSelector("int2") &&
	        ModuleLabelSelector("nomatch") &&
                ProcessNameSelector("EARLY"));
  CPPUNIT_ASSERT(!currentEvent_->get(sel2, h));
  CPPUNIT_ASSERT(!h.isValid());
  CPPUNIT_ASSERT_THROW(*h, cet::exception);

  Selector sel3(ProductInstanceNameSelector("int2") &&
	        ModuleLabelSelector("modMulti") &&
                ProcessNameSelector("nomatch"));
  CPPUNIT_ASSERT(!currentEvent_->get(sel3, h));
  CPPUNIT_ASSERT(!h.isValid());
  CPPUNIT_ASSERT_THROW(*h, cet::exception);

  Selector sel4(ModuleLabelSelector("modMulti") &&
                ProcessNameSelector("EARLY"));
  //multiple selections throw
  CPPUNIT_ASSERT_THROW(currentEvent_->get(sel4, h), cet::exception);

  Selector sel5(ModuleLabelSelector("modMulti") &&
                ProcessNameSelector("LATE"));;
  currentEvent_->get(sel5, h);
  CPPUNIT_ASSERT(h->value == 100);

  Selector sel6(ModuleLabelSelector("modMulti") &&
                ProcessNameSelector("CURRENT"));;
  currentEvent_->get(sel6, h);
  CPPUNIT_ASSERT(h->value == 200);

  Selector sel7(ModuleLabelSelector("modMulti"));;
  currentEvent_->get(sel7, h);
  CPPUNIT_ASSERT(h->value == 200);

  handle_vec handles;
  currentEvent_->getMany(ModuleLabelSelector("modMulti"), handles);
  CPPUNIT_ASSERT(handles.size() == 5);
  int sum = 0;
  for (int k = 0; k < 5; ++k) sum += handles[k]->value;
  CPPUNIT_ASSERT(sum == 306);
}

void testEvent::getByLabel()
{
  typedef arttest::IntProduct product_t;
  typedef std::auto_ptr<product_t> ap_t;
  typedef Handle<product_t> handle_t;
  typedef std::vector<handle_t> handle_vec;

  ap_t one(new product_t(1));
  ap_t two(new product_t(2));
  ap_t three(new product_t(3));
  ap_t four(new product_t(4));
  addProduct(one,   "int1_tag", "int1");
  addProduct(two,   "int2_tag", "int2");
  addProduct(three, "int3_tag");
  addProduct(four,  "nolabel_tag");

  //  std::auto_ptr<std::vector<arttest::Thing> > ap_vthing(new std::vector<arttest::Thing>);
  //  addProduct(ap_vthing, "thing", "");

  ap_t oneHundred(new product_t(100));
  addProduct(oneHundred, "int1_tag_late", "int1");

  std::auto_ptr<arttest::IntProduct> twoHundred(new arttest::IntProduct(200));
  currentEvent_->put(twoHundred, "int1");
  EDProducer::commitEvent(*currentEvent_);

  CPPUNIT_ASSERT(currentEvent_->size() == 6);

  handle_t h;
  CPPUNIT_ASSERT(currentEvent_->getByLabel("modMulti", h));
  CPPUNIT_ASSERT(h->value == 3);

  CPPUNIT_ASSERT(currentEvent_->getByLabel("modMulti", "int1", h));
  CPPUNIT_ASSERT(h->value == 200);

  CPPUNIT_ASSERT(!currentEvent_->getByLabel("modMulti", "nomatch", h));
  CPPUNIT_ASSERT(!h.isValid());
  CPPUNIT_ASSERT_THROW(*h, cet::exception);

  InputTag inputTag("modMulti", "int1");
  CPPUNIT_ASSERT(currentEvent_->getByLabel(inputTag, h));
  CPPUNIT_ASSERT(h->value == 200);

  BasicHandle bh =
    principal_->getByLabel(TypeID(typeid(arttest::IntProduct)), "modMulti", "int1", "LATE");
  convert_handle(bh, h);
  CPPUNIT_ASSERT(h->value == 100);
  BasicHandle bh2(principal_->getByLabel(TypeID(typeid(arttest::IntProduct)), "modMulti", "int1", "nomatch"));
  CPPUNIT_ASSERT(!bh2.isValid());

  boost::shared_ptr<Wrapper<arttest::IntProduct> const> ptr = getProductByTag<arttest::IntProduct>(*principal_, inputTag);
  CPPUNIT_ASSERT(ptr->product()->value == 200);
}


void testEvent::getByType()
{
  typedef arttest::IntProduct product_t;
  typedef std::auto_ptr<product_t> ap_t;
  typedef Handle<product_t> handle_t;
  typedef std::vector<handle_t> handle_vec;

  ap_t one(new product_t(1));
  ap_t two(new product_t(2));
  ap_t three(new product_t(3));
  ap_t four(new product_t(4));
  addProduct(one,   "int1_tag", "int1");
  addProduct(two,   "int2_tag", "int2");
  addProduct(three, "int3_tag");
  addProduct(four,  "nolabel_tag");

  //  std::auto_ptr<std::vector<arttest::Thing> > ap_vthing(new std::vector<arttest::Thing>);
  //  addProduct(ap_vthing, "thing", "");

  //  std::auto_ptr<std::vector<arttest::Thing> > ap_vthing2(new std::vector<arttest::Thing>);
  //  addProduct(ap_vthing2, "thing2", "inst2");

  ap_t oneHundred(new product_t(100));
  addProduct(oneHundred, "int1_tag_late", "int1");

  std::auto_ptr<arttest::IntProduct> twoHundred(new arttest::IntProduct(200));
  currentEvent_->put(twoHundred, "int1");
  EDProducer::commitEvent(*currentEvent_);

  CPPUNIT_ASSERT(currentEvent_->size() == 6);

  handle_t h;
  CPPUNIT_ASSERT(currentEvent_->getByType(h));
  CPPUNIT_ASSERT(h->value == 200);

  Handle<int> h_nomatch;
  CPPUNIT_ASSERT(!currentEvent_->getByType(h_nomatch));
  CPPUNIT_ASSERT(!h_nomatch.isValid());
  CPPUNIT_ASSERT(h_nomatch.failedToGet());
  CPPUNIT_ASSERT_THROW(*h_nomatch, cet::exception);

  //  Handle<std::vector<arttest::Thing> > hthing;
  //  CPPUNIT_ASSERT_THROW(currentEvent_->getByType(hthing),cet::exception);

  handle_vec handles;
  currentEvent_->getManyByType(handles);
  CPPUNIT_ASSERT(handles.size() == 6);
  int sum = 0;
  for (int k = 0; k < 6; ++k) sum += handles[k]->value;
  CPPUNIT_ASSERT(sum == 310);
}

void testEvent::printHistory()
{
  ProcessHistory const& history = currentEvent_->processHistory();
  std::ofstream out("history.log");

  cet::copy_all(history, std::ostream_iterator<ProcessHistory::const_iterator::value_type>(out, "\n"));
}
