#include "art/Persistency/Common/Wrapper.h"

#include "CLHEP/Matrix/GenMatrix.h"
#include "CLHEP/Matrix/Matrix.h"
#include "CLHEP/Matrix/SymMatrix.h"
#include "CLHEP/Matrix/Vector.h"
#include "CLHEP/Vector/LorentzVector.h"
#include "CLHEP/Vector/ThreeVector.h"
#include "CLHEP/Vector/TwoVector.h"

#include <vector>
namespace {
   struct dictionary {
     std::vector<CLHEP::Hep3Vector> h3v;
     art::Wrapper<std::vector<CLHEP::Hep3Vector> > h3v_w;
     std::vector<CLHEP::HepLorentzVector> hlv;
     art::Wrapper<std::vector<CLHEP::HepLorentzVector> > hlv_w;
     art::Wrapper<CLHEP::Hep2Vector> w01;
     art::Wrapper<CLHEP::Hep3Vector> w02;
     art::Wrapper<CLHEP::HepMatrix> w03;
     art::Wrapper<CLHEP::HepLorentzVector> w04;
     art::Wrapper<CLHEP::HepSymMatrix> w05;
     art::Wrapper<CLHEP::HepVector> w06;
   };
}
