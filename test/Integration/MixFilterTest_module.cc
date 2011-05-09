#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Modules/MixFilter.h"
#include "art/Framework/IO/ProductMix/MixHelper.h"
#include "art/Framework/Core/PtrRemapper.h"
#include "art/Persistency/Common/CollectionUtilities.h"
#include "art/Persistency/Common/PtrVector.h"
#include "art/Utilities/InputTag.h"
#include "test/TestObjects/ToyProducts.h"

namespace arttest {
  class MixFilterTestDetail;
  typedef art::MixFilter<MixFilterTestDetail> MixFilterTest;
  void ff_mix(std::vector<int const *> const &in,
                int &out,
                art::PtrRemapper const &remap);
}

class arttest::MixFilterTestDetail {
public:
  // Constructor is resposible for registering mix operations with
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
  void
  mixByAddition(std::vector<T const *> const &,
                T &,
                art::PtrRemapper const &);

  void
  aggregateCollection(std::vector<std::vector<double> const *> const &in,
                      std::vector<double> &out,
                      art::PtrRemapper const &);

  void
  mixPtrs(std::vector<std::vector<art::Ptr<double> > const *> const &in,
          std::vector<art::Ptr<double> > &out,
          art::PtrRemapper const &remap);

  void
  mixPtrVectors(std::vector<art::PtrVector<double> const *> const &in,
                 art::PtrVector<double> &out,
                 art::PtrRemapper const &remap);

private:
  size_t nSecondaries_;
  bool testRemapper_;
  std::vector<size_t> doubleVectorOffsets_;
};

arttest::MixFilterTestDetail::
MixFilterTestDetail(fhicl::ParameterSet const &p,
                      art::MixHelper &helper)
  :
  nSecondaries_(p.get<size_t>("nunSecondaries", 1)),
  testRemapper_(p.get<bool>("testRemapper", 1))
{
  helper.produces<std::string>(); // "Bookkeeping"

  helper.declareMixOp
    (art::InputTag("doubleLabel", ""),
     &MixFilterTestDetail::mixByAddition<double>, this);

  helper.declareMixOp
    (art::InputTag("IntProductLabel", ""),
     &MixFilterTestDetail::mixByAddition<arttest::IntProduct>, this);

  helper.declareMixOp
    (art::InputTag("stringLabel", "SWRITE"),
     &MixFilterTestDetail::mixByAddition<std::string>, this);

  helper.declareMixOp
    (art::InputTag("doubleCollectionLabel", ""),
     &MixFilterTestDetail::aggregateCollection, this);

  helper.declareMixOp
    (art::InputTag("doubleVectorPtrLabel", ""),
     &MixFilterTestDetail::mixPtrs, this);

  helper.declareMixOp
    (art::InputTag("doublePtrVectorLabel", ""),
     &MixFilterTestDetail::mixPtrVectors, this);
}

void
arttest::MixFilterTestDetail::
startEvent() {
  // FIXME: placeholder.
}

size_t
arttest::MixFilterTestDetail::
nSecondaries() const {
  return nSecondaries_;
}

void
arttest::MixFilterTestDetail::
processEventIDs(art::EventIDSequence const &seq) {
  // FIXME: placeholder.
}

void
arttest::MixFilterTestDetail::
finalizeEvent(art::Event &e) {
  e.put(std::auto_ptr<std::string>(new std::string("BlahBlahBlah")));
}

template<typename T>
void
mixByAddition(std::vector<T const *> const &in,
              T &out,
              art::PtrRemapper const &) {
  for (typename std::vector<T const *>::const_iterator
         i = in.begin(),
         e = in.end();
       i != e;
       ++i) {
    out += *i;
  }
}

void
arttest::MixFilterTestDetail::
aggregateCollection(std::vector<std::vector<double> const *> const &in,
                    std::vector<double> &out,
                    art::PtrRemapper const &) {
  art::flattenCollections(in, out, doubleVectorOffsets_);
}

namespace {
  namespace n1 {
    typedef typename std::vector<art::Ptr<double> > PROD;
    typedef typename PROD::const_iterator InIter;
    typedef typename std::pair<InIter, InIter> InIterPair;

    InIterPair f(PROD const* p) { return InIterPair(p->begin(), p->end()); }
  }

  namespace n2 {
    typedef typename art::PtrVector<double> PROD;
    typedef typename PROD::const_iterator InIter;
    typedef typename std::pair<InIter, InIter> InIterPair;

    InIterPair f(PROD const* p) { return InIterPair(p->begin(), p->end()); }
  }
}

void
arttest::MixFilterTestDetail::
mixPtrs(std::vector<std::vector<art::Ptr<double> > const *> const &in,
        std::vector<art::Ptr<double> > &out,
        art::PtrRemapper const &remap) {
  using namespace ::n1;
  std::function<InIterPair (PROD const *)> foff(f);
  remap(in,
        std::back_inserter(out),
        doubleVectorOffsets_,
        foff);
}

void
arttest::MixFilterTestDetail::
mixPtrVectors(std::vector<art::PtrVector<double> const *> const &in,
              art::PtrVector<double> &out,
              art::PtrRemapper const &remap) {
  using namespace ::n2;
  std::function<InIterPair (PROD const *)> foff(f);
  remap(in,
        std::back_inserter(out),
        doubleVectorOffsets_,
        foff);
}

DEFINE_ART_MODULE(arttest::MixFilterTest);

