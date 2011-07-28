#include "art/Framework/Principal/Event.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Modules/MixFilter.h"
#include "art/Framework/IO/ProductMix/MixHelper.h"
#include "art/Framework/Core/PtrRemapper.h"
#include "art/Persistency/Common/CollectionUtilities.h"
#include "art/Persistency/Common/PtrVector.h"
#include "cetlib/map_vector.h"
#include "cpp0x/memory"
#include "art/Utilities/InputTag.h"
#include "test/TestObjects/ProductWithPtrs.h"
#include "test/TestObjects/ToyProducts.h"

#include "boost/noncopyable.hpp"

namespace arttest {
  class MixFilterTestDetail;
  typedef art::MixFilter<MixFilterTestDetail> MixFilterTest;
}

class arttest::MixFilterTestDetail : private boost::noncopyable {
public:
  typedef cet::map_vector<unsigned int> mv_t;
  typedef typename mv_t::value_type mvv_t;
  typedef typename mv_t::mapped_type mvm_t;

  // Constructor is responsible for registering mix operations with
  // MixHelper::declareMixOp() and bookkeeping products with
  // MixHelperproduces().
  MixFilterTestDetail(fhicl::ParameterSet const &p,
                      art::MixHelper &helper);

  // Optional startEvent(): initialize state for each event,
  void startEvent();

  // Return the number of secondaries to read this time. Declare const
  // if you don't plan to change your class' state.
  size_t nSecondaries() const;

  // Optional processEventIDs(): after the generation of the event
  // sequence, this function will be called if it exists to provide the
  // sequence of EventIDs.
  void processEventIDs(art::EventIDSequence const &seq);

  // Optional.finalizeEvent(): (eg) put bookkeping products in event. Do
  // *not* place mix products into the event: this will already have
  // been done for you.
  void finalizeEvent(art::Event &t);

  // Mixing functions. Note that they do not *have* to be member
  // functions of this detail class: they may be member functions of a
  // completely unrelated class; free functions or function objects
  // provided they (or the function object's operator()) have the
  // expected signature.
  template <typename T>
  bool
  mixByAddition(std::vector<T const *> const &,
                T &,
                art::PtrRemapper const &);

  bool
  aggregateDoubleCollection(std::vector<std::vector<double> const *> const &in,
                      std::vector<double> &out,
                      art::PtrRemapper const &);

  bool
  aggregate_map_vector(std::vector<mv_t const *> const &in,
                       mv_t &out,
                       art::PtrRemapper const &);

  bool
  mixPtrs(std::vector<std::vector<art::Ptr<double> > const *> const &in,
          std::vector<art::Ptr<double> > &out,
          art::PtrRemapper const &remap);

#ifndef ART_NO_MIX_PTRVECTOR
  bool
  mixPtrVectors(std::vector<art::PtrVector<double> const *> const &in,
                 art::PtrVector<double> &out,
                 art::PtrRemapper const &remap);
#endif

  bool
  mixProductWithPtrs(std::vector<arttest::ProductWithPtrs const *> const &in,
                     arttest::ProductWithPtrs &out,
                     art::PtrRemapper const &remap);

  bool
  mixmap_vectorPtrs(std::vector<std::vector<art::Ptr<cet::map_vector<unsigned int>::value_type> > const *> const &in,
                    std::vector<art::Ptr<cet::map_vector<unsigned int>::value_type> > &out,
                    art::PtrRemapper const &remap);

  size_t nSecondaries_;
  bool testRemapper_;
  std::vector<size_t> doubleVectorOffsets_, map_vectorOffsets_;
  std::auto_ptr<art::EventIDSequence> eIDs_;
  bool startEvent_called_;
  bool processEventIDs_called_;
};

arttest::MixFilterTestDetail::
MixFilterTestDetail(fhicl::ParameterSet const &p,
                      art::MixHelper &helper)
  :
  nSecondaries_(p.get<size_t>("numSecondaries", 1)),
  testRemapper_(p.get<bool>("testRemapper", 1)),
  doubleVectorOffsets_(),
  map_vectorOffsets_(),
  eIDs_(),
  startEvent_called_(false),
  processEventIDs_called_(false)
{
  std::string mixProducerLabel(p.get<std::string>("mixProducerLabel",
                                                  "mixProducer"));

  helper.produces<std::string>(); // "Bookkeeping"
  helper.produces<art::EventIDSequence>(); // "Bookkeeping"

  helper.declareMixOp
    (art::InputTag(mixProducerLabel, "doubleLabel"),
     &MixFilterTestDetail::mixByAddition<double>, *this);

  helper.declareMixOp
    (art::InputTag(mixProducerLabel, "IntProductLabel"),
     &MixFilterTestDetail::mixByAddition<arttest::IntProduct>, *this);

  helper.declareMixOp
    (art::InputTag(mixProducerLabel, "stringLabel", "SWRITE"),
     &MixFilterTestDetail::mixByAddition<std::string>, *this);

  helper.declareMixOp
    (art::InputTag(mixProducerLabel, "doubleCollectionLabel"),
     &MixFilterTestDetail::aggregateDoubleCollection, *this);

  helper.declareMixOp
    (art::InputTag(mixProducerLabel, "doubleVectorPtrLabel"),
     &MixFilterTestDetail::mixPtrs, *this);

#ifndef ART_NO_MIX_PTRVECTOR
  helper.declareMixOp
    (art::InputTag(mixProducerLabel, "doublePtrVectorLabel"),
     &MixFilterTestDetail::mixPtrVectors, *this);
#endif

  helper.declareMixOp
    (art::InputTag(mixProducerLabel, "ProductWithPtrsLabel"),
     &MixFilterTestDetail::mixProductWithPtrs, *this);

  helper.declareMixOp
    (art::InputTag(mixProducerLabel, "mapVectorLabel"),
     &MixFilterTestDetail::aggregate_map_vector, *this);

  helper.declareMixOp
    (art::InputTag(mixProducerLabel, "intVectorPtrLabel"),
     &MixFilterTestDetail::mixmap_vectorPtrs, *this);

}

