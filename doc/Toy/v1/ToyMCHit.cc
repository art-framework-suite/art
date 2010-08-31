#include "ToyMCHit.hh"

ToyMCHit::ToyMCHit( int track_id,
	     int channel_id,
	     double distance,
		    double pulse_height):
  track(track_id),
  chan(channel_id),
  dist(distance),
  ph(pulse_height)
{}


ToyMCHit::~ToyMCHit(){
}
