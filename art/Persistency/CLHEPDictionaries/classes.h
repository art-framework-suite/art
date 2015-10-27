#include "art/Persistency/Common/Wrapper.h"

#include "CLHEP/Matrix/GenMatrix.h"
#include "CLHEP/Matrix/Matrix.h"
#include "CLHEP/Matrix/SymMatrix.h"
#include "CLHEP/Matrix/Vector.h"
#include "CLHEP/Vector/LorentzVector.h"
#include "CLHEP/Vector/ThreeVector.h"
#include "CLHEP/Vector/TwoVector.h"

template class std::vector<CLHEP::Hep3Vector>;
template class art::Wrapper<std::vector<CLHEP::Hep3Vector>>;
template class std::vector<CLHEP::HepLorentzVector>;
template class art::Wrapper<std::vector<CLHEP::HepLorentzVector>>;
template class art::Wrapper<CLHEP::Hep2Vector>;
template class art::Wrapper<CLHEP::Hep3Vector>;
template class art::Wrapper<CLHEP::HepMatrix>;
template class art::Wrapper<CLHEP::HepLorentzVector>;
template class art::Wrapper<CLHEP::HepSymMatrix>;
template class art::Wrapper<CLHEP::HepVector>;