void
arttest::MixFilterTestDetail::
startEvent() {
  startEvent_called_ = true;
  eIDs_.reset();
}

size_t
arttest::MixFilterTestDetail::
nSecondaries() const {
  return nSecondaries_;
}

void
arttest::MixFilterTestDetail::
processEventIDs(art::EventIDSequence const &seq) {
  processEventIDs_called_ = true;
  eIDs_.reset(new art::EventIDSequence(seq));
}

void
arttest::MixFilterTestDetail::
finalizeEvent(art::Event &e) {
  e.put(std::auto_ptr<std::string>(new std::string("BlahBlahBlah")));
  e.put(eIDs_);

  assert(startEvent_called_);
  assert(processEventIDs_called_);
  startEvent_called_ = false;
  processEventIDs_called_ = false;
}

template<typename T>
bool
arttest::MixFilterTestDetail::
mixByAddition(std::vector<T const *> const &in,
              T &out,
              art::PtrRemapper const &) {
  for (typename std::vector<T const *>::const_iterator
         i = in.begin(),
         e = in.end();
       i != e;
       ++i) {
    out += **i;
  }
  return true; //  Always want product in event.
}

bool
arttest::MixFilterTestDetail::
aggregateDoubleCollection(std::vector<std::vector<double> const *> const &in,
                    std::vector<double> &out,
                    art::PtrRemapper const &) {
  art::flattenCollections(in, out, doubleVectorOffsets_);
  return true; //  Always want product in event.
}

bool
arttest::MixFilterTestDetail::
aggregate_map_vector(std::vector<mv_t const *> const &in,
                     mv_t &out,
                     art::PtrRemapper const &) {
  art::flattenCollections(in, out, map_vectorOffsets_);
  return true; //  Always want product in event.
}

bool
arttest::MixFilterTestDetail::
mixPtrs(std::vector<std::vector<art::Ptr<double> > const *> const &in,
        std::vector<art::Ptr<double> > &out,
        art::PtrRemapper const &remap) {
  remap(in,
        std::back_inserter(out),
        doubleVectorOffsets_);
  return true; //  Always want product in event.
}

#ifndef ART_NO_MIX_PTRVECTOR
bool
arttest::MixFilterTestDetail::
mixPtrVectors(std::vector<art::PtrVector<double> const *> const &in,
              art::PtrVector<double> &out,
              art::PtrRemapper const &remap) {
  remap(in,
        std::back_inserter(out),
        doubleVectorOffsets_);
  return true; //  Always want product in event.
}
#endif

bool
arttest::MixFilterTestDetail::
mixProductWithPtrs(std::vector<arttest::ProductWithPtrs const *> const &in,
                   arttest::ProductWithPtrs &out,
                   art::PtrRemapper const &remap) {
#ifndef ART_NO_MIX_PTRVECTOR
  remap(in,
        std::back_inserter(out.ptrVectorDouble()),
        doubleVectorOffsets_,
        &arttest::ProductWithPtrs::ptrVectorDouble);
#endif

  remap(in,
        std::back_inserter(out.vectorPtrDouble()),
        doubleVectorOffsets_,
        &arttest::ProductWithPtrs::vectorPtrDouble);

  // Throw-away object to test non-standard remap interface.
  arttest::ProductWithPtrs tmp;

  remap(in,
        std::back_inserter(tmp.vectorPtrDouble()),
        doubleVectorOffsets_,
        &arttest::ProductWithPtrs::vpd_);

  return true; //  Always want product in event.
}

bool
arttest::MixFilterTestDetail::
mixmap_vectorPtrs(std::vector<std::vector<art::Ptr<cet::map_vector<unsigned int>::value_type> > const *> const &in,
                  std::vector<art::Ptr<cet::map_vector<unsigned int>::value_type> > &out,
                  art::PtrRemapper const &remap) {
  remap(in,
        std::back_inserter(out),
        map_vectorOffsets_);

  return true; //  Always want product in event.
}

DEFINE_ART_MODULE(arttest::MixFilterTest);

