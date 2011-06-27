#include "art/Persistency/Common/Ptr.h"
#include "art/Persistency/Common/PtrVector.h"
#include "art/Persistency/Common/Wrapper.h"

#include "test/TestObjects/MockCluster.h"
#include "test/TestObjects/ProductWithPtrs.h"
#include "test/TestObjects/ToyProducts.h"

namespace {
  struct dictionary {
    art::Wrapper<arttest::DummyProduct> dummyw12;
    art::Wrapper<arttest::IntProduct> dummyw13;
    art::Wrapper<arttest::Int16_tProduct> dummyw23;
    art::Wrapper<arttest::DoubleProduct> dummyw14;
    art::Wrapper<arttest::StringProduct> dummyw15;
    art::Wrapper<arttest::Prodigal> dummyw24;

    std::vector<arttest::SimpleDerived> dummy231;
    art::Wrapper<std::vector<arttest::SimpleDerived> > dummy231w;

    art::Ptr<arttest::SimpleDerived> dummy234;
    art::PtrVector<arttest::SimpleDerived> dummy235;
    art::Wrapper<art::PtrVector<arttest::SimpleDerived> > dummy235w;

    arttest::MockClusterList dummyMCL;
    art::Wrapper<arttest::MockClusterList> dummyMCLw;

    art::Wrapper<arttest::ProductWithPtrs> wpwp;
    std::vector<art::Ptr<double> > wpwp1;
    art::PtrVector<double> wpwp2;
    art::Wrapper<std::vector<art::Ptr<double> > > wpwp3;
    art::Wrapper<art::PtrVector<double> > wpwp4;
    art::Ptr<double> wpwp5;
  };
}
