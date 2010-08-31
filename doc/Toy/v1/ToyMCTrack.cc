#include "ToyMCTrack.hh"

ToyMCTrack::ToyMCTrack( int pdgid,
			double q[4],
			double pos[3] ):
  _pdgid(pdgid)
{
  _p[0] = q[0];
  _p[1] = q[1];
  _p[2] = q[2];
  _p[3] = q[3];
  _pos[0] = pos[0];
  _pos[1] = pos[1];
  _pos[2] = pos[2];
}

ToyMCTrack::~ToyMCTrack(){
}
